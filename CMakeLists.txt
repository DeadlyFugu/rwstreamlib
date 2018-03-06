cmake_minimum_required(VERSION 3.7)
project(rwstreamlib)

set(CMAKE_CXX_STANDARD 11)

include_directories(include)

add_library(rwstream
		include/util.hh
		include/buffer.hh
		include/chunk.hh
		include/material.hh
		include/texture.hh
		include/world.hh
		include/animation.hh
		include/geometry.hh

		src/util.cc
		src/buffer.cc
		src/chunk.cc
		src/material.cc
		src/texture.cc
		src/world.cc
		src/animation.cc
		src/geometry.cc
)

add_executable(rwdump
		include/util.hh
		include/chunk.hh
		include/buffer.hh

		test/rwdump.cc
)

target_link_libraries(rwdump rwstream)
