project(lodepng)
cmake_minimum_required(VERSION 2.8.6)

set(lodepng-source
	${CMAKE_CURRENT_SOURCE_DIR}/lodepng.h
	${CMAKE_CURRENT_SOURCE_DIR}/lodepng.cpp
	)

add_library(lodepng
			${lodepng-source}
	)