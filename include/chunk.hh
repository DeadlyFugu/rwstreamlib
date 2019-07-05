/*
 * File: chunk.hh
 * Author: DeadlyFugu
 * License: zlib
 * Description: Defines classes for various RW section types
 * (note: terms 'chunk' and 'section' used interchangeably within this library)
 */

#pragma once
#include "util.hh"
#include <vector>
#include "buffer.hh"

/// RW binary stream section types
enum ChunkType : uint32_t {
	RW_NONE        = 0x0,
	RW_STRUCT          = 0x1,
	RW_STRING          = 0x2,
	RW_EXTENSION       = 0x3,
	RW_CAMERA          = 0x5,
	RW_TEXTURE         = 0x6,
	RW_MATERIAL        = 0x7,
	RW_MATERIAL_LIST         = 0x8,
	RW_ATOMIC_SECTION      = 0x9,
	RW_PLANE_SECTION       = 0xA,
	RW_WORLD           = 0xB,
	RW_SPLINE          = 0xC,
	RW_MATRIX          = 0xD,
	RW_FRAME_LIST       = 0xE,
	RW_GEOMETRY        = 0xF,
	RW_CLUMP           = 0x10,
	RW_LIGHT           = 0x12,
	RW_UNICODE_STRING   = 0x13,
	RW_ATOMIC          = 0x14,
	RW_TEXTURE_NATIVE   = 0x15,
	RW_TEXTURE_DICT   = 0x16,
	RW_ANIMATION_DATABASE    = 0x17,
	RW_IMAGE           = 0x18,
	RW_SKIN_ANIMATION   = 0x19,
	RW_GEOMETRY_LIST    = 0x1A,
	RW_ANIM_ANIMATION   = 0x1B,
	RW_HANIM_ANIMATION  = 0x1B,
	RW_TEAM            = 0x1C,
	RW_CROWD           = 0x1D,
	RW_DELTA_MORPH_ANIMATION = 0x1E,
	RW_RIGHT_TO_RENDER   = 0x1F,
	RW_MULTITEXTURE_EFFECT_NATIVE  = 0x20,
	RW_MULTITEXTURE_EFFECT_DICT    = 0x21,
	RW_TEAM_DICTIONARY  = 0x22,
	RW_PI_TEX_DICT = 0x23, // Platform Independent Texture Dictionary
	RW_TABLE_OF_CONTENTS             = 0x24,
	RW_PARTICLE_STD_GLOBAL_DATA = 0x25,
	RW_ALTPIPE         = 0x26,
	RW_PI_PEDS          = 0x27,
	RW_PATCH_MESH       = 0x28,
	RW_CHUNK_GROUP_START = 0x29,
	RW_CHUNK_GROUP_END   = 0x2A,
	RW_UVANIM_DICT      = 0x2B,
	RW_COLLTREE        = 0x2C,
	RW_ENVIRONMENT     = 0x2D,
	RW_CORE_PLUGIN_ID_MAX = 0x2E,

	RW_MORPH_PLG           = 0x105,
	RW_SKY_MIPMAP_VAL       = 0x110,
	RW_SKIN_PLG            = 0x116,
	RW_PARTICLES_PLG       = 0x118,
	RW_HANIM_PLG           = 0x11E,
	RW_MATERIAL_EFFECTS_PLG = 0x120,
	RW_DELTA_MORPH_PLG  = 0x122,
	RW_PDS_PLG          = 0x131,
	RW_ADC_PLG          = 0x134,
	RW_UVANIM_PLG       = 0x135,
	RW_BINMESH_PLG         = 0x50E,
	RW_NATIVEDATA_PLG      = 0x510,
	RW_VERTEXFORMAT_PLG    = 0x510,

	RW_RS_PIPELINESET      = 0x253F2F3,
	RW_RS_SPECULARMAT      = 0x253F2F6,
	RW_RS_2DFX             = 0x253F2F8,
	RW_RS_NIGHTVERTEXCOLOR = 0x253F2F9,
	RW_RS_COLLISIONMODEL   = 0x253F2FA,
	RW_RS_REFLECTIONMAT    = 0x253F2FC,
	RW_RS_MESHEXTENSION    = 0x253F2FD,
	RW_RS_FRAME            = 0x253F2FE
};

namespace rw {
	/// abstract section base class
	class Chunk {
	public:
		ChunkType type;
		uint32_t version;

		Chunk(ChunkType type, uint32_t version): type(type), version(version) {};
		virtual ~Chunk() {};

		virtual void read(util::Buffer& in) = 0;
		virtual void write(util::Buffer& out) = 0;

		virtual void dump(util::DumpWriter out) = 0;

		virtual bool isList() = 0;
		virtual bool isData() = 0;
	};

	/// base class for any component consisting of just children (e.g. MaterialList)
	class ListChunk : public Chunk {
	public:
		std::vector<Chunk*> children;

		ListChunk(ChunkType type, uint32_t version) : Chunk(type, version) {}
		virtual ~ListChunk();

		virtual void read(util::Buffer& in);
		virtual void write(util::Buffer& out);

		virtual void dump(util::DumpWriter out);

		/// sub-classes may override this to implement custom functionality
		virtual void postReadHook() {}
		/// sub-classes may override this to implement custom functionality
		virtual void preWriteHook() {}

		void addChild(Chunk* c);
		Chunk* getChild(int idx);
		int getChildCount();
		std::vector<Chunk*> filterChildren(ChunkType type);

		virtual bool isList() {return true;}
		virtual bool isData() {return false;}
	};

	/// base class for any component consisting of just data (e.g. Struct)
	class StructChunk : public Chunk {
	protected:
		util::Buffer data;
	public:
		StructChunk(ChunkType type, uint32_t version) : Chunk(type, version), data(0) {}
		StructChunk(ChunkType type, uint32_t version, util::Buffer& data) : Chunk(type, version), data(data.copy()) {}
		virtual ~StructChunk() {};

		virtual util::Buffer& getBuffer() {
			data.seek(0);
			return data;
		}

		virtual void read(util::Buffer& in);
		virtual void write(util::Buffer& out);

		virtual void dump(util::DumpWriter out);

		/// sub-classes may override this to implement custom functionality
		virtual void postReadHook() {}
		/// sub-classes may override this to implement custom functionality
		virtual void preWriteHook() {}

		virtual bool isList() {return false;}
		virtual bool isData() {return true;}
	};

	class StringChunk : public StructChunk {
	public:
		StringChunk(ChunkType type, uint32_t version, const char* content) : StructChunk(type, version) {
			data.write(content, std::strlen(content));
		}
		StringChunk(ChunkType type, uint32_t version, util::Buffer& buf) : StructChunk(type, version, buf) {}
		StringChunk(ChunkType type, uint32_t version) : StructChunk(type, version) {}

		virtual void dump(util::DumpWriter out);
	};

	const char* getChunkName(ChunkType i);

	/// Reads an entire chunk from a buffer
	Chunk* readChunk(util::Buffer& buf);
}