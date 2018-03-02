/*
 * File: world.cc
 * Author: DeadlyFugu
 * License: zlib
 * Description: Classes representing BinMesh PLG, Atomic Section, and World sections
 */

#include "world.hh"

namespace rw {
	void BinMeshPLGChunk::dump(rw::util::DumpWriter out) {
		out.print("BinMesh PLG:");
		out.print("  flags: 0x%08x", flags);
		out.print("  object count: %d", objectCount);
		out.print("  total index count: %d", indexCount);

		int idx = 0;
		for (auto& object : objects) {
			out.print("");
			out.print("  Mesh(%d):", idx++);
			out.print("    mesh index count: %d", object.meshIndexCount);
			out.print("    material: %d", object.material);
			if (out.isVerbose()) {
				out.print("    indices: {");
				for (uint32_t index : object.indices) {
					out.print("      %d", index);
				}
				out.print("    }");
			} else {
				out.print("    indices: <array of %d u32>", object.indices.size());
			}
		}
	}

	void BinMeshPLGChunk::postReadHook() {
		data.seek(0);

		data.read(&flags);
		data.read(&objectCount);
		data.read(&indexCount);

		for (int i = 0; i < objectCount; i++) {
			BinMeshObject object;
			data.read(&object.meshIndexCount);
			data.read(&object.material);
			for (int j = 0; j < object.meshIndexCount; j++) {
				uint32_t index;
				data.read(&index);
				object.indices.push_back(index);
			}
			objects.push_back(object);
		}
	}

	void BinMeshPLGChunk::preWriteHook() {
		StructChunk::preWriteHook();
	}

	void AtomicSectionChunk::dump(util::DumpWriter out) {
		out.print("Atomic Section:");
		out.print("  model flags: %08x", modelFlags);
		out.print("  triangle count: %d", faceCount);
		out.print("  vertex count: %d", vertexCount);
		out.print("");

		out.print("  bbox max: vec3(%f, %f, %f)", bboxMax[0], bboxMax[1], bboxMax[2]);
		out.print("  bbox min: vec3(%f, %f, %f)", bboxMin[0], bboxMin[1], bboxMin[2]);
		if (unknownA != 0x84d9502f) {
			out.print("  unknown a: %08x (irregular)", unknownA);
		} else if (unknownB != 0){
			out.print("  unknown b: %08x (irregular)", unknownB);
		}
		out.print("");

		if (out.isVerbose()) {
			out.print("  vertex positions: {");
			for (auto& pos : vertexPositions) {
				out.print("    vec3(%f, %f, %f)", pos.x, pos.y, pos.z);
			}
			out.print("  }");

			out.print("  vertex colors: {");
			for (auto& color : vertexColors) {
				out.print("    rgba(%d, %d, %d, %d)", color.r, color.g, color.b, color.a);
			}
			out.print("  }");

			out.print("  vertex uvs: {");
			for (auto& uv : vertexUVs) {
				out.print("    vec2(%f, %f)", uv.u, uv.v);
			}
			out.print("  }");

			out.print("  faces: {");
			for (auto& face : faces) {
				out.print("    material(%d) triangle(%d, %d, %d)", face.material, face.vertex1, face.vertex2, face.vertex3);
			}
			out.print("  }");
		} else {
			out.print("  vertex positions: <array of %d vec3>", vertexPositions.size());
			out.print("  vertex colors: <array of %d rgba>", vertexColors.size());
			out.print("  vertex uvs: <array of %d vec2>", vertexUVs.size());
			out.print("  vertex faces: <array of %d faces>", faces.size());
		}

		if (binMeshPLG) {
			out.print("");
			binMeshPLG->dump(out);
		}
	}

	void AtomicSectionChunk::postReadHook() {
		bool structWasSeen = false;
		bool binMeshWasSeen = false;
		binMeshPLG = nullptr;
		for (auto child : children) {
			if (child->type == RW_STRUCT) {
				if (structWasSeen) {
					util::logger.warn("Multiple structs found within Atomic Section");
					continue;
				}
				structWasSeen = true;

				util::Buffer& content = ((StructChunk*) child)->getBuffer();
				content.read(&modelFlags);
				content.read(&faceCount);
				content.read(&vertexCount);
				content.read(&bboxMax);
				content.read(&bboxMin);
				content.read(&unknownA);
				content.read(&unknownB);

				for (int i = 0; i < vertexCount; i++) {
					VertexPosition position;
					content.read(&position);
					vertexPositions.push_back(position);
				}

				for (int i = 0; i < vertexCount; i++) {
					VertexColor color;
					content.read(&color);
					vertexColors.push_back(color);
				}

				for (int i = 0; i < vertexCount; i++) {
					VertexUVs uvs;
					content.read(&uvs);
					vertexUVs.push_back(uvs);
				}

				for (int i = 0; i < faceCount; i++) {
					Face face;
					content.read(&face);
					faces.push_back(face);
				}
			} else if (child->type == RW_EXTENSION) {
				for (auto extension : ((ListChunk*) child)->children) {
					if (extension->type == RW_BINMESH_PLG) {
						if (binMeshWasSeen) {
							util::logger.warn("Multiple BinMesh PLG found within Atomic Section");
							continue;
						}
						binMeshWasSeen = true;

						binMeshPLG = (BinMeshPLGChunk*) extension;
					} else {
						util::logger.warn("Unsupported extension in Atomic Section: %s", getChunkName(extension->type));
					}
				}
			} else {
				util::logger.warn("Unsupported chunk in Atomic Section: %s", getChunkName(child->type));
			}
		}

		if (!structWasSeen) {
			util::logger.warn("Atomic Section is missing struct");
		}
	}

	void AtomicSectionChunk::preWriteHook() {
		ListChunk::preWriteHook();
	}

	bool AtomicSectionChunk::isAtomic() {
		return true;
	}

	void PlaneSectionChunk::dump(util::DumpWriter out) {
		out.print("Plane Section:");
		out.print("  type: %d", type);
		out.print("  value: %f", value);
		out.print("  leftIsAtomic: %s", leftIsAtomic ? "yes" : "no");
		out.print("  rightIsAtomic: %s", leftIsAtomic ? "yes" : "no");
		out.print("  leftValue: %f", leftValue);
		out.print("  rightValue: %f", rightValue);

		out.print("");
		if (left)
			left->dump(out);
		else
			out.print("  left: null");

		out.print("");
		if (right)
			right->dump(out);
		else
			out.print("  right: null");
	}

	void PlaneSectionChunk::postReadHook() {
		bool structWasSeen = false;
		bool leftWasSeen = false;
		bool rightWasSeen = false;
		for (auto child : children) {
			if (child->type == RW_STRUCT) {
				if (structWasSeen) {
					util::logger.warn("Multiple structs found within Plane Section");
					continue;
				}
				structWasSeen = true;

				util::Buffer& content = ((StructChunk*) child)->getBuffer();
				content.read(&type);
				content.read(&value);
				content.read(&leftIsAtomic);
				content.read(&rightIsAtomic);
				content.read(&leftValue);
				content.read(&rightValue);
			} else if (child->type == RW_ATOMIC_SECTION || child->type == RW_PLANE_SECTION) {
				if (!leftWasSeen) {
					leftWasSeen = true;

					left = (AbstractSectionChunk*) child;

					if (leftIsAtomic != left->isAtomic()) {
						util::logger.warn("Left child type does not match struct in Plane Section");
					}
				} else if (!rightWasSeen) {
					rightWasSeen = true;

					right = (AbstractSectionChunk*) child;

					if (rightIsAtomic != right->isAtomic()) {
						util::logger.warn("Right child type does not match struct in Plane Section");
					}
				} else {
					util::logger.warn("Extraneous child section in Plane Section");
				}
			} else if (child->type == RW_EXTENSION) {
				// todo: extensions
			} else {
				util::logger.warn("Unsupported chunk in Plane Section: %s", getChunkName(child->type));
			}
		}

		if (!structWasSeen) {
			util::logger.warn("Plane Section is missing struct");
		}
	}

	void PlaneSectionChunk::preWriteHook() {
		ListChunk::preWriteHook();
	}

	bool PlaneSectionChunk::isAtomic() {
		return false;
	}

	void WorldChunk::dump(util::DumpWriter out) {
		out.print("World:");
		out.print("  unknown a: {0x%x, 0x%x, 0x%x, 0x%x}", unknownA[0], unknownA[1], unknownA[2], unknownA[3]);
		out.print("  face count: %d", faceCount);
		out.print("  vertex count: %d", vertexCount);
		out.print("  unknown b: {0x%x, 0x%x, 0x%x, 0x%x}", unknownB[0], unknownB[1], unknownB[2], unknownB[3]);
		out.print("");

		out.print("  bbox max: vec3(%f, %f, %f)", bboxMax[0], bboxMax[1], bboxMax[2]);
		out.print("  bbox min: vec3(%f, %f, %f)", bboxMin[0], bboxMin[1], bboxMin[2]);
		out.print("");

		materialList->dump(out);
		rootSection->dump(out);
	}

	void WorldChunk::postReadHook() {
		bool structWasSeen = false;
		bool materialListSeen = false;
		bool rootSectionSeen = false;
		for (auto child : children) {
			if (child->type == RW_STRUCT) {
				if (structWasSeen) {
					util::logger.warn("Multiple structs found within World");
					continue;
				}
				structWasSeen = true;

				util::Buffer& content = ((StructChunk*) child)->getBuffer();
				content.read(&unknownA);
				content.read(&faceCount);
				content.read(&vertexCount);
				content.read(&unknownB);
				content.read(&bboxMax);
				content.read(&bboxMin);
			} else if (child->type == RW_MATERIAL_LIST) {
				if (materialListSeen) {
					util::logger.warn("Multiple Material Lists found within World");
					continue;
				}
				materialListSeen = true;
				materialList = (MaterialListChunk*) child;
			} else if (child->type == RW_ATOMIC_SECTION || child->type == RW_PLANE_SECTION) {
				if (rootSectionSeen) {
					util::logger.warn("Multiple root Sections found within World");
					continue;
				}
				rootSectionSeen = true;
				rootSection = (AbstractSectionChunk*) child;
			} else if (child->type == RW_EXTENSION) {
				// todo: extensions
			} else {
				util::logger.warn("Unsupported chunk in World: %s", getChunkName(child->type));
			}
		}

		if (!structWasSeen) {
			util::logger.warn("World is missing struct");
		}
		if (!materialListSeen) {
			util::logger.warn("World is missing Material List");
		}
		if (!rootSectionSeen) {
			util::logger.warn("World is missing root Section");
		}
	}

	void WorldChunk::preWriteHook() {
		ListChunk::preWriteHook();
	}
}
