/*
 * File: chunk.cc
 * Author: DeadlyFugu
 * License: zlib
 * Description: Defines classes for various RW section types
 * (note: terms 'chunk' and 'section' used interchangeably within this library)
 */

#include "chunk.hh"
#include "material.hh"
#include "world.hh"
#include "texture.hh"
#include "animation.hh"
#include "geometry.hh"

#include <unordered_map>

namespace std {
	template<> struct hash<ChunkType> {
		size_t operator()(const ChunkType& k) const {
			return hash<int>()((int)k);
		}
	};
}
namespace rw {
// From https://github.com/aap/rwtools
	static const char* chunks[] = {
			"None", "Struct", "String", "Extension", "Unknown",
			"Camera", "Texture", "Material", "Material List", "Atomic Section",
			"Plane Section", "World", "Spline", "Matrix", "Frame List",
			"Geometry", "Clump", "Unknown", "Light", "Unicode String", "Atomic",
			"Texture Native", "Texture Dictionary", "Animation Database",
			"Image", "Skin Animation", "Geometry List", "Anim Animation",
			"Team", "Crowd", "Delta Morph Animation", "Right To Render",
			"MultiTexture Effect Native", "MultiTexture Effect Dictionary",
			"Team Dictionary", "Platform Independet Texture Dictionary",
			"Table of Contents", "Particle Standard Global Data", "AltPipe",
			"Platform Independet Peds", "Patch Mesh", "Chunk Group Start",
			"Chunk Group End", "UV Animation Dictionary", "Coll Tree"
	};

/* From 0x0101 through 0x0135 */
	static const char* toolkitchunks0[] = {
			"Metrics PLG", "Spline PLG", "Stereo PLG",
			"VRML PLG", "Morph PLG", "PVS PLG", "Memory Leak PLG", "Animation PLG",
			"Gloss PLG", "Logo PLG", "Memory Info PLG", "Random PLG",
			"PNG Image PLG", "Bone PLG", "VRML Anim PLG", "Sky Mipmap Val",
			"MRM PLG", "LOD Atomic PLG", "ME PLG", "Lightmap PLG",
			"Refine PLG", "Skin PLG", "Label PLG", "Particles PLG", "GeomTX PLG",
			"Synth Core PLG", "STQPP PLG",
			"Part PP PLG", "Collision PLG", "HAnim PLG", "User Data PLG",
			"Material Effects PLG", "Particle System PLG", "Delta Morph PLG",
			"Patch PLG", "Team PLG", "Crowd PP PLG", "Mip Split PLG",
			"Anisotrophy PLG", "Not used", "GCN Material PLG", "Geometric PVS PLG",
			"XBOX Material PLG", "Multi Texture PLG", "Chain PLG", "Toon PLG",
			"PTank PLG", "Particle Standard PLG", "PDS PLG", "PrtAdv PLG",
			"Normal Map PLG", "ADC PLG", "UV Animation PLG"
	};

/* From 0x0180 through 0x01c1 */
	static const char* toolkitchunks1[] = {
			"Character Set PLG", "NOHS World PLG", "Import Util PLG",
			"Slerp PLG", "Optim PLG", "TL World PLG", "Database PLG",
			"Raytrace PLG", "Ray PLG", "Library PLG",
			"Not used", "Not used", "Not used", "Not used", "Not used", "Not used",
			"2D PLG", "Tile Render PLG", "JPEG Image PLG", "TGA Image PLG",
			"GIF Image PLG", "Quat PLG", "Spline PVS PLG", "Mipmap PLG",
			"MipmapK PLG", "2D Font", "Intersection PLG", "TIFF Image PLG",
			"Pick PLG", "BMP Image PLG", "RAS Image PLG", "Skin FX PLG",
			"VCAT PLG", "2D Path", "2D Brush", "2D Object", "2D Shape", "2D Scene",
			"2D Pick Region", "2D Object String", "2D Animation PLG",
			"2D Animation",
			"Not used", "Not used", "Not used", "Not used", "Not used", "Not used",
			"2D Keyframe", "2D Maestro", "Barycentric",
			"Platform Independent Texture Dictionary TK", "TOC TK", "TPL TK",
			"AltPipe TK", "Animation TK", "Skin Split Tookit", "Compressed Key TK",
			"Geometry Conditioning PLG", "Wing PLG", "Generic Pipeline TK",
			"Lightmap Conversion TK", "Filesystem PLG", "Dictionary TK",
			"UV Animation Linear", "UV Animation Parameter"
	};

	static const char* RSchunks[] = {
			"Unused 1", "Unused 2", "Unused 3",
			"Pipeline Set", "Unused 5", "Unused 6", "Specular Material",
			"Unused 8", "2dfx", "Night Vertex Colors", "Collision Model",
			"Unused 12", "Reflection Material", "Mesh Extension", "Frame",
			"Unused 16"
	};

	const char* getChunkName(ChunkType i) {
		switch ((uint32_t) i) {
			case 0x50E:
				return "Bin Mesh PLG";
				break;
			case 0x510:
				return "Native Data PLG";
				break;
			case 0xF21E:
				return "ZModeler Lock";
				break;
			default:
				break;
		}

		if (i <= 45)
			return chunks[i];
		else if (i <= 0x0253F2FF && i >= 0x0253F2F0)
			return RSchunks[i - 0x0253F2F0];
		else if (i <= 0x0135 && i >= 0x0101)
			return toolkitchunks0[i - 0x0101];
		else if (i <= 0x01C0 && i >= 0x0181)
			return toolkitchunks1[i - 0x0181];
		else
			return "Unknown";
	}

	typedef Chunk* (*ChunkLoadFn)(ChunkType type, uint32_t version);
	static std::unordered_map<ChunkType, ChunkLoadFn> chunkTypeLoaders;

	static bool loadersWereInit = false;
	static void initLoaders() {
		chunkTypeLoaders[(ChunkType) -1] = [](ChunkType type, uint32_t version){return (Chunk*) new ListChunk(type, version);};
		chunkTypeLoaders[RW_STRUCT] = [](ChunkType type, uint32_t version){return (Chunk*) new StructChunk(type, version);};
		chunkTypeLoaders[RW_STRING] = [](ChunkType type, uint32_t version){return (Chunk*) new StringChunk(type, version);};

		chunkTypeLoaders[RW_TEXTURE] = [](ChunkType type, uint32_t version){return (Chunk*) new TextureChunk(type, version);};
		chunkTypeLoaders[RW_MATERIAL] = [](ChunkType type, uint32_t version){return (Chunk*) new MaterialChunk(type, version);};
		chunkTypeLoaders[RW_MATERIAL_LIST] = [](ChunkType type, uint32_t version){return (Chunk*) new MaterialListChunk(type, version);};

		chunkTypeLoaders[RW_BINMESH_PLG] = [](ChunkType type, uint32_t version){return (Chunk*) new BinMeshPLGChunk(type, version);};
		chunkTypeLoaders[RW_ATOMIC_SECTION] = [](ChunkType type, uint32_t version){return (Chunk*) new AtomicSectionChunk(type, version);};
		chunkTypeLoaders[RW_PLANE_SECTION] = [](ChunkType type, uint32_t version){return (Chunk*) new PlaneSectionChunk(type, version);};
		chunkTypeLoaders[RW_WORLD] = [](ChunkType type, uint32_t version){return (Chunk*) new WorldChunk(type, version);};

		chunkTypeLoaders[RW_TEXTURE_DICT] = [](ChunkType type, uint32_t version){return (Chunk*) new TextureDictionary(type, version);};
		chunkTypeLoaders[RW_TEXTURE_NATIVE] = [](ChunkType type, uint32_t version){return (Chunk*) new TextureNative(type, version);};

		chunkTypeLoaders[RW_ANIM_ANIMATION] = [](ChunkType type, uint32_t version){return (Chunk*) new AnimAnimationChunk(type, version);};

		chunkTypeLoaders[RW_GEOMETRY] = [](ChunkType type, uint32_t version){return (Chunk*) new GeometryChunk(type, version);};
		chunkTypeLoaders[RW_GEOMETRY_LIST] = [](ChunkType type, uint32_t version){return (Chunk*) new GeometryListChunk(type, version);};
		chunkTypeLoaders[RW_FRAME_LIST] = [](ChunkType type, uint32_t version){return (Chunk*) new FrameListChunk(type, version);};
		chunkTypeLoaders[RW_ATOMIC] = [](ChunkType type, uint32_t version){return (Chunk*) new AtomicChunk(type, version);};
		chunkTypeLoaders[RW_CLUMP] = [](ChunkType type, uint32_t version){return (Chunk*) new ClumpChunk(type, version);};
		chunkTypeLoaders[RW_DELTA_MORPH_PLG] = [](ChunkType type, uint32_t version){return (Chunk*) new DeltaMorphPLGChunk(type, version);};

		loadersWereInit = true;
	}

	Chunk* readChunk(util::Buffer& buf) {
		using namespace sk::types;
		using util::logger;

		if (!loadersWereInit) initLoaders();

		struct {
			u32 type;
			u32 size;
			u32 version;
		} header;

		if (buf.remaining() < 12) {
			logger.warn("No chunk found");
			return nullptr;
		}
		buf.read(&header);

		if (buf.remaining() < header.size) {
			logger.warn("Invalid chunk (size too large)");
			// todo: somehow report offset of chunk in file (store file offs as extra param on buffer?)
			buf.seek(buf.size());
			return nullptr;
		}

		util::Buffer content = buf.view(buf.tell(), header.size);
		buf.seek(buf.tell() + header.size);

		auto itLoader = chunkTypeLoaders.find((ChunkType)header.type);
		ChunkLoadFn loader;

		if (itLoader == chunkTypeLoaders.end()) {
			// try and guess whether struct or list type
			if (content.size() >= 12) {
				u32 version;
				content.seek(8);
				content.read(&version);
				content.seek(0);
				if (version == header.version) {
					loader = chunkTypeLoaders[(ChunkType) -1];
				} else {
					loader = chunkTypeLoaders[RW_STRUCT];
				}
			} else {
				loader = chunkTypeLoaders[RW_STRUCT];
			}
		} else {
			loader = itLoader->second;
		}

		Chunk* chunk = loader((ChunkType) header.type, header.version);
		chunk->read(content);

		return chunk;
	}

	ListChunk::~ListChunk() {
		for (auto chunk : children) {
			delete chunk;
		}
	}

	void ListChunk::addChild(rw::Chunk* c) {
		children.push_back(c);
	}

	std::vector<Chunk*> ListChunk::filterChildren(ChunkType type) {
		std::vector<Chunk*> filtered;
		for (auto chunk : children) {
			if (chunk->type == type) {
				filtered.push_back(chunk);
			}
		}
		return filtered;
	}

	Chunk* ListChunk::getChild(int idx) {
		return children[idx];
	}

	int ListChunk::getChildCount() {
		return children.size();
	}

	void ListChunk::read(util::Buffer& in) {
		while (in.remaining()) {
			Chunk* chunk = readChunk(in);
			if (chunk) {
				addChild(chunk);
			}
		}
		postReadHook();
	}

	void ListChunk::write(util::Buffer& out) {
		preWriteHook();
		// todo: impl
	}

	void ListChunk::dump(util::DumpWriter out) {
		out.print("%s: (%d children)", getChunkName(type), children.size());

		bool isFirstChild = true;
		for (auto child : children) {
			if (isFirstChild) isFirstChild = false;
			else out.print("");

			child->dump(out);
		}
	}

	void StructChunk::read(util::Buffer& in) {
		data.setStretchy(true);
		data.resize(in.size());
		data.seek(0);
		data.write(in);
		data.setStretchy(false);
		postReadHook();
	}

	void StructChunk::write(util::Buffer& out) {
		preWriteHook();
		// todo: impl
	}

	void StructChunk::dump(util::DumpWriter out) {
		if (out.isVerbose()) {
			out.print("%s: (%d bytes)", getChunkName(type), data.size());
			util::dumpBuffer(data, out);
		} else {
			out.print("%s: <buffer of %d bytes>", getChunkName(type), data.size());
		}
	}

	void StringChunk::dump(util::DumpWriter out) {
		out.print("%s: \"%s\"", getChunkName(type), data.base_ptr());
	}
}