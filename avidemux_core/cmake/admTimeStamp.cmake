MACRO(ADM_TIMESTAMP src)
 IF(${CMAKE_VERSION} VERSION_GREATER 2.8.11)
        STRING(TIMESTAMP  ${src} "%y%m%d")
 ELSE()
        SET( ${src} "0")
 ENDIF()
ENDMACRO()
