INCLUDE(vf_plugin)


SET(HistEqCommon_SRCS 
ADM_vidHistEq.cpp  
)
# ---------- QT4 Version ----------------
INCLUDE(vf_plugin_qt4)
SET(HistEqQT_SRCS  qt4/Q_histEq.cpp)
SET(HistEqQT_HEADERS  qt4/Q_histEq.h)
SET(HistEqQT_UI    qt4/histEq)
INIT_VIDEO_FILTER_QT4(ADM_vf_histEq${QT_LIBRARY_EXTENSION} ${HistEqQT_SRCS} ${HistEqQT_HEADERS} ${HistEqQT_UI} ${HistEqCommon_SRCS} qt4/DIA_flyHistEq.cpp)

# /QT4



#------------ Cli Version ----------------
INCLUDE(vf_plugin_cli)
SET(HistEqCli_SRCS cli/DIA_histEq.cpp)
INIT_VIDEO_FILTER_CLI(  ADM_vf_histEqCli ${HistEqCli_SRCS} ${HistEqCommon_SRCS})
#
