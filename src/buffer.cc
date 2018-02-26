/*
 * File: buffer.cc
 * Author: DeadlyFugu
 * License: zlib
 * Description: Managed memory blocks
 */

#include "util.hh"
#include "buffer.hh"
#include <stdlib.h>

using std::string;
using rw::util::logger;

namespace sk {

	Buffer::Buffer(unsigned len, bool zeroed) : stretchy(false), owned(true) {
		if (zeroed) {
			base = (u8*) calloc(1, len);
		} else {
			base = (u8*) malloc(len);
		}

		head = base;
		end = base + len;
	}

	Buffer::Buffer(void* src, unsigned len, bool owned) : stretchy(false), owned(owned) {
		base = (u8*) src;
		head = base;
		end = base + len;
	}

	Buffer::~Buffer() {
		if (owned) {
			free(base);
		}
	}

	Buffer Buffer::view() {
		return Buffer(base, size(), false);
	}

	Buffer Buffer::view(unsigned start, unsigned len) {
		if(start + len > size()) {
			logger.error("view out of bounds");
			exit(-1);
		}
		return Buffer(base + start, len, false);
	}

	Buffer Buffer::copy() {
		auto len = size();
		auto new_base = malloc(len);
		memcpy(new_base, base, len);
		return Buffer(new_base, len, true);
	}

	Buffer Buffer::copy(unsigned start, unsigned len) {
		if (start + len > size()) {
			logger.error("copy out of bounds");
			exit(-1);
		}
		auto new_base = malloc(len);
		memcpy(new_base, base + start, len);
		return Buffer(new_base, len, true);
	}

	void Buffer::seek(unsigned pos) {
		head = base + pos;
	}

	void Buffer::skip(unsigned bytes) {
		head += bytes;
	}

	unsigned Buffer::tell() {
		return (unsigned) (head - base);
	}

	void* Buffer::base_ptr() {
		return base;
	}

	void* Buffer::head_ptr() {
		return head;
	}

	unsigned Buffer::size() {
		return (unsigned) (end - base);
	}

	unsigned Buffer::remaining() {
		return (unsigned) (end - head);
	}

	void Buffer::read(void* dst, unsigned len) {
		if (head + len > end) {
			logger.error("read out of bounds");
			exit(-1);
		}
		memcpy(dst, head, len);
		head += len;
	}

	void Buffer::write(const void* src, unsigned len) {
		if (head + len > end) {
			if (stretchy) {
				resize(head - base + len);
			} else {
				logger.error("write out of bounds");
				exit(-1);
			}
		}
		memcpy(head, src, len);
		head += len;
	}

	void Buffer::fill(u8 c) {
		for (u8* p = base; p < end; p++) {
			*p = c;
		}
	}

	void Buffer::resize(unsigned len) {
		if (!stretchy) {
			logger.error("cannot resize non-stretchy buffer");
		} else {
			auto head_offs = tell();
			auto end_offs = size();
			base = (u8*) realloc(base, len);
			head = base + head_offs;
			end = base + len;
		}
	}

	void Buffer::setStretchy(bool enable) {
		if (owned) {
			stretchy = enable;
		} else {
			logger.warn("Failed to set stretchy (data not owned)");
		}
	}

	bool Buffer::isStretchy() {
		return stretchy;
	}
}