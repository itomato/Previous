# Install script for directory: /Users/wm/Code/previous-code-635-branches-branch_realtime/python-ui

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE PROGRAM FILES "/Users/wm/Code/previous-code-635-branches-branch_realtime/python-ui/previousui")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/previous/previousui" TYPE PROGRAM FILES
    "/Users/wm/Code/previous-code-635-branches-branch_realtime/python-ui/config.py"
    "/Users/wm/Code/previous-code-635-branches-branch_realtime/python-ui/dialogs.py"
    "/Users/wm/Code/previous-code-635-branches-branch_realtime/python-ui/previous.py"
    "/Users/wm/Code/previous-code-635-branches-branch_realtime/python-ui/uihelpers.py"
    "/Users/wm/Code/previous-code-635-branches-branch_realtime/python-ui/previousui.py"
    "/Users/wm/Code/previous-code-635-branches-branch_realtime/python-ui/debugui.py"
    )
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/previous/previousui" TYPE FILE FILES
    "/Users/wm/Code/previous-code-635-branches-branch_realtime/python-ui/README"
    "/Users/wm/Code/previous-code-635-branches-branch_realtime/python-ui/TODO"
    "/Users/wm/Code/previous-code-635-branches-branch_realtime/python-ui/release-notes.txt"
    "/Users/wm/Code/previous-code-635-branches-branch_realtime/python-ui/previous-icon.png"
    "/Users/wm/Code/previous-code-635-branches-branch_realtime/python-ui/previous.png"
    )
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/32x32/apps" TYPE FILE FILES "/Users/wm/Code/previous-code-635-branches-branch_realtime/python-ui/previous-icon.png")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/applications" TYPE FILE FILES "/Users/wm/Code/previous-code-635-branches-branch_realtime/python-ui/previousui.desktop")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man1" TYPE FILE FILES "/Users/wm/Code/previous-code-635-branches-branch_realtime/python-ui/previousui.1.gz")
endif()

