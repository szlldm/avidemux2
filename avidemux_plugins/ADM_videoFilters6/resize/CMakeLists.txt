INCLUDE(vf_plugin)

SET(ADM_vf_swscaleResize_SRCS swScaleResize.cpp)


# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(ADM_vf_swscaleResize_QT4_SRCS    qt4/Q_resize.cpp)
SET(ADM_vf_swscaleResize_QT4_Headers qt4/Q_resize.h)
SET(ADM_vf_swscaleResize_QT4_UI      qt4/resizing)
INIT_VIDEO_FILTER_QT4(ADM_vf_swscaleResize${QT_LIBRARY_EXTENSION} ${ADM_vf_swscaleResize_QT4_SRCS} ${ADM_vf_swscaleResize_QT4_Headers} ${ADM_vf_swscaleResize_QT4_UI} ${ADM_vf_swscaleResize_SRCS})
IF(DO_QT4)
  TARGET_LINK_LIBRARIES(ADM_vf_swscaleResize${QT_LIBRARY_EXTENSION} PRIVATE ADM_libswscale)
ENDIF()
# /QT4


# ----------------- GTK Version -----------------------
INCLUDE(vf_plugin_gtk)
SET(ADM_vf_swscaleResize_GTK_SRCS    gtk/gtk_resize.cpp)
INIT_VIDEO_FILTER_GTK(ADM_vf_swscaleResize_gtk ${ADM_vf_swscaleResize_GTK_SRCS} ${ADM_vf_swscaleResize_SRCS})
IF(DO_GTK)
  TARGET_LINK_LIBRARIES(ADM_vf_swscaleResize_gtk PRIVATE ADM_libswscale)
ENDIF()

# ----------------- CLI Version -----------------------
INCLUDE(vf_plugin_cli)
SET(ADM_vf_swscaleResize_CLI_SRCS    cli/cli_resize.cpp)
INIT_VIDEO_FILTER_CLI(ADM_vf_swscaleResize_cli ${ADM_vf_swscaleResize_SRCS} ${ADM_vf_swscaleResize_CLI_SRCS})
IF(DO_CLI)
  TARGET_LINK_LIBRARIES(ADM_vf_swscaleResize_cli PRIVATE ADM_libswscale)
ENDIF()

# /QT4
