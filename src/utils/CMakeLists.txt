

find_package (UNISTRING REQUIRED)
set (unicode_dirs ${UNISTRING_INCLUDE_DIRS})
set (unicode_libs ${UNISTRING_LIBRARIES})

set (utils_src 
    line.c
    line.h
)

add_library (utils_obj OBJECT ${utils_src})

target_include_directories (utils_obj PUBLIC ${unicode_dirs})

add_library (utils INTERFACE)
set_property (TARGET utils
    PROPERTY INTERFACE_SOURCES $<TARGET_OBJECTS:utils_obj>)
target_link_libraries (utils
    INTERFACE ${unicode_libs})
