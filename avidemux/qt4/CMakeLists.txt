cmake_minimum_required(VERSION 3.7)

PROJECT(Avidemux_Qt)

set(ORIGINAL_CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../../cmake" "${CMAKE_MODULE_PATH}")
include(avidemuxVersion)

MESSAGE("")
MESSAGE("#########################################")
MESSAGE("Configure for Avidemux Qt Started")
MESSAGE("#########################################")
MESSAGE("")
MESSAGE(STATUS "Checking for avidemux development files ..")

IF(NOT FAKEROOT)
  SET(AVIDEMUX_FAKEROOT "")
else(NOT FAKEROOT)
  SET(AVIDEMUX_FAKEROOT "${FAKEROOT}")
endif(NOT FAKEROOT)


IF(WIN32)
  IF(MSVC)
    include(adm_win32_vs.cmake)
  ELSE(MSVC)
    include(adm_win32_mingw.cmake)
  ENDIF(MSVC)
ELSE(WIN32)
  IF(APPLE)
    # Silence missing nullability specifier warnings on macOS
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-nullability-completeness")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-nullability-completeness")
    include(adm_osx.cmake)
  ELSE(APPLE)
    include(adm_default.cmake)
  ENDIF(APPLE)
ENDIF(WIN32)
#-----------------------------------
# Set extra libs, system dependant
#-----------------------------------
ADM_PREBUILD()

# Locate commonCmakeApplication.cmake
SET(ADM_HEADER_DIR ${AVIDEMUX_FAKEROOT}${CMAKE_INSTALL_PREFIX}/include/avidemux/${AVIDEMUX_API_VERSION})
SET(ADM_CMAKE_DIR  ${ADM_HEADER_DIR}/cmake)
SET(CMAKE_MODULE_PATH "${ADM_CMAKE_DIR}" "${ORIGINAL_CMAKE_MODULE_PATH}")
MESSAGE(STATUS  "Checking for avidemux include folder (i.e. CMAKE_INSTALL_PREFIX/include/avidemux/${AVIDEMUX_API_VERSION}, looking in ${ADM_HEADER_DIR}")
if(NOT EXISTS "${ADM_HEADER_DIR}")
  MESSAGE(STATUS  "Make sure you installed all the files.\n i cannot find avidemux include folder.cmake .\nSet CMAKE_INSTALL_PREFIX to the install folder, current value is ${CMAKE_INSTALL_PREFIX}")
  MESSAGE(FATAL_ERROR  "Aborting")
endif(NOT EXISTS "${ADM_HEADER_DIR}")

MESSAGE(STATUS "Found avidemux include folder. good.")
MESSAGE(STATUS "Checking for cmake subfolder")

if(NOT EXISTS "${ADM_CMAKE_DIR}/commonCmakeApplication.cmake")
  MESSAGE(STATUS  "Make sure you installed all the files.\n I cannot find content of the cmake subfolder .\n")
  MESSAGE(STATUS  "Set CMAKE_INSTALL_PREFIX to the install folder, current value is ${CMAKE_INSTALL_PREFIX}")
  MESSAGE(STATUS  "I was looking for commonCmakeApplication.cmake in  ${ADM_CMAKE_DIR}")
  MESSAGE(FATAL_ERROR  "Aborting")
endif(NOT EXISTS "${ADM_CMAKE_DIR}/commonCmakeApplication.cmake")
MESSAGE(STATUS "Found cmake subfolder.good.")

include(commonCmakeApplication)
include(../admAppSettings.cmake)
INCLUDE(admCheckQt)
INCLUDE(admWindRes)

MESSAGE(STATUS "Checking Qt")
checkQt()

IF (NOT QT_FOUND)
  MESSAGE(FATAL_ERROR "Qt NOT FOUND")
ENDIF (NOT QT_FOUND)

# Qt4 openGL
include(admCheckOpenGl)
#
#--
MESSAGE(STATUS "Adding Qt inc paths : <${QT_INCLUDES}> and definitions <${QT_DEFINITIONS}>")
INCLUDE_DIRECTORIES(${QT_INCLUDES})
ADD_DEFINITIONS(${QT_DEFINITIONS})
#--

#
# #########################################
# Config
# #########################################
ADD_DEFINITIONS(-DADM_UI_TYPE_BUILD=ADM_UI_QT4)
SET(CONFIG_HEADER_TYPE ADM_BUILD_QT4)
SET(UI_SUFFIX ${QT_EXTENSION})
IF (${QT_EXTENSION} STREQUAL "qt6")
  SET(CMAKE_CXX_STANDARD 17)
  SET(CMAKE_CXX_STANDARD_REQUIRED ON)
  SET(CMAKE_CXX_EXTENSIONS OFF)
ENDIF (${QT_EXTENSION} STREQUAL "qt6")


CONFIGURE_FILE("${ADM_CMAKE_DIR}/config.h.cmake" "${CMAKE_BINARY_DIR}/config/${QT_EXTENSION}/config.h")
MESSAGE(STATUS "${QT_EXTENSION} config.h generated")

INCLUDE_DIRECTORIES(BEFORE "${CMAKE_BINARY_DIR}/config/${QT_EXTENSION}/")


# ##########################################
# Add job control
# ##########################################
ADD_SUBDIRECTORY(ADM_jobs)
ADD_SUBDIRECTORY(ADM_update)
IF(USE_OPENGL)
  ADD_SUBDIRECTORY(ADM_openGL)
ENDIF(USE_OPENGL)

# ##########################################
# Custom fixed width font for time display
# ##########################################
if (NOT WIN32)
  SET(USE_CUSTOM_TIME_DISPLAY_FONT 1)
endif (NOT WIN32)

# #######################################
# Add common as a symlink or directly
# #######################################
INCLUDE_DIRECTORIES(ADM_UIs/include/)

if (WIN32 AND NOT CROSS)
  ADD_SUBDIRECTORY(../common ./commonQt4)
else (WIN32 AND NOT CROSS)
  # Make symlink else eclipe and kdev4 are puzzled by the tree structure
  # Not needed for plain build
  MESSAGE(STATUS "Creating common symlink in ${CMAKE_CURRENT_SOURCE_DIR}")

  execute_process(COMMAND rm -f common
                                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  execute_process(COMMAND ln -s ../common .
                                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

  ADD_SUBDIRECTORY(common ./commonQt4)
endif (WIN32 AND NOT CROSS)

# Add misc subdirs
ADD_SUBDIRECTORY(ADM_UIs ./ADM_UIsQt4)
ADD_SUBDIRECTORY(ADM_userInterfaces ./ADM_userInterfaces${QT_LIBRARY_EXTENSION})

SDLify(../common/main.cpp)
if (ADM_SUBVERSION)
  ADD_SOURCE_CFLAGS(../common/main.cpp "-DADM_SUBVERSION=\"${ADM_SUBVERSION}\"")
endif (ADM_SUBVERSION)
ADD_SOURCE_CFLAGS(../common/main.cpp "-DADM_VERSION=\"${AVIDEMUX_VERSION}\"")
ADD_SOURCE_CFLAGS(../common/main.cpp "-DQT_FLAVOR=\"${QT_EXTENSION}\"")

# ##########################################
# Icon for windows
# ##########################################
if (WIN32 )
  WINDRESIFY(mainQt  "${CMAKE_CURRENT_SOURCE_DIR}/../common/xpm/adm.ico" src "avidemux" "Avidemux Video Editor")
  SET(ADM_EXE_SRCS ${ADM_EXE_SRCS} ${src})
endif (WIN32)

# ##########################################
# Executable
# ##########################################
include_directories("${PTHREAD_INCLUDE_DIR}")

ADM_MAIN_APP()

# ##########################################
# Construct common libraries
# ##########################################
FOREACH (_libName ${commonLibs1})
  TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ${_libName})
ENDFOREACH (_libName ${commonLibs1})

FOREACH (_libName ${commonLibs2})
  TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ${_libName})
ENDFOREACH (_libName ${commonLibs2})

FOREACH (_libName ${coreLibs})
  TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ${_libName})
ENDFOREACH (_libName ${coreLibs})



# ############################################
# Add qt specific libs
# ############################################
TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION}
 ADM_gui${QT_LIBRARY_EXTENSION}
 ADM_filters${QT_LIBRARY_EXTENSION}
 ADM_UI${QT_LIBRARY_EXTENSION}6
 ADM_dialogQt4
        ADM_update${QT_LIBRARY_EXTENSION}6
 ADM_internalVideoFilter6
 ADM_UI${QT_LIBRARY_EXTENSION}6
 ADM_gui${QT_LIBRARY_EXTENSION}
 ADM_UI_${QT_LIBRARY_EXTENSION}6
 ADM_shell${QT_LIBRARY_EXTENSION}
 ADM_toolkit6
 ADM_coreAudio6
 ADM_coreAudioDevice6
 ADM_coreVideoEncoder6
 ADM_osSupport6
)

# ##########################################
# External libs
# ##########################################
# gettext
IF (GETTEXT_FOUND)
  TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ${GETTEXT_LIBRARY_DIR})
ENDIF (GETTEXT_FOUND)


# SDL
IF (USE_SDL)
  TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ${SDL2_LIBRARY} ${SDL2_MAIN})
ENDIF (USE_SDL)


# ##########################################
# UI Specific
# ##########################################
TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${PTHREAD_LIBRARIES})

TARGET_LINK_LIBRARIES(avidemux3_${QT_EXTENSION} ${PlatformLibs})

#
# i18n
#
ADD_SUBDIRECTORY(i18n)
ADM_LINK_THREAD(avidemux3_${QT_EXTENSION})
# ##########################################
# Install
# ##########################################

DISPLAY_SUMMARY_LIST()

ADM_POSTBUILD()
include(admPackager)
admPackager(qt4Package)
INSTALL(FILES ${CMAKE_BINARY_DIR}/config/${QT_EXTENSION}/config.h DESTINATION "${AVIDEMUX_INCLUDE_DIR}/avidemux/${AVIDEMUX_API_VERSION}/${QT_EXTENSION}" COMPONENT dev)


# install headers
ADM_INSTALL_QT_INCLUDE_FOLDER("${CMAKE_CURRENT_SOURCE_DIR}/ADM_openGL/include/"  ADM_openGL)
ADM_INSTALL_QT_INCLUDE_FOLDER("${CMAKE_CURRENT_SOURCE_DIR}/ADM_UIs/include/"  ADM_UIs)

# install XDG metadata
if(UNIX AND NOT APPLE)
  add_subdirectory(xdg_data)
endif()

#
MESSAGE("")
