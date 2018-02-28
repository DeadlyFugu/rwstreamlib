/*
 * File: animation.cc
 * Author: DeadlyFugu
 * License: zlib
 * Description: Classes representing AnimAnimation, and DMorphAnimation sections
 */

#pragma once
#include "chunk.hh"

namespace rw {
	class AnimAnimationChunk : public StructChunk {
	public:
		uint32_t animationVersion; // should always be 0x100
		uint32_t interpolationType; // 1: for standard layout, 20: .uvb, todo: other layouts
		uint32_t frameCount;
		uint32_t flags;
		float duration;

		struct KeyFrame {
			float time;
			uint32_t previousOffset;
			union {
				struct {
					float rotationQuat[4];
					float translation[3];
				} standard;
				struct {
					float unk1;
					float unk2;
					float unk3;
					float unk4;
					float unk5;
				} uvb;
			} data;
		};

		std::vector<KeyFrame> frames;

		AnimAnimationChunk(ChunkType type, uint32_t version) : StructChunk(type, version) {}

		virtual void dump(util::DumpWriter out);

		/// sub-classes may override this to implement custom functionality
		virtual void postReadHook();

		/// sub-classes may override this to implement custom functionality
		virtual void preWriteHook();
	};
}
