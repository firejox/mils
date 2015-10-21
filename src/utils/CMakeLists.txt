

find_package (UNISTRING REQUIRED)
set (unicode_dirs ${UNISTRING_INCLUDE_DIRS})
set (unicode_libs ${UNISTRING_LIBRARIES})

set (utils_src 
    line.c
    line.h
)

add_library (utils ${utils_src})

target_include_directories (utils PUBLIC ${unicode_dirs})
target_link_libraries (utils ${unicode_libs})
