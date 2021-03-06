/*
 * File: world.hh
 * Author: DeadlyFugu
 * License: zlib
 * Description: Classes representing BinMesh PLG, Atomic Section, and World sections
 */

#pragma once
#include "chunk.hh"
#include "material.hh"
#include "geometry.hh"

namespace rw {
	class BinMeshPLGChunk : public StructChunk {
	public:
		uint32_t flags; // 0 is trilist; 1 is tristrip
		uint32_t objectCount;
		uint32_t indexCount;

		struct BinMeshObject {
			uint32_t meshIndexCount;
			uint32_t material;
			std::vector<uint32_t> indices;
		};

		std::vector<BinMeshObject> objects;

		BinMeshPLGChunk(ChunkType type, uint32_t version) : StructChunk(type, version) {}

		virtual void dump(util::DumpWriter out);

		/// sub-classes may override this to implement custom functionality
		virtual void postReadHook();

		/// sub-classes may override this to implement custom functionality
		virtual void preWriteHook();
	};

	class AbstractSectionChunk : public ListChunk {
	public:
		AbstractSectionChunk(ChunkType type, uint32_t version) : ListChunk(type, version) {}

		virtual bool isAtomic() = 0;
	};

	class AtomicSectionChunk : public AbstractSectionChunk {
	public:
		uint32_t modelFlags;
		uint32_t faceCount;
		uint32_t vertexCount;
		float bboxMax[3];
		float bboxMin[3];
		uint32_t unknownA; // always 0x84d9502f
		uint32_t unknownB; // always 0

		std::vector<geom::VertexPosition> vertexPositions;
		std::vector<geom::VertexColor> vertexColors;
		std::vector<geom::VertexUVs> vertexUVs;
		std::vector<geom::Face> faces;

		BinMeshPLGChunk* binMeshPLG; // (null) if extension not present

		AtomicSectionChunk(ChunkType type, uint32_t version) : AbstractSectionChunk(type, version) {}

		virtual void dump(util::DumpWriter out);

		/// sub-classes may override this to implement custom functionality
		virtual void postReadHook();

		/// sub-classes may override this to implement custom functionality
		virtual void preWriteHook();

		virtual bool isAtomic() override;
	};

	class PlaneSectionChunk : public AbstractSectionChunk {
	public:
		uint32_t type;
		float value;
		bool leftIsAtomic;
		bool rightIsAtomic;
		float leftValue;
		float rightValue;

		AbstractSectionChunk* left;
		AbstractSectionChunk* right;

		PlaneSectionChunk(ChunkType type, uint32_t version) : AbstractSectionChunk(type, version) {}

		virtual void dump(util::DumpWriter out);

		/// sub-classes may override this to implement custom functionality
		virtual void postReadHook();

		/// sub-classes may override this to implement custom functionality
		virtual void preWriteHook();

		virtual bool isAtomic() override;
	};

	class WorldChunk : public ListChunk {
	public:
		uint32_t unknownA[4];
		uint32_t faceCount;
		uint32_t vertexCount;
		uint32_t unknownB[4];
		float bboxMax[3];
		float bboxMin[3];

		MaterialListChunk* materialList;
		AbstractSectionChunk* rootSection;

		WorldChunk(ChunkType type, uint32_t version) : ListChunk(type, version) {}

		virtual void dump(util::DumpWriter out);

		/// sub-classes may override this to implement custom functionality
		virtual void postReadHook();

		/// sub-classes may override this to implement custom functionality
		virtual void preWriteHook();
	};
}