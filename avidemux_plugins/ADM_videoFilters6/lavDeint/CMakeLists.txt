INCLUDE(vf_plugin)

SET(ADM_vf_lavDeint_SRCS lavDeint.cpp)

ADD_VIDEO_FILTER(ADM_vf_lavDeint ${ADM_vf_lavDeint_SRCS})
IF(DO_COMMON)
  TARGET_LINK_LIBRARIES(ADM_vf_lavDeint PRIVATE ADM_libpostproc)
ENDIF()
INIT_VIDEO_FILTER(ADM_vf_lavDeint)
INSTALL_VIDEO_FILTER(ADM_vf_lavDeint)
