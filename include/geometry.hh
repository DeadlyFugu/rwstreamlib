/*
 * File: geometry.hh
 * Author: DeadlyFugu
 * License: zlib
 * Description: Classes representing Geometry List, and Geometry sections
 */

#pragma once

#include "chunk.hh"
#include "material.hh"

namespace rw {
	namespace geom {
		struct VertexPosition {
			float x;
			float y;
			float z;
		};

		struct VertexNormal {
			float x;
			float y;
			float z;
		};

		struct VertexColor {
			union {
				struct {
					uint8_t r;
					uint8_t g;
					uint8_t b;
					uint8_t a;
				};
				uint32_t as_int;
			};
		};

		struct VertexUVs {
			float u;
			float v;
		};

		struct Face {
			uint16_t material;
			uint16_t vertex1;
			uint16_t vertex2;
			uint16_t vertex3;
		};

		struct Vector3f {
			float x;
			float y;
			float z;
		};

		struct Matrix3x3f {
			Vector3f row1;
			Vector3f row2;
			Vector3f row3;
		};
	};

	const int RW_GEOMETRY_TRISTRIP = 0x00000001; // Is triangle strip (if disabled it will be an triangle list)
	const int RW_GEOMETRY_POSITIONS = 0x00000002; // Vertex translation
	const int RW_GEOMETRY_TEXTURED = 0x00000004; // Texture coordinates
	const int RW_GEOMETRY_PRELIT = 0x00000008; // Vertex colors
	const int RW_GEOMETRY_NORMALS = 0x00000010; // Store normals
	const int RW_GEOMETRY_LIGHT = 0x00000020; // Geometry is lit (dynamic and static)
	const int RW_GEOMETRY_MODULATE_MATERIAL_COLOR = 0x00000040; // Modulate material color
	const int RW_GEOMETRY_TEXTURED2 = 0x00000080; // Texture coordinates 2
	const int RW_GEOMETRY_NATIVE = 0x01000000; // Native Geometry

	class GeometryChunk : public ListChunk {
	public:
		uint32_t format;
		uint32_t triangleCount;
		uint32_t vertexCount;
		uint32_t morphTargetCount;

		float ambient;
		float specular;
		float diffuse;
		bool hasSurfaceProperties;

		struct MorphTarget {
			struct {
				float x;
				float y;
				float z;
				float radius;
			} boundingSphere;
			uint32_t hasVertices;
			uint32_t hasNormals;
			std::vector<geom::VertexPosition> vertexPositions;
			std::vector<geom::VertexNormal> vertexNormals;
		};

		std::vector<MorphTarget> morphTargets;
		std::vector<geom::VertexColor> vertexColors;
		std::vector<std::vector<geom::VertexUVs>> vertexUVLayers;
		std::vector<geom::Face> faces;

		MaterialListChunk* materialList;

		std::vector<Chunk*> extensions;

		GeometryChunk(ChunkType type, uint32_t version) : ListChunk(type, version) {}

		virtual void dump(util::DumpWriter out);

		virtual void postReadHook();

		virtual void preWriteHook();
	};

	class GeometryListChunk : public ListChunk {
	public:
		std::vector<GeometryChunk*> geometries;

		GeometryListChunk(ChunkType type, uint32_t version) : ListChunk(type, version) {}

		virtual void dump(util::DumpWriter out);

		virtual void postReadHook();

		virtual void preWriteHook();
	};

	class FrameListChunk : public ListChunk {
	public:
		struct Frame {
			geom::Matrix3x3f rotation;
			geom::Vector3f translation;
			uint32_t previous;
			uint32_t matrixFlags;
		};
		std::vector<Frame> frames;

		FrameListChunk(ChunkType type, uint32_t version) : ListChunk(type, version) {}

		virtual void dump(util::DumpWriter out);

		virtual void postReadHook();

		virtual void preWriteHook();
	};

	class AtomicChunk : public ListChunk {
	public:
		uint32_t frameIndex;
		uint32_t geometryIndex;
		uint32_t flags;
		uint32_t unused;

		AtomicChunk(ChunkType type, uint32_t version) : ListChunk(type, version) {}

		virtual void dump(util::DumpWriter out);

		virtual void postReadHook();

		virtual void preWriteHook();
	};

	class ClumpChunk : public ListChunk {
	public:
		uint32_t atomicCount;
		uint32_t lightCount;
		uint32_t cameraCount;

		FrameListChunk* frameList;
		GeometryListChunk* geometryList;
		std::vector<AtomicChunk*> atomics;
		std::vector<Chunk*> extensions;

		ClumpChunk(ChunkType type, uint32_t version) : ListChunk(type, version) {}

		virtual void dump(util::DumpWriter out);

		virtual void postReadHook();

		virtual void preWriteHook();
	};

	class DeltaMorphPLGChunk : public StructChunk {
	public:
		struct DMorphPoint {
			float x, y, z;
		};

		struct DMorphTarget {
			std::string name;
			uint32_t flags; // likely same as geom flags
			uint32_t num2;

			std::vector<uint8_t> mapping;
			std::vector<DMorphPoint> vertices;
			std::vector<DMorphPoint> normals;

			float boundX;
			float boundY;
			float boundZ;
			float boundRadius;
		};

		std::vector<DMorphTarget> targets;

		DeltaMorphPLGChunk(ChunkType type, uint32_t version) : StructChunk(type, version) {}

		virtual void dump(util::DumpWriter out);

		/// sub-classes may override this to implement custom functionality
		virtual void postReadHook();

		/// sub-classes may override this to implement custom functionality
		virtual void preWriteHook();
	};
}