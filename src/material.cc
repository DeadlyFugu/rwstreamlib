#include "material.hh"
#include "util.hh"

namespace rw {
	void TextureChunk::dump(util::DumpWriter out) {
		out.print("Texture:");
		out.print("  filter mode: %s", TEXTURE_FILTER_MODE_LABELS[this->filterMode]);
		out.print("  address U mode: %s", TEXTURE_ADDRESS_MODE_LABELS[this->addressUMode]);
		out.print("  address V mode: %s", TEXTURE_ADDRESS_MODE_LABELS[this->addressVMode]);
		out.print("  use mip levels: %s", this->useMipLevels ? "yes" : "no");

		out.print("");
		out.print("  texture name: %s", this->textureName.c_str());
		out.print("  mask name: %s", this->maskName.c_str());
	}

	void TextureChunk::postReadHook() {
		bool structWasSeen = false;
		bool texNameSeen = false;
		bool maskNameSeen = false;
		for (auto child : children) {
			if (child->type == RW_STRUCT) {
				if (structWasSeen) {
					util::logger.warn("Multiple structs found within Texture");
					continue;
				}
				structWasSeen = true;

				util::Buffer& content = ((StructChunk*) child)->getBuffer();
				content.read(&this->filterMode);
				uint8_t addressModeCombined;
				content.read(&addressModeCombined);
				this->addressUMode = (TextureAddressMode) (addressModeCombined >> 4);
				this->addressVMode = (TextureAddressMode) (addressModeCombined & 0x0f);
				content.read(&this->useMipLevels);
			} else if (child->type == RW_STRING) {
				if (!texNameSeen) {
					texNameSeen = true;
					textureName = (char*) ((StringChunk*) child)->getBuffer().base_ptr();
				} else if (!maskNameSeen) {
					maskNameSeen = true;
					maskName = (char*) ((StringChunk*) child)->getBuffer().base_ptr();
				} else {
					util::logger.warn("Additional String chunks within Texture (will be ignored)");
				}
			} else if (child->type == RW_EXTENSION) {
				// todo: extensions
			} else {
				util::logger.warn("Unsupported chunk in Texture: %s", getChunkName(child->type));
			}
		}

		if (!structWasSeen) {
			util::logger.warn("Texture is missing struct");
		}
		if (!texNameSeen) {
			util::logger.warn("Texture is missing texture name");
		}
		if (!maskNameSeen) {
			util::logger.warn("Texture is missing mask name");
		}
	}

	void TextureChunk::preWriteHook() {
		ListChunk::preWriteHook();
	}

	void MaterialChunk::dump(rw::util::DumpWriter out) {
		out.print("Material:");
		out.print("  flags: %08x", this->flags);
		out.print("  color: #%08x", this->color);
		out.print("  unused: 0x%08x", this->unused);
		out.print("  isTextured: %s", this->isTextured ? "yes" : "no");
		if (this->hasSurfaceProperties) {
			out.print("");
			out.print("  ambient: %.3f", this->ambient);
			out.print("  specular: %.3f", this->specular);
			out.print("  diffuse: %.3f", this->diffuse);
		}

		if (this->texture) {
			out.print("");
			this->texture->dump(out);
		}
	}

	void MaterialChunk::postReadHook() {
		bool structWasSeen = false;
		bool textureWasSeen = false;
		std::vector<Chunk*> materialChunks = this->filterChildren(RW_MATERIAL);
		int chunkIdx = 0;
		this->isTextured = 0;
		for (auto child : children) {
			if (child->type == RW_STRUCT) {
				if (structWasSeen) {
					util::logger.warn("Multiple structs found within Material");
					continue;
				}
				structWasSeen = true;

				util::Buffer& content = ((StructChunk*) child)->getBuffer();
				content.read(&this->flags);
				content.read(&this->color);
				content.read(&this->unused);
				content.read(&this->isTextured);
				if (util::unpackVersionNumber(this->version) > 0x30400) {
					content.read(&this->ambient);
					content.read(&this->specular);
					content.read(&this->diffuse);
					this->hasSurfaceProperties = true;
				} else {
					this->hasSurfaceProperties = false;
				}
			} else if (child->type == RW_TEXTURE) {
				if (textureWasSeen) {
					util::logger.warn("Multiple Textures found within Material");
					continue;
				}
				textureWasSeen = true;

				if (!isTextured) {
					util::logger.warn("Non-textured Material has Texture struct (will be ignored)");
					continue;
				}

				this->texture = (TextureChunk*) child;
			} else if (child->type == RW_EXTENSION) {
				// todo: extensions
			} else {
				util::logger.warn("Unsupported chunk in Material: %s", getChunkName(child->type));
			}
		}

		if (!structWasSeen) {
			util::logger.warn("Material is missing struct");
		}
	}

	void MaterialChunk::preWriteHook() {
		// todo: impl
		ListChunk::preWriteHook();
	}

	void MaterialListChunk::dump(util::DumpWriter out) {
		// todo: impl
		ListChunk::dump(out);
	}

	void MaterialListChunk::postReadHook() {
		bool structWasSeen = false;
		std::vector<Chunk*> materialChunks = this->filterChildren(RW_MATERIAL);
		int chunkIdx = 0;
		for (auto child : children) {
			if (child->type == RW_STRUCT) {
				if (structWasSeen) {
					util::logger.warn("Multiple structs found within MaterialList");
				} else {
					structWasSeen = true;

					util::Buffer& content = ((StructChunk*) child)->getBuffer();
					uint32_t materialCount;
					content.read(&materialCount);

					for (int i = 0; i < materialCount; i++) {
						int32_t matReference;
						content.read(&matReference);
						if (matReference == -1) {
							if (chunkIdx >= materialChunks.size()) {
								util::logger.warn("More materials referenced in MaterialList struct than actually exist");
								break;
							}
							this->materials.push_back((MaterialChunk*) materialChunks[chunkIdx++]);
						} else {
							if (matReference >= this->materials.size()) {
								util::logger.warn("Forward reference not allowed in MaterialList");
								break;
							}
							// todo: are references based on ID into this->materials or materialChunks?
							// (below assumes this->materials)
							this->materials.push_back(this->materials[matReference]);
						}
					}

					if (chunkIdx < materialChunks.size()) {
						util::logger.warn("Unused materials are present");
					}
				}
			} else if (child->type == RW_MATERIAL) {
				// ignore, materials are read by above struct
			} else if (child->type == RW_EXTENSION) {
				// todo: extensions
			} else {
				util::logger.warn("Unsupported chunk in MaterialList: %s", getChunkName(child->type));
			}
		}

		if (!structWasSeen) {
			util::logger.warn("MaterialList is missing struct");
			// fallback to generate material list
			for (Chunk* chunk : materialChunks) {
				this->materials.push_back((MaterialChunk*) chunk);
			}
		}
	}

	void MaterialListChunk::preWriteHook() {
		// todo: impl
		ListChunk::preWriteHook();
	}
}
