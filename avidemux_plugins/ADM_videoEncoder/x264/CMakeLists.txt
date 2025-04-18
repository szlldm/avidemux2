
INCLUDE(ve_plugin)
INCLUDE(ve_settings_plugin)
ADD_DEFINITIONS(-DX264_USE_UTF8=1)
ADD_DEFINITIONS(-DSIMPLE_MODE_EX=1)

IF(WIN32) # This should be pulled by the .pc file if we were using pkg config. Oh well...
  ADD_DEFINITIONS(-DX264_API_IMPORTS=1)
ENDIF(WIN32)


SET(x264_SRCS
        ADM_x264.cpp
        ADM_x264Plugin.cpp
        ADM_x264Setup.cpp
        x264_encoder_json.cpp
)
include(admCheckX264)
checkX264()
IF(USE_X264)
  MESSAGE(STATUS "x264 plugin activated.")
  INCLUDE_DIRECTORIES(${X264_INCLUDE_DIR})

  #-----------------------QT4 VERSION---------------------
  INCLUDE(ve_plugin_qt4)
  if(DO_QT4)
    SET(ADM_ve_x264_QT4_SRCS    qt4/Q_x264.cpp)
    SET(ADM_ve_x264_QT4_Headers qt4/Q_x264.h)
    SET(ADM_ve_x264_QT4_UI      qt4/x264ConfigDialog)
    ADD_VIDEO_ENCODER_QT4(ADM_ve_x264_${QT_LIBRARY_EXTENSION}   ${ADM_ve_x264_QT4_SRCS} ${ADM_ve_x264_QT4_Headers} ${ADM_ve_x264_QT4_UI} ${x264_SRCS})
    TARGET_LINK_LIBRARIES(ADM_ve_x264_${QT_LIBRARY_EXTENSION} PRIVATE "${X264_LIBRARY_DIR}")
    TARGET_LINK_LIBRARIES(ADM_ve_x264_${QT_LIBRARY_EXTENSION} PRIVATE ADM_core6)
    INIT_VIDEO_ENCODER(ADM_ve_x264_${QT_LIBRARY_EXTENSION})
    INSTALL_VIDEO_ENCODER_QT4(ADM_ve_x264_${QT_LIBRARY_EXTENSION})
  endif(DO_QT4)
  #-----------------------/QT4 VERSION---------------------
  #-----------------------GTK or CLI VERSION---------------------
  if(DO_COMMON)
    ADD_VIDEO_ENCODER(ADM_ve_x264_other   ADM_x264_noui.cpp ${x264_SRCS})
    ADD_TARGET_CFLAGS(ADM_ve_x264_other "-DADM_UI_TYPE_BUILD=1") # CLI!
    TARGET_LINK_LIBRARIES(ADM_ve_x264_other PRIVATE "${X264_LIBRARY_DIR}")
    TARGET_LINK_LIBRARIES(ADM_ve_x264_other PRIVATE ADM_core6)
    INIT_VIDEO_ENCODER(ADM_ve_x264_other)
    INSTALL_VIDEO_ENCODER(ADM_ve_x264_other)
  endif(DO_COMMON)
  # ------------- PRESET -----------
  if(DO_SETTINGS)
    FILE(GLOB json_files ${CMAKE_CURRENT_SOURCE_DIR}/preset/*.json)
    if(json_files)
      message(STATUS "Presets found : ${json_files}")
      INSTALL_VIDEO_ENCODER_SETTINGS(x264 VERSION 3 FILES ${json_files})
    endif(json_files)
  endif(DO_SETTINGS)
ELSE(USE_X264)
  MESSAGE(STATUS "x264 disabled.")
ENDIF(USE_X264)

MESSAGE("")
