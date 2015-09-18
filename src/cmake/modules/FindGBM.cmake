
IF (GBM_INCLUDE_DIRS AND GBM_LIBRARIES)
    SET(GBM_FOUND TRUE)
ELSE (GBM_INCLUDE_DIRS AND GBM_LIBRARIES)
    IF (NOT WIN32)
        FIND_PACKAGE (PkgConfig)
        IF (PKG_CONFIG_FOUND)
            pkg_check_modules(_GBM_PC QUIET gbm)
        ENDIF (PKG_CONFIG_FOUND)
    ENDIF (NOT WIN32)

    FIND_PATH (GBM_INCLUDE_DIRS gbm.h 
        ${_GBM_PC_INCLUDE_DIRS})

    FIND_LIBRARY (GBM_LIBRARIES NAMES gbm
        PATH ${_GBM_PC_LIBDIR})

    IF (GBM_INCLUDE_DIRS AND GBM_LIBRARIES)
        SET(GBM_FOUND TRUE)
    ENDIF (GBM_INCLUDE_DIRS AND GBM_LIBRARIES)

    IF (GBM_FOUND)
        IF (NOT GBM_FIND_QUIETLY)
            MESSAGE (STATUS "Found gbm : ${GBM_LIBRARIES}")
        ENDIF (NOT GBM_FIND_QUIETLY)
    ELSE (GBM_FOUND)
        IF (NOT GBM_FIND_QUIETLY)
            MESSAGE (FATAL_ERROR "Could NOT find gbm")
        ENDIF (NOT GBM_FIND_QUIETLY)
    ENDIF (GBM_FOUND)

    MARK_AS_ADVANCED(GBM_INCLUDE_DIRS GBM_LIBRARIES)

ENDIF (GBM_INCLUDE_DIRS AND GBM_LIBRARIES)