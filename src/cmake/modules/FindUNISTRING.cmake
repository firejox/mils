

IF (UNISTRING_INCLUDE_DIRS)
    SET (UNISTRING_FIND_QUIETLY TRUE)
ENDIF (UNISTRING_INCLUDE_DIRS)

FIND_PATH (UNISTRING_INCLUDE_DIRS unistr.h)

SET(UNISTRING_NAMES unistring libunistring)
FIND_LIBRARY (UNISTRING_LIBRARIES NAMES ${UNISTRING_NAMES})

# handle the QUIETLY and REQUIRED arguments and set UNISTRING_FOUND to
# TRUE if all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS (UNISTRING DEFAULT_MSG
    UNISTRING_INCLUDE_DIRS UNISTRING_LIBRARIES)

MARK_AS_ADVANCED (UNISTRING_LIBRARIES UNISTRING_INCLUDE_DIRS)
    
