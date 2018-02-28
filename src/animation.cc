/*
 * File: animation.cc
 * Author: DeadlyFugu
 * License: zlib
 * Description: Classes representing AnimAnimation, and DMorphAnimation sections
 */

#include <animation.hh>
#include "animation.hh"

namespace rw {
	void AnimAnimationChunk::dump(util::DumpWriter out) {
		out.print("Anim Animation:");
		out.print("  version: %d", animationVersion);
		out.print("  interpolation type: %d", interpolationType);
		out.print("  frame count: %d", frameCount);
		out.print("  flags: 0x%08x", flags);
		out.print("  duration: %f", duration);

		int i = 0;
		for (auto& frame : frames) {
			out.print("");
			out.print("  Frame(%d):", i++);
			out.print("    time: %f", frame.time);
			auto& data = frame.data.standard;
			out.print("    rotation: quat(%f, %f, %f, %f)",
					  data.rotationQuat[0], data.rotationQuat[1], data.rotationQuat[2], data.rotationQuat[3]);
			out.print("    translation: vec3(%f, %f, %f)",
					  data.translation[0], data.translation[1], data.translation[2]);
			out.print("    previous offset: 0x%08x", frame.previousOffset);
		}
	}

	void AnimAnimationChunk::postReadHook() {
		data.seek(0);
		data.read(&animationVersion);
		data.read(&interpolationType);
		data.read(&frameCount);
		data.read(&flags);
		data.read(&duration);

		for (int i = 0; i < frameCount; i++) {
			frames.emplace_back();
			auto& frame = frames.back();

			data.read(&frame.time);
			data.read(&frame.data.standard.rotationQuat);
			data.read(&frame.data.standard.translation);
			data.read(&frame.previousOffset);
		}
	}

	void AnimAnimationChunk::preWriteHook() {
		StructChunk::preWriteHook();
	}
}