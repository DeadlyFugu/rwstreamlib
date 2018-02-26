
#include "chunk.hh"
#include "util.hh"

int main() {
	using namespace rw;

	util::Buffer b(0);
//	util::readFile("/Users/matt/Workspace/Heroes/dvdroot/s29.one/S29_ON_01.bsp", b);
	util::readFile("/Users/matt/Shared/TestTXDs/s29.txd", b);
	Chunk* root = readChunk(b); // note: functions like new Chunk(); - i.e. caller must delete pointer

	root->dump(util::DumpWriter(false));

	delete root;
}