
SET(ADMcoreDemuxer_SRCS
ADM_dynaDemuxer.cpp
ADM_demuxer.cpp
ADM_metaToFile.cpp
)	

ADM_ADD_SHARED_LIBRARY(ADM_coreDemuxer6 ${ADMcoreDemuxer_SRCS})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/avidemux/ADM_coreAudio/include)
TARGET_LINK_LIBRARIES(ADM_coreDemuxer6 ADM_coreImage6 ADM_coreAudio6 ADM_core6)

ADM_INSTALL_LIB(ADM_coreDemuxer6)
