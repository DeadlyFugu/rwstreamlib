/*
 * File: rwdump.cc
 * Author: DeadlyFugu
 * License: zlib
 * Description: Dumps RenderWare Binary Stream files as human-readable text
 */

#include <stdio.h>
#include "util.hh"
#include "chunk.hh"

int main(int argc, char** argv) {
	using namespace rw;

	if (argc > 1) {
		util::Buffer b(0);
		util::readFile(argv[1], b);
		Chunk* root = readChunk(b); // note: functions like new Chunk(); - i.e. caller must delete pointer

		bool verbose = false;
		if (argc > 2) {
			verbose = !strcmp(argv[2], "verbose");
		}
		root->dump(util::DumpWriter(verbose));

		delete root;
	} else {
		printf("usage: rwdump <file.rws> [verbose]");
	}
}