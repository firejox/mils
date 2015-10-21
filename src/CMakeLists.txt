cmake_minimum_required(VERSION 2.8)

project(mils)

set (CMAKE_COLOR_MAKEFILE ON)
set (CMAKE_VERBOSE_MAKEFILE ON)
set (CMAKE_INCLUDE_CURRENT_DIR TRUE)


set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")

message (${CMAKE_MODULE_PATH})
add_subdirectory (utils)

add_library (common SHARED common.c)

add_library (options options.c)
target_link_libraries (options common)

#image 
add_library (image image.c)
find_package (JPEG REQUIRED)
set (image_dirs ${image_dirs} PUBLIC ${JPEG_INCLUDE_DIRS})
set (image_libs ${image_libs} ${JPEG_LIBRARIES})
find_package (Cairo REQUIRED)
set (image_dirs ${image_dirs} PUBLIC ${CAIRO_INCLUDE_DIRS})
set (image_libs ${image_libs} ${CAIRO_LIBRARIES})
target_include_directories (image ${image_dirs})
target_link_libraries (image ${image_libs})

#grphical compile
add_library (graphic screen.c view.c view_private.c)

find_package (DRM REQUIRED)
find_package (GBM REQUIRED)
find_package (EGL REQUIRED)
find_package (PANGOCAIRO REQUIRED)

set (graphic_dirs 
    PUBLIC ${LIBDRM_INCLUDE_DIRS}
    PUBLIC ${GBM_INCLUDE_DIRS}
    PUBLIC ${EGL_INCLUDE_DIRS}
    PUBLIC ${PANGOCAIRO_INCLUDE_DIRS} )

set (graphic_libs
    ${LIBDRM_LIBRARIES}
    ${GBM_LIBRARIES}
    ${EGL_LIBRARIES}
    ${PANGOCAIRO_LIBRARIES}
    options image utils)

message ("graphic : ${graphic_dirs}")

target_include_directories (graphic ${graphic_dirs})
target_link_libraries (graphic ${graphic_libs})

#input compile
add_library (input input.c keytree.c)

find_package (XkbCommon REQUIRED)
set (input_dirs ${input_dirs} PUBLIC ${XKBCOMMON_INCLUDE_DIRS})
set (input_libs ${input_libs} ${XKBCOMMON_LIBRARIES})

find_package (LibInput REQUIRED)
set (input_dirs ${input_dirs} PUBLIC ${LIBINPUT_INCLUDE_DIRS})
set (input_libs ${input_libs} ${LIBINPUT_LIBRARIES})

find_package (UDEV REQUIRED)
set (input_dirs ${input_dirs} PUBLIC ${LIBUDEV_INCLUDE_DIRS})
set (input_libs ${input_libs} ${LIBUDEV_LIBRARIES})

set (input_libs ${input_libs} common)

message (" input :${input_dirs}")

target_include_directories (input ${input_dirs})
target_link_libraries (input ${input_libs})




add_executable (mils main.c)
target_link_libraries (mils graphic input utils)
