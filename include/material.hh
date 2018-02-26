/*
 * File: material.hh
 * Author: DeadlyFugu
 * License: zlib
 * Description: Classes representing MaterialList, Material, and Texture sections
 */

#pragma once
#include "chunk.hh"

namespace rw {
	const char* const TEXTURE_FILTER_MODE_LABELS[] = {
			"No Filtering",
			"Nearest",
			"Linear",
			"Mip Nearest",
			"Mip Linear",
			"Linear Mip Nearest",
			"Trilinear",
	};

	const char* const TEXTURE_ADDRESS_MODE_LABELS[] = {
			"No Tiling",
			"Tile",
			"Mirror",
			"Clamp",
			"Border",
	};

	enum TextureFilterMode : uint8_t {
		FILTERNAFILTERMODE, // filtering is disabled
		FILTERNEAREST, // Point sampled
		FILTERLINEAR, // Bilinear
		FILTERMIPNEAREST, // Point sampled per pixel mip map
		FILTERMIPLINEAR, // Bilinear per pixel mipmap
		FILTERLINEARMIPNEAREST, // MipMap interp point sampled
		FILTERLINEARMIPLINEAR, // Trilinear
	};

	enum TextureAddressMode : uint8_t {
		TEXTUREADDRESSNATEXTUREADDRESS, // no tiling
		TEXTUREADDRESSWRAP, // tile in U or V direction
		TEXTUREADDRESSMIRROR, // mirror in U or V direction
		TEXTUREADDRESSCLAMP,
		TEXTUREADDRESSBORDER,
	};

	class TextureChunk : public ListChunk {
	public:
		TextureFilterMode filterMode;
		TextureAddressMode addressUMode:4;
		TextureAddressMode addressVMode:4;
		uint16_t useMipLevels; // (bool)

		std::string textureName;
		std::string maskName;

		TextureChunk(ChunkType type, uint32_t version) : ListChunk(type, version) {}

		virtual void dump(util::DumpWriter out);

		/// sub-classes may override this to implement custom functionality
		virtual void postReadHook();

		/// sub-classes may override this to implement custom functionality
		virtual void preWriteHook();
	};

	class MaterialChunk : public ListChunk {
	public:
		uint32_t flags;
		uint32_t color;
		uint32_t unused; // usually 25293e84
		uint32_t isTextured; // (bool)
		float ambient;
		float specular;
		float diffuse;
		bool hasSurfaceProperties;
		TextureChunk* texture;

		MaterialChunk(ChunkType type, uint32_t version) : ListChunk(type, version) {}

		virtual void dump(util::DumpWriter out);

		/// sub-classes may override this to implement custom functionality
		virtual void postReadHook();

		/// sub-classes may override this to implement custom functionality
		virtual void preWriteHook();
	};

	class MaterialListChunk : public ListChunk {
	public:
		std::vector<MaterialChunk*> materials;

		MaterialListChunk(ChunkType type, uint32_t version) : ListChunk(type, version) {}

		virtual void dump(util::DumpWriter out);

		/// sub-classes may override this to implement custom functionality
		virtual void postReadHook();

		/// sub-classes may override this to implement custom functionality
		virtual void preWriteHook();
	};
}