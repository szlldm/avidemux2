SET(ADM_LIB ADM_gui2Cli6)

SET(${ADM_LIB}_SRCS
 file_none.cpp  gui_none.cpp  preview_none.cpp)

ADD_LIBRARY(${ADM_LIB} STATIC ${${ADM_LIB}_SRCS})
TARGET_LINK_LIBRARIES(${ADM_LIB} PRIVATE ADM_editor6_interface ADM_ui_interface adm_gettext)
