#include "util.hh"
#include "chunk.hh"
#include "texture.hh"

namespace rw {
	char* getRasterFormatLabel(uint32_t format) {
		char* buffer = new char[512];
		for (int i = 0; i < 15; i++) {
			auto idx = TEXTURE_RASTER_FORMAT_INDICES[i];
			if (idx < RASTER_AUTOMIPMAP ? (format & 0x0f00) == idx : (bool) (format & idx)) {
				if (buffer[0]) strncat(buffer, ", ", 512);
				strncat(buffer, TEXTURE_RASTER_FORMAT_LABELS[i], 512);
			}
		}
		if (!buffer[0]) strncat(buffer, "Default", 512);
		return buffer;
	}

	void rw::TextureNative::dump(util::DumpWriter out) {
		out.print("Texture Native:");
		out.print("  name: %s", name.c_str());
		out.print("  mask: %s", name.c_str());
		out.print("");
		out.print("  platform: %s", TEXTURE_PLATFORM_ID_LABELS[platformId]);
		out.print("  filter mode: %s", TEXTURE_FILTER_MODE_LABELS[filterMode]);
		out.print("  address U mode: %s", TEXTURE_ADDRESS_MODE_LABELS[addressUMode]);
		out.print("  address V mode: %s", TEXTURE_ADDRESS_MODE_LABELS[addressVMode]);
		out.print("");
		char* buffer = getRasterFormatLabel(format);
		out.print("  format: %s (0x%08x)", buffer, format);
		delete[] buffer;
		out.print("  has alpha: %s", hasAlpha ? "yes" : "no");
		out.print("  unknown: %d", unknownFlag);
		out.print("  width: %d", width);
		out.print("  height: %d", height);
		out.print("  depth: %d", depth);
		out.print("  mip levels: %d", mipLevels);
		out.print("  type: %d", type);
		out.print("  compression: %d", compression);
		out.print("  total size: %d", dataSize);
		out.print("");
		if (palette) {
			out.print("  palette: ...");
		} else {
			out.print("  palette: none");
		}
		if (data) {
			if (out.isVerbose()) {
				out.print("  data: ...");
			} else {
				out.print("  data: <%d bytes>", dataSize);
			}
		} else {
			out.print("  data: none");
		}
		out.print("...");
	}

	void rw::TextureNative::postReadHook() {
		bool structWasSeen = false;
		for (auto child : children) {
			if (child->type == RW_STRUCT) {
				if (structWasSeen) {
					util::logger.warn("Multiple structs found within TextureNative");
					continue;
				}
				structWasSeen = true;

				util::Buffer& content = ((StructChunk*) child)->getBuffer();
				content.read(&platformId);
				content.read(&filterMode);
				uint8_t addressModeCombined;
				content.read(&addressModeCombined);
				addressUMode = (TextureAddressMode) (addressModeCombined >> 4);
				addressVMode = (TextureAddressMode) (addressModeCombined & 0x0f);
				content.skip(2);
				char strBuffer[32];
				content.read(strBuffer, 32);
				name = std::string(strBuffer);
				content.read(strBuffer, 32);
				maskName = std::string(strBuffer);

				if (platformId == PLATFORM_XBOX) {
					struct {
						uint32_t format;
						uint16_t hasAlpha;
						uint16_t unknownFlag; // possibly cube map
						uint16_t width;
						uint16_t height;
						uint8_t depth;
						uint8_t mipLevels;
						uint8_t type;
						uint8_t compression;
						uint32_t totalSize;
					} header;
					content.read(&header);
					format = header.format;
					hasAlpha = header.hasAlpha;
					unknownFlag = header.unknownFlag;
					width = header.width;
					height = header.height;
					depth = header.depth;
					mipLevels = header.mipLevels;
					type = header.type;
					compression = header.compression;
					dataSize = header.totalSize;

					if (format & RASTER_PAL4) {
						palette = new uint32_t[32];
						content.read(palette, 4 * 32);
					} else if (format & RASTER_PAL8) {
						palette = new uint32_t[256];
						content.read(palette, 4 * 256);
					}

					data = new uint8_t[dataSize];
					util::logger.info("%d %d", content.remaining(), dataSize);
					content.read(data, dataSize);
					if (content.remaining() > 0) {
						util::logger.warn("%d additional bytes at end of texture", content.remaining());
					}
				} else {
					util::logger.warn("Unsupported platform: %s", TEXTURE_PLATFORM_ID_LABELS[platformId]);
				}
			} else if (child->type == RW_EXTENSION) {
				// todo: extensions
			} else {
				util::logger.warn("Unsupported chunk in TextureNative: %s", getChunkName(child->type));
			}
		}

		if (!structWasSeen) {
			util::logger.warn("TextureNative is missing struct");
		}
	}

	void rw::TextureNative::preWriteHook() {
		ListChunk::preWriteHook();
	}

	void TextureDictionary::dump(util::DumpWriter out) {
		ListChunk::dump(out);
	}

	void TextureDictionary::postReadHook() {
		bool structWasSeen = false;
		for (auto child : children) {
			if (child->type == RW_STRUCT) {
				if (structWasSeen) {
					util::logger.warn("Multiple structs found within TextureDictionary");
					continue;
				}
				structWasSeen = true;

				util::Buffer& content = ((StructChunk*) child)->getBuffer();
				content.read(&textureCount);
				content.read(&deviceId);
			} else if (child->type == RW_TEXTURE_NATIVE) {
				textures.push_back((TextureNative*) child);
			} else if (child->type == RW_EXTENSION) {
				// todo: extensions
			} else {
				util::logger.warn("Unsupported chunk in TextureDictionary: %s", getChunkName(child->type));
			}
		}

		if (!structWasSeen) {
			util::logger.warn("TextureDictionary is missing struct");
		}

		if (textureCount != textures.size()) {
			util::logger.warn("TextureDictionary structure claims %d textures are present; but %d found", textureCount, textures.size());
			textureCount = (uint16_t) textures.size();
		}
	}

	void TextureDictionary::preWriteHook() {
		ListChunk::preWriteHook();
	}
}