IF(WIN32)
	SET(ADM_PLUGIN_DIR "plugins")
    IF(MSVC)
        SET(CMAKE_SHARED_LIBRARY_PREFIX  "lib") # For compatibility with NSIS
    ENDIF()
ELSE()
	SET(ADM_PLUGIN_DIR "ADM_plugins6")
ENDIF()


