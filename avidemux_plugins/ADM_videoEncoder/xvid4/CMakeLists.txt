
INCLUDE(ve_plugin)

SET(xvid4_SRCS
        ADM_xvid4.cpp
        xvid4Plugin.cpp
        xvid4Dialog.cpp
)
include(admCheckThreads)
include(admCheckXvid)
checkXvid()
IF(USE_XVID)
  INCLUDE_DIRECTORIES(${XVID_INCLUDE_DIR})
  ADD_VIDEO_ENCODER(ADM_ve_xvid4  ${xvid4_SRCS})
  TARGET_LINK_LIBRARIES(ADM_ve_xvid4 PRIVATE "${XVID_LIBRARY_DIR}")
  ADM_LINK_THREAD(ADM_ve_xvid4)
  INIT_VIDEO_ENCODER(ADM_ve_xvid4)
  INSTALL_VIDEO_ENCODER(ADM_ve_xvid4)
ENDIF(USE_XVID)
