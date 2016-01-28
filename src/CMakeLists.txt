cmake_minimum_required(VERSION 2.8)

project(mils)

set (CMAKE_COLOR_MAKEFILE ON)
set (CMAKE_VERBOSE_MAKEFILE ON)
set (CMAKE_INCLUDE_CURRENT_DIR TRUE)
set (CMAKE_POSITION_INDEPENDENT_CODE TRUE)


set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")

message (${CMAKE_MODULE_PATH})
add_subdirectory (utils)

add_library (options OBJECT options.c)

#image 
add_library (image OBJECT image.c)
find_package (JPEG REQUIRED)
find_package (Cairo REQUIRED)
set (image_dirs ${image_dirs} 
            PUBLIC ${JPEG_INCLUDE_DIRS}
            PUBLIC ${CAIRO_INCLUDE_DIRS})
set (image_libs ${image_libs}
    ${JPEG_LIBRARIES}
    ${CAIRO_LIBRARIES})
target_include_directories (image ${image_dirs})

#grphical compile
add_library (graphic OBJECT screen.c view.c view_private.c)

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
    )


target_include_directories (graphic ${graphic_dirs})

#input compile
add_library (input OBJECT input.c keytree.c)

find_package (XkbCommon REQUIRED)
find_package (LibInput REQUIRED)
find_package (UDEV REQUIRED)

set (input_dirs ${input_dirs}
    PUBLIC ${XKBCOMMON_INCLUDE_DIRS}
    PUBLIC ${LIBINPUT_INCLUDE_DIRS}
    PUBLIC ${LIBUDEV_INCLUDE_DIRS})

set (input_libs ${input_libs}
        ${XKBCOMMON_LIBRARIES}
        ${LIBINPUT_LIBRARIES}
        ${LIBUDEV_LIBRARIES})

target_include_directories (input ${input_dirs})

#auth compile
add_library (auth OBJECT auth.c auth.h)

find_package (PAM REQUIRED)
set (auth_dirs
    PUBLIC ${PAM_INCLUDE_DIRS})

set (auth_libs ${PAM_LIBRARIES})

target_include_directories (auth ${auth_dirs})


add_library (mils SHARED common
    $<TARGET_OBJECTS:graphic>
    $<TARGET_OBJECTS:image>
    $<TARGET_OBJECTS:input>
    $<TARGET_OBJECTS:options>
    $<TARGET_OBJECTS:auth>)

target_link_libraries (
    mils
    ${graphic_libs}
    ${image_libs}
    ${input_libs}
    ${auth_libs}
    utils
    )

file (COPY ${CMAKE_SOURCE_DIR}/default DESTINATION ${CMAKE_BINARY_DIR}/)

add_executable (Mils main.c)
target_link_libraries (Mils mils)
