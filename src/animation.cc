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

	void DMorphAnimationChunk::dump(util::DumpWriter out) {
		out.print("Delta Morph Animation:");
		out.print("  version: %d", animationVersion);
		out.print("  interpolation type: %d", interpolationType);
		out.print("  target count: %d", targetCount);
		out.print("  total frame count: %d", totalFrameCount);

		int i = 0;
		for (auto& target : targets) {
			out.print("");
			out.print("  Target(%d):", i++);
			out.print("    frame count: %d", target.frames.size());

			int j = 0;
			for (auto& frame : target.frames) {
				out.print("");
				out.print("    Frame(%d):", j++);
				out.print("      start value: %f", frame.startValue);
				out.print("      end value: %f", frame.startValue);
				out.print("      duration: %f", frame.duration);
				out.print("      1/duration: %f", frame.oneOverDuration);
				out.print("      next id: %d", frame.nextId);
			}
		}
	}

	void DMorphAnimationChunk::postReadHook() {
		data.seek(12); // skip struct header
		data.read(&animationVersion);
		data.read(&interpolationType);
		data.read(&targetCount);
		data.read(&totalFrameCount);

		for (int i = 0; i < targetCount; i++) {
			targets.emplace_back();
			auto& target = targets.back();

			uint32_t frameCount;
			data.read(&frameCount);

			for (int j = 0; j < frameCount; j++) {
				target.frames.emplace_back();
				auto& frame = target.frames.back();

				data.read(&frame.startValue);
				data.read(&frame.endValue);
				data.read(&frame.duration);
				data.read(&frame.oneOverDuration);
				data.read(&frame.nextId);
			}
		}
	}

	void DMorphAnimationChunk::preWriteHook() {
		StructChunk::preWriteHook();
	}
}
