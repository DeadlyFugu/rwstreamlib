/*
 * File: util.hh
 * Author: DeadlyFugu
 * License: zlib
 * Description: Various utility classes including Vector and IOStreams.
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdarg>
#include "buffer.hh"

namespace rw {
	using std::size_t;
	using std::uint32_t;
	using std::uint8_t;

	namespace util {

		using sk::Buffer;

		/// Represents a 2D vector (used for UV coords).
		struct Vector2f {
			float x;
			float y;
		};

		/// Represents a 3D vector (used for vertex positions).
		struct Vector3f {
			float x;
			float y;
			float z;
		};

		/// Represents a 4D vector.
		struct Vector4f {
			float x;
			float y;
			float z;
			float w;
		};

		/// Represents a single face.
		struct Face {
			/// Index into MaterialList.
			uint32_t m_MaterialId;
			/// Indices into vertices.
			uint32_t m_Indices[3];
		};

		template<typename T>
		T min(T a, T b) {
			return (a < b ? a : b);
		}

		template<typename T>
		T max(T a, T b) {
			return (a > b ? a : b);
		}

		template<typename T>
		T clamp(T x, T min, T max) {
			return (x < min ? min : (x > max ? max : x));
		}

		class Logger {
		public:
			/// Represents level of a given log item
			enum LogLevel {
				INFO, WARN, ERROR
			};
			/// Stores function used to print messages
			typedef void (*LoggerCallbackFn)(LogLevel, const char*);
		private:
			/// Callback used to print text used by logger
			LoggerCallbackFn m_CallbackFn;

			/// Internally used by each print function
			const void printFormatted(LogLevel level, const char* format, ...);
		public:
			/// Initializes print callback function
			Logger();

			/// Print an info message
			template<typename... Args>
			const void info(const char* format, Args... args) {
				printFormatted(INFO, format, args...);
			}
			/// Print a warning message
			template<typename... Args>
			const void warn(const char* format, Args... args) {
				printFormatted(WARN, format, args...);
			}
			/// Print an error message
			template<typename... Args>
			const void error(const char* format, Args... args) {
				printFormatted(ERROR, format, args...);
			}

			/// Returns the default function used for printing by the logger
			LoggerCallbackFn getDefaultPrintCallback();
			/// Sets a custom function to be used for printing by the logger
			void setPrintCallback(LoggerCallbackFn callback);
		};

		extern Logger logger;

		bool readFile(const char* filepath, Buffer& buffer);
		bool writeFile(const char* filepath, Buffer& buffer);

		class DumpWriter {
		private:
			int indent;
			bool verbose;
			void (*fnPrintCallback)(const char*);
		public:
			DumpWriter(bool verbose = true);
			DumpWriter(const DumpWriter& parent);
			DumpWriter(void (*fnPrint)(const char*), bool verbose = true);

			const void print(const char* format, ...);

			bool isVerbose();
		};

		void dumpBuffer(Buffer& buf, DumpWriter out);

		uint32_t unpackVersionNumber(uint32_t packedVersion);
		uint32_t unpackBuild(uint32_t packedVersion);
		uint32_t packVersion(uint32_t version, uint32_t build);
	}
}