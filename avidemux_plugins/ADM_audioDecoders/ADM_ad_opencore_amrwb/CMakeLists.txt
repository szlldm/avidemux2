# OpenCORE AMR-WB decoder
INCLUDE(admCheckOpencore)
checkOpencoreAmrwb()

IF(OPENCORE_AMRWB_FOUND)
  INCLUDE(ad_plugin)

  SET(ADM_ad_opencore_amrwb_SRCS ADM_ad_opencore_amrwb.cpp)

  ADD_AUDIO_DECODER(ADM_ad_opencore_amrwb ${ADM_ad_opencore_amrwb_SRCS})
  TARGET_LINK_LIBRARIES(ADM_ad_opencore_amrwb PRIVATE  ${OPENCORE_AMRWB_LIBRARY_DIR})

  ADD_TARGET_CFLAGS(ADM_ad_opencore_amrwb "-I${OPENCORE_AMRWB_INCLUDE_DIR}")

  INIT_AUDIO_PLUGIN(ADM_ad_opencore_amrwb)
  INSTALL_AUDIODECODER(ADM_ad_opencore_amrwb)
ENDIF()
