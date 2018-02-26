/*
 * File: util.cc
 * Author: DeadlyFugu
 * License: zlib
 * Description: Various utility classes including Vector and IOStreams.
 */

#include "util.hh"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <exception>

namespace rw {
	namespace util {
		// Logger impl
		static void defaultLoggerCallback(Logger::LogLevel level, const char* str) {
			static const char* levelNameTable = "INFO\0WARN\0ERR";
			printf("[%s] %s\n", &levelNameTable[level*5], str);
		}

		Logger::Logger() {
			setPrintCallback(getDefaultPrintCallback());
		}

		Logger::LoggerCallbackFn Logger::getDefaultPrintCallback() {
			return defaultLoggerCallback;
		}

		void Logger::setPrintCallback(Logger::LoggerCallbackFn callback) {
			m_CallbackFn = callback;
		}

		const void Logger::printFormatted(Logger::LogLevel level, const char* format, ...) {
			char buffer[512];
			va_list args;
			va_start(args, format);
			vsnprintf(buffer, 512, format, args);
			va_end(args);
			m_CallbackFn(level, buffer);
		}

		Logger logger;

		bool readFile(const char* filename, Buffer& buffer) {
			using namespace std;

			// open file
			FILE* f = fopen(filename, "rb");
			if (!f) {
				logger.error("Unable to open file %s for reading", filename);
				return false;
			}

			// determine file size
			fseek(f, 0, SEEK_END);
			size_t length = (size_t) ftell(f);
			fseek(f, 0, SEEK_SET);

			// read to intermediate memory
			uint8_t* data = (uint8_t*) malloc(length);
			auto result = fread(data, length, 1, f);
			fclose(f);
			if (!result) {
				logger.error("Internal error reading file %s", filename);
				free(data);
				return false;
			}

			// copy to buffer
			bool wasStretchy = buffer.isStretchy();
			int oldOffs = buffer.tell();
			buffer.setStretchy(true);
			buffer.write(data, length);
			buffer.setStretchy(wasStretchy);
			buffer.seek(oldOffs);

			// release intermediate memory
			free(data);
			return true;
		}

		bool writeFile(const char* filepath, Buffer& buffer) {
			using namespace std;
			FILE* f = fopen(filepath, "wb");
			if (!f) {
				logger.warn("Unable to open file %s for writing", filepath);
				return false;
			}

			auto result = fwrite(buffer.base_ptr(), buffer.size(), 1, f);
			if (!result) {
				logger.warn("Internal error writing file %s", filepath);
				fclose(f);
				return false;
			}

			return true;
		}

		static void dumpWriterDefaultCallback(const char* text) {
			printf("%s\n", text);
		}

		DumpWriter::DumpWriter(bool _verbose) : DumpWriter(dumpWriterDefaultCallback, _verbose) {}

		DumpWriter::DumpWriter(const DumpWriter& parent) {
			fnPrintCallback = parent.fnPrintCallback;
			indent = parent.indent + 1;
			verbose = parent.verbose;
		}

		DumpWriter::DumpWriter(void (* fnPrint)(const char*), bool _verbose) {
			fnPrintCallback = fnPrint;
			indent = 0;
			verbose = _verbose;
		}

		const void DumpWriter::print(const char* format, ...) {
			char buffer[512];
			// write indent
			char* p = buffer;
			for (int i = 0; i < indent; i++) {
				*p++ = ' ';
				*p++ = ' ';
			}
			// write formatted string
			va_list args;
			va_start(args, format);
			vsnprintf(p, 512 - indent * 2, format, args);
			va_end(args);
			fnPrintCallback(buffer);
		}

		bool DumpWriter::isVerbose() {
			return verbose;
		}

		void dumpBuffer(Buffer& buf, DumpWriter out) {
			auto start = (uint8_t*) buf.base_ptr();
			auto p = start;
			auto end = start + buf.size();
			char hex_row[] = "........ ........ ........ ........";
			static const char hex_char[] = "0123456789abcdef";
			char ascii_row[] = "................";
			while (p < end) {
				auto local_offs = p - start;
				auto char_offs = 0;
				for (int i = 0; i < 16; i++) {
					uint8_t value = uint8_t(p < end ? *p : 0);
					hex_row[char_offs] = hex_char[value >> 4];
					hex_row[char_offs + 1] = hex_char[value & 0x0f];
					char_offs += (i % 4 == 3 ? 3 : 2);
					ascii_row[i] = isprint(value) ? value : '.';
					p++;
				}
				out.print("[0x%08x] %s  %s", local_offs, hex_row, ascii_row);
			}
		}

		// next 3 functions from https://www.gtamodding.com/wiki/RenderWare

		uint32_t unpackVersionNumber(uint32_t packedVersion) {
			if(packedVersion & 0xFFFF0000)
				return (packedVersion>>14 & 0x3FF00) + 0x30000 |
					   (packedVersion>>16 & 0x3F);
			return packedVersion<<8;
		}

		uint32_t unpackBuild(uint32_t packedVersion) {
			if(packedVersion & 0xFFFF0000)
				return packedVersion & 0xFFFF;
			return 0;
		}

		uint32_t packVersion(uint32_t version, uint32_t build) {
			if (version < 0x31000)
				return version>>8;

			return (version-0x30000 & 0x3FF00) << 14 | (version & 0x3F) << 16 | (build & 0xFFFF);
		}
	}
}