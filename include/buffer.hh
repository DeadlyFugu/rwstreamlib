// buffer.hh: Managed memory blocks
// Copyright © DeadlyFugu 2016
#pragma once
#include "util.hh"
#include <string>

namespace sk {
	namespace types {
		typedef uint8_t u8;
		typedef uint16_t u16;
		typedef uint32_t u32;
		typedef uint64_t u64;

		typedef int8_t i8;
		typedef int16_t i16;
		typedef int32_t i32;
		typedef int64_t i64;

		typedef uintptr_t uptr;
		typedef intptr_t iptr;
	}

	using namespace types;

	// Align to 4 byte offset
	inline u32 align32(u32 offs) {
		u32 rem = offs % 4;
		return rem ? offs + 4 - rem : offs;
	}
	// Align to 8 byte offset
	inline u32 align64(u32 offs) {
		u32 rem = offs % 8;
		return rem ? offs + 8 - rem : offs;
	}
	// Align to 16 byte offset
	inline u32 align128(u32 offs) {
		u32 rem = offs % 16;
		return rem ? offs + 16 - rem : offs;
	}
	// Align to 16 byte offset
	inline u64 align128(u64 offs) {
		u64 rem = offs % 16;
		return rem ? offs + 16 - rem : offs;
	}

	class Buffer {
	private:
		u8* base;
		u8* head;
		u8* end;
		bool stretchy, owned;
	public:

		// create new buffer of len len
		// if zeroed is true, data will be set to zero first
		Buffer(unsigned len, bool zeroed = false);

		// create new buffer from existing data
		// if owned is true, this buffer will manage deleting the data afterwards
		Buffer(void* src, unsigned len, bool owned);

		// deletes owned data
		~Buffer();

		// delete copy constructor (must explicitly copy via copy, view, or move)
		Buffer(const Buffer&) = delete;

		// move constructor
		Buffer(Buffer&& other) {
			memcpy(this, &other, sizeof(Buffer));
			other.base = nullptr;
		}

		// returns a view (doesn't own data) copy of the buffer
		Buffer view();

		// returns a view into part of this buffer
		Buffer view(unsigned start, unsigned len);

		// returns a copy (duplicate data, won't modify original) of the buffer
		Buffer copy();

		// returns a copy of part of this buffer
		Buffer copy(unsigned start, unsigned len);

		// set buffer head to pos
		void seek(unsigned pos);

		// increment buffer head by a given amount
		void skip(unsigned bytes);

		// return buffer head pos
		unsigned tell();

		// align buffer head to 2 byte boundary
		inline void align16() {
			if ((head - base) % 2 == 1) head++;
		}

		// align buffer head to 4 byte boundary
		inline void align32() {
			seek(sk::align32(tell()));
		}

		// align buffer head to 8 byte boundary
		inline void align64() {
			seek(sk::align64(tell()));
		}

		// align buffer head to 16 byte boundary
		inline void align128() {
			seek(sk::align128(tell()));
		}

		// return pointer to base
		void* base_ptr();

		// return pointer to head
		void* head_ptr();

		// return size
		unsigned size();

		// return bytes remaining until end
		unsigned remaining();

		// read len bytes from buffer to dst
		void read(void* dst, unsigned len);

		// write len bytes from src to buffer
		void write(const void* src, unsigned len);

		// read a struct to value
		template<typename T>
		void read(T* value) {
			read(value, sizeof(T));
		}

		// write a struct from value
		template<typename T>
		void write(const T& value) {
			write(&value, sizeof(T));
		}

		// write the contents of another buffer into this one
		inline void write(Buffer& other) {
			write(other.base_ptr(), other.size());
		}

		// set all bytes in buffer to a specific value
		void fill(u8 c);

		// resize buffer, only works if stretchy
		void resize(unsigned len);

		// set auto-resize
		void setStretchy(bool enable);

		// test if auto-resize enabled
		bool isStretchy();
	};
}