/*
 * File: geometry.cc
 * Author: DeadlyFugu
 * License: zlib
 * Description: Classes representing Geometry List, and Geometry sections
 */

#include "geometry.hh"

void rw::GeometryChunk::dump(rw::util::DumpWriter out) {
	out.print("Geometry:");
	out.print("  format: %08x", format);
	out.print("  triangle count: %d", triangleCount);
	out.print("  vertex count: %d", vertexCount);
	out.print("  target count: %d", morphTargetCount);

	if (hasSurfaceProperties) {
		out.print("");
		out.print("  ambient: %f", ambient);
		out.print("  specular: %f", specular);
		out.print("  diffuse: %f", diffuse);
	}

	if (out.isVerbose()) {
		if (format & RW_GEOMETRY_PRELIT) {
			out.print("  vertex colors: {");
			for (auto& color : vertexColors) {
				out.print("    rgba(%d, %d, %d, %d)", color.r, color.g, color.b, color.a);
			}
			out.print("  }");
		}

		int i = 0;
		for (auto& vertexUVs : vertexUVLayers) {
			out.print("  vertex uv layer %d: {", i++);
			for (auto& uv : vertexUVs) {
				out.print("    vec2(%f, %f)", uv.u, uv.v);
			}
			out.print("  }");
		}

		out.print("  faces: {");
		for (auto& face : faces) {
			out.print("    material(%d) triangle(%d, %d, %d)", face.material, face.vertex1, face.vertex2, face.vertex3);
		}
		out.print("  }");

		i = 0;
		for (auto& morphTarget : morphTargets) {
			out.print("  morph target %d: {", i++);
			if (morphTarget.hasVertices) {
				out.print("    vertex positions: {");
				for (auto& pos : morphTarget.vertexPositions) {
					out.print("      vec3(%f, %f, %f)", pos.x, pos.y, pos.z);
				}
				out.print("    }");
			}
			if (morphTarget.hasNormals) {
				out.print("    vertex normals: {");
				for (auto& normal : morphTarget.vertexNormals) {
					out.print("      vec3(%f, %f, %f)", normal.x, normal.y, normal.z);
				}
				out.print("    }");
			}
			out.print("  }");
		}

		if (materialList) {
			out.print("");
			materialList->dump(out);
		}

		// todo: dump extensions too (store extensions in vector?) (honestly should do that for all types, built into ListChunk)

		for (auto ext : extensions) {
			out.print("");
			ext->dump(out);
		}
	} else {
		// todo: non-verbose output
	}
}

void rw::GeometryChunk::postReadHook() {
	bool structWasSeen = false;
	bool materialListWasSeen = false;
	materialList = nullptr;
	for (auto child : children) {
		if (child->type == RW_STRUCT) {
			if (structWasSeen) {
				util::logger.warn("Multiple structs found within Geometry");
				continue;
			}
			structWasSeen = true;

			util::Buffer& content = ((StructChunk*) child)->getBuffer();
			content.read(&format);
			content.read(&triangleCount);
			content.read(&vertexCount);
			content.read(&morphTargetCount);

			if (util::unpackVersionNumber(this->version) < 0x34000) {
				content.read(&this->ambient);
				content.read(&this->specular);
				content.read(&this->diffuse);
				this->hasSurfaceProperties = true;
			} else {
				this->hasSurfaceProperties = false;
			}

			if (!(format & RW_GEOMETRY_NATIVE)) {
				if (format & RW_GEOMETRY_PRELIT) {
					for (int i = 0; i < vertexCount; i++) {
						geom::VertexColor color;
						content.read(&color);
						vertexColors.push_back(color);
					}
				}

				if (format & (RW_GEOMETRY_TEXTURED | RW_GEOMETRY_TEXTURED2)) {
					int numTexSets = (format & 0x00ff0000) >> 16;
					if (!numTexSets) numTexSets = (format & RW_GEOMETRY_TEXTURED) ? 1 : 2;
					for (int layer = 0; layer < numTexSets; layer++) {
						vertexUVLayers.emplace_back();
						auto& vertexUVs = vertexUVLayers.back();
						for (int i = 0; i < vertexCount; i++) {
							geom::VertexUVs uvs;
							content.read(&uvs);
							vertexUVs.push_back(uvs);
						}
					}
				}

				for (int i = 0; i < triangleCount; i++) {
					geom::Face face;
					content.read(&face);
					// swap vertex 2 and material indices (Geometry sections  store these swapped)
					auto v2 = face.material;
					face.material = face.vertex2;
					face.vertex2 = v2;
					faces.push_back(face);
				}
			}

			for (int target = 0; target < morphTargetCount; target++) {
				morphTargets.emplace_back();
				auto& morphTarget = morphTargets.back();

				content.read(&morphTarget.boundingSphere);
				content.read(&morphTarget.hasVertices);
				content.read(&morphTarget.hasNormals);

				if (morphTarget.hasVertices) {
					for (int i = 0; i < vertexCount; i++) {
						geom::VertexPosition pos;
						content.read(&pos);
						morphTarget.vertexPositions.push_back(pos);
					}
				}
				if (morphTarget.hasNormals) {
					for (int i = 0; i < vertexCount; i++) {
						geom::VertexNormal normal;
						content.read(&normal);
						morphTarget.vertexNormals.push_back(normal);
					}
				}
			}

			if (content.remaining()) {
				util::logger.warn("Excess data in Geometry struct");
			}
		} else if (child->type == RW_MATERIAL_LIST) {
			if (materialListWasSeen) {
				util::logger.warn("Multiple Material Lists found within Geometry");
				continue;
			}
			materialListWasSeen = true;
			materialList = ((MaterialListChunk*) child);
		} else if (child->type == RW_EXTENSION) {
			extensions.push_back(child);
		} else {
			util::logger.warn("Unsupported chunk in Geometry: %s", getChunkName(child->type));
		}
	}

	if (!structWasSeen) {
		util::logger.warn("Geometry is missing struct");
	}
	if (!materialListWasSeen) {
		util::logger.warn("Geometry is missing Material List");
	}
}

void rw::GeometryChunk::preWriteHook() {
	ListChunk::preWriteHook();
}

void rw::GeometryListChunk::dump(rw::util::DumpWriter out) {
	out.print("Geometry List: (%d geometries)", geometries.size());
	for (auto geometry : geometries) {
		geometry->dump(out);
	}
}

void rw::GeometryListChunk::postReadHook() {
	bool structWasSeen = false;
	uint32_t geometryCount = 0;
	for (auto child : children) {
		if (child->type == RW_STRUCT) {
			if (structWasSeen) {
				util::logger.warn("Multiple structs found within Geometry List");
				continue;
			}
			structWasSeen = true;

			util::Buffer& content = ((StructChunk*) child)->getBuffer();
			content.read(&geometryCount);
		} else if (child->type == RW_GEOMETRY) {
			geometries.push_back((GeometryChunk*) child);
		} else if (child->type == RW_EXTENSION) {
			// todo: extensions
		} else {
			util::logger.warn("Unsupported chunk in Geometry List: %s", getChunkName(child->type));
		}
	}

	if (!structWasSeen) {
		util::logger.warn("Geometry List is missing struct");
	} else if (geometryCount != geometries.size()) {
		util::logger.warn("Geometry List actual children count %d does not match header (%d)", geometries.size(), geometryCount);
	}
}

void rw::GeometryListChunk::preWriteHook() {
	ListChunk::preWriteHook();
}

void rw::FrameListChunk::dump(rw::util::DumpWriter out) {
	out.print("Frame List: (%d frames)", frames.size());
	int i = 0;
	for (auto& frame : frames) {
		if (i != 0) out.print("");
		out.print("  frame %d:", i++);
		out.print("    rotation:  %5.3f, %5.3f, %5.3f", frame.rotation.row1.x, frame.rotation.row1.y, frame.rotation.row1.z);
		out.print("               %5.3f, %5.3f, %5.3f", frame.rotation.row2.x, frame.rotation.row2.y, frame.rotation.row2.z);
		out.print("               %5.3f, %5.3f, %5.3f", frame.rotation.row3.x, frame.rotation.row3.y, frame.rotation.row3.z);
		out.print("    translation: %g, %g, %g", frame.translation.x, frame.translation.y, frame.translation.z);
		out.print("");
		out.print("    previous: %d", frame.previous);
		out.print("    matrix flags: %d", frame.matrixFlags);
	}
}

void rw::FrameListChunk::postReadHook() {
	bool structWasSeen = false;
	for (auto child : children) {
		if (child->type == RW_STRUCT) {
			if (structWasSeen) {
				util::logger.warn("Multiple structs found within Frame List");
				continue;
			}
			structWasSeen = true;

			util::Buffer& content = ((StructChunk*) child)->getBuffer();
			uint32_t frameCount = 0;
			content.read(&frameCount);

			for (int i = 0; i < frameCount; i++) {
				frames.emplace_back();
				content.read(&frames[i]);
			}
		} else if (child->type == RW_EXTENSION) {
			// todo: extensions
		} else {
			util::logger.warn("Unsupported chunk in Frame List: %s", getChunkName(child->type));
		}
	}

	if (!structWasSeen) {
		util::logger.warn("Frame List is missing struct");
	}
}

void rw::FrameListChunk::preWriteHook() {
	ListChunk::preWriteHook();
}

void rw::AtomicChunk::dump(rw::util::DumpWriter out) {
	out.print("Atomic:");
	out.print("  frame: %d", frameIndex);
	out.print("  geometry: %d", geometryIndex);
	out.print("  flags: %08x", flags);
	if (unused != 0) out.print("  unused: %08x (unusual)");
}

void rw::AtomicChunk::postReadHook() {
	bool structWasSeen = false;
	for (auto child : children) {
		if (child->type == RW_STRUCT) {
			if (structWasSeen) {
				util::logger.warn("Multiple structs found within Atomic");
				continue;
			}
			structWasSeen = true;

			util::Buffer& content = ((StructChunk*) child)->getBuffer();
			content.read(&frameIndex, 16);

		} else if (child->type == RW_EXTENSION) {
			// todo: extensions
		} else {
			util::logger.warn("Unsupported chunk in Atomic: %s", getChunkName(child->type));
		}
	}

	if (!structWasSeen) {
		util::logger.warn("Atomic is missing struct");
	}
}

void rw::AtomicChunk::preWriteHook() {
	ListChunk::preWriteHook();
}

void rw::ClumpChunk::dump(rw::util::DumpWriter out) {
	out.print("Clump: (%d atomics)", atomics.size());
	frameList->dump(out);
	out.print("");
	geometryList->dump(out);
	for (auto atomic : atomics) {
		out.print("");
		atomic->dump(out);
	}
	for (auto ext : extensions) {
		out.print("");
		ext->dump(out);
	}
}

void rw::ClumpChunk::postReadHook() {
	bool structWasSeen = false;
	bool frameListSeen = false;
	bool geometryListSeen = false;
	for (auto child : children) {
		if (child->type == RW_STRUCT) {
			if (structWasSeen) {
				util::logger.warn("Multiple structs found within Clump");
				continue;
			}
			structWasSeen = true;

			util::Buffer& content = ((StructChunk*) child)->getBuffer();
			content.read(&atomicCount);
			if (util::unpackVersionNumber(this->version) < 0x33000) {
				content.read(&lightCount);
				content.read(&cameraCount);
			}
		} else if (child->type == RW_FRAME_LIST) {
			if (frameListSeen) {
				util::logger.warn("Multiple Frame Lists found within Clump");
				continue;
			}
			frameListSeen = true;
			frameList = (FrameListChunk*) child;
		} else if (child->type == RW_GEOMETRY_LIST) {
			if (geometryListSeen) {
				util::logger.warn("Multiple Geometry Lists found within Clump");
				continue;
			}
			geometryListSeen = true;
			geometryList = (GeometryListChunk*) child;
		} else if (child->type == RW_ATOMIC) {
			atomics.push_back((AtomicChunk*) child);
		} else if (child->type == RW_EXTENSION) {
			extensions.push_back(child);
		} else {
			util::logger.warn("Unsupported chunk in Clump: %s", getChunkName(child->type));
		}
	}

	if (!structWasSeen) {
		util::logger.warn("Clump is missing struct");
	}
	if (!frameListSeen) {
		util::logger.warn("Clump is missing Frame List");
	}
	if (!geometryListSeen) {
		util::logger.warn("Clump is missing Geometry List");
	}
	if (atomics.size() != atomicCount) {
		util::logger.warn("Clump actual Atomic count %d does not match header (%d)", atomics.size(), atomicCount);
	}
}

void rw::ClumpChunk::preWriteHook() {
	ListChunk::preWriteHook();
}

void rw::DeltaMorphPLGChunk::dump(util::DumpWriter out) {
	out.print("Delta Morph PLG:");
	out.print("  target count: %d", targets.size());

	int idx = 0;
	for (auto& target : targets) {
		out.print("");
		out.print("  Target(%d):", idx++);
		out.print("    name: %s", target.name.c_str());
		out.print("    flags: %d", target.flags);
		out.print("    num2: %d", target.num2);
		if (out.isVerbose()) {
			out.print("    mapping: {");
			for (uint8_t byte : target.mapping) {
				if (byte & 0x80) {
					out.print("      + %d", byte & 0x7f);
				} else {
					out.print("      skip %d", byte & 0x7f);
				}
			}
			out.print("    }");
		} else {
			out.print("    mapping: <%d bytes>", target.mapping.size());
		}
		if (out.isVerbose()) {
			out.print("    vertex count: %d", target.vertices.size());
			out.print("    vertices: {");
			for (auto& vertex : target.vertices) {
				out.print("      (%f, %f, %f)", vertex.x, vertex.y, vertex.z);
			}
			out.print("    }");
		} else {
			out.print("    vertices: <array of %d vec3f>", target.vertices.size());
		}
		if (out.isVerbose()) {
			out.print("    normals: {");
			for (auto& normal : target.normals) {
				out.print("      (%f, %f, %f)", normal.x, normal.y, normal.z);
			}
			out.print("    }");
		} else {
			out.print("    normals: <array of %d vec3f>", target.normals.size());
		}
		out.print("    bounding centre: (%f, %f, %f)",
				  target.boundX, target.boundY, target.boundZ);
		out.print("    bounding radius: %f", target.boundRadius);
	}
}

void rw::DeltaMorphPLGChunk::postReadHook() {
	data.seek(0);

	uint32_t targetCount;
	data.read(&targetCount);

	for (int i = 0; i < targetCount; i++) {
		uint32_t nameLength;
		data.read(&nameLength);

		char* nameBuffer = new char[nameLength];
		data.read(nameBuffer, nameLength);
		std::string name(nameBuffer);
		delete[] nameBuffer;

		DMorphTarget target;
		target.name = name;
		data.read(&target.flags);
		data.read(&target.num2);

		uint32_t mappingLength;
		uint32_t pointCount;
		data.read(&mappingLength);
		data.read(&pointCount);

		for (int j = 0; j < mappingLength; j++) {
			uint8_t value;
			data.read(&value);
			target.mapping.push_back(value);
		}

		for (int j = 0; j < pointCount; j++) {
			DMorphPoint point;
			data.read(&point);
			target.vertices.push_back(point);
		}

		if (target.flags & 0x10) {
			for (int j = 0; j < pointCount; j++) {
				DMorphPoint point;
				data.read(&point);
				target.normals.push_back(point);
			}
		}

		data.read(&target.boundX);
		data.read(&target.boundY);
		data.read(&target.boundZ);
		data.read(&target.boundRadius);

		targets.push_back(target);
	}
}

void rw::DeltaMorphPLGChunk::preWriteHook() {
	StructChunk::preWriteHook();
}
