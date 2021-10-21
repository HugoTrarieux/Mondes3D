project(m3d_td2)

cmake_minimum_required(VERSION 2.8.3)

add_subdirectory(ext ext_build)

if(APPLE)
    add_definitions(-DGL_SILENCE_DEPRECATION)
endif()

include_directories(
  # include files
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  # tinyformat string formatting library
  ${TFM_INCLUDE_DIR}
  # Eigen linear algebra library
  SYSTEM ${EIGEN_INCLUDE_DIR}
  # OpenEXR high dynamic range bitmap library
  ${TINYEXR_INCLUDE_DIR}
  # PNG library
  ${LODEPNG_INCLUDE_DIR}
  # PugiXML parser
  ${PUGIXML_INCLUDE_DIR}
  # GLFW library for OpenGL context creation
  ${GLFW_INCLUDE_DIR}
  # GLEW library for accessing OpenGL functions
  ${GLEW_INCLUDE_DIR}
  # NanoVG drawing library
  ${NANOVG_INCLUDE_DIR}
  # NanoGUI user interface library
  ${NANOGUI_INCLUDE_DIR}
  # Portable filesystem API
  ${FILESYSTEM_INCLUDE_DIR}
  # OBJ mesh loader
  ${TINYOBJLOADER_INCLUDE_DIR}
)

add_definitions(-DDATA_DIR="${PROJECT_SOURCE_DIR}/data")

# the list of source files
set(SRCS
    include/common.h
    include/vector.h
    include/block.h
    include/viewer.h
    include/bitmap.h
    include/camera.h
    include/ray.h
    include/object.h
    include/sphere.h
    include/scene.h
    include/mesh.h
    include/bvh.h
    include/shape.h
    include/material.h
    include/light.h
    include/plane.h
    include/sphere.h
    include/color.h
    include/parser.h
    include/proplist.h
    include/transform.h
    include/integrator.h

    src/common.cpp
    src/block.cpp
    src/viewer.cpp
    src/bitmap.cpp
    src/mesh.cpp
    src/bvh.cpp
    src/camera.cpp
    src/main.cpp
    src/object.cpp
    src/shape.cpp
    src/sphere.cpp
    src/scene.cpp
    src/material.cpp
    src/directionalLight.cpp
    src/pointLight.cpp
    src/plane.cpp
    src/phong.cpp
    src/parser.cpp
    src/proplist.cpp
    src/flat.cpp
    src/normals.cpp
    src/direct.cpp
    src/whitted.cpp
    src/texcoords.cpp
)

add_executable(mds3d_raytracer ${SRCS})

target_link_libraries(mds3d_raytracer pugixml lodepng tinyobjloader nanogui ${NANOGUI_EXTRA_LIBS})
