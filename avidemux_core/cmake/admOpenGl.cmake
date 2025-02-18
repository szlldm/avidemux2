MACRO(ADM_OPEN_GL target)
  IF(USE_OPENGL)
    TARGET_INCLUDE_DIRECTORIES(${target} PRIVATE ${QT_HEADERS_DIR})
    TARGET_INCLUDE_DIRECTORIES(${target} PRIVATE ${OPENGL_INCLUDE_DIR})
    TARGET_INCLUDE_DIRECTORIES(${target} PRIVATE ${QT_QTOPENGL_INCLUDE_DIR})
    TARGET_INCLUDE_DIRECTORIES(${target} PRIVATE ${AVIDEMUX_CORE_SOURCE_DIR}/avidemux/qt4/ADM_openGL/include/)
    TARGET_LINK_LIBRARIES(${target} PUBLIC ${OPENGL_LIBRARIES} ${QT_QTOPENGL_LIBRARIES} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY}  ADM_openGL${QT_LIBRARY_EXTENSION}6)
  ENDIF()
ENDMACRO()
