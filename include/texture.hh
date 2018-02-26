/*
 * File: texture.hh
 * Author: DeadlyFugu
 * License: zlib
 * Description: Classes representing Texture Dictionary, and Texture Native sections
 */

#pragma once
#include "chunk.hh"
#include "material.hh"

namespace rw {
	const char* const TEXTURE_PLATFORM_ID_LABELS[] = {
			"Any",
			"D3D8",
			"D3D9",
			"GameCube",
			"Null",
			"OpenGL",
			"PS2",
			"Software Raster",
			"XBox",
			"PSP",
	};

	enum TexturePlatformID : uint16_t {
		PLATFORM_ANY,
		PLATFORM_D3D8,
		PLATFORM_D3D9,
		PLATFORM_GAMECUBE,
		PLATFORM_NULL,
		PLATFORM_OPENGL,
		PLATFORM_PS2,
		PLATFORM_SOFTWARE_RASTER,
		PLATFORM_XBOX,
		PLATFORM_PSP,
	};

	const char* const TEXTURE_RASTER_FORMAT_LABELS[] = {
			"DEFAULT",
			"C1555",
			"C565",
			"C4444",
			"LUM8",
			"C8888",
			"C888",
			"D16",
			"D24",
			"D32",
			"C555",
			"AUTOMIPMAP",
			"PAL8",
			"PAL4",
			"MIPMAP",
	};

	const uint32_t TEXTURE_RASTER_FORMAT_INDICES[] = {
			0,0x0100,0x0200,0x0300,0x0400,0x0500,0x0600,0x0700,0x0800,0x0900,0x0A00,0x1000,0x2000,0x4000,0x8000
	};

	enum TextureRasterFormat : uint32_t {
		RASTER_DEFAULT    = 0,
		RASTER_C1555      = 0x0100,
		RASTER_C565       = 0x0200,
		RASTER_C4444      = 0x0300,
		RASTER_LUM8       = 0x0400,
		RASTER_C8888      = 0x0500,
		RASTER_C888       = 0x0600,
		RASTER_D16        = 0x0700,
		RASTER_D24        = 0x0800,
		RASTER_D32        = 0x0900,
		RASTER_C555       = 0x0A00,
		RASTER_AUTOMIPMAP = 0x1000,
		RASTER_PAL8       = 0x2000,
		RASTER_PAL4       = 0x4000,
		RASTER_MIPMAP     = 0x8000
	};

	/// NOTE! result must be delete[]'d!!!
	char* getRasterFormatLabel(uint32_t format);

	class TextureNative : public ListChunk {
	public:
		uint32_t platformId;
		TextureFilterMode filterMode;
		TextureAddressMode addressUMode:4;
		TextureAddressMode addressVMode:4;
		std::string name;
		std::string maskName;

		uint32_t format;
		uint16_t hasAlpha;
		uint16_t unknownFlag; // possibly cube map
		uint16_t width;
		uint16_t height;
		uint8_t depth;
		uint8_t mipLevels;
		uint8_t type;
		uint8_t compression;
		uint32_t dataSize;

		uint32_t* palette;
		uint8_t* data;

		TextureNative(ChunkType type, uint32_t version) : ListChunk(type, version) {}

		virtual void dump(util::DumpWriter out);

		/// sub-classes may override this to implement custom functionality
		virtual void postReadHook();

		/// sub-classes may override this to implement custom functionality
		virtual void preWriteHook();
	};

	class TextureDictionary : public ListChunk {
	public:
		uint16_t textureCount;
		uint16_t deviceId; // note: only present in 3.6.0.0 and above; otherwise must be 0

		std::vector<TextureNative*> textures;

		TextureDictionary(ChunkType type, uint32_t version) : ListChunk(type, version) {}

		virtual void dump(util::DumpWriter out);

		/// sub-classes may override this to implement custom functionality
		virtual void postReadHook();

		/// sub-classes may override this to implement custom functionality
		virtual void preWriteHook();
	};
}