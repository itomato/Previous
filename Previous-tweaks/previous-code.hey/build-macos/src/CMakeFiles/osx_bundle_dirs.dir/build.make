# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.27

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /opt/homebrew/Cellar/cmake/3.27.2/bin/cmake

# The command to remove a file.
RM = /opt/homebrew/Cellar/cmake/3.27.2/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/wjk/Code/Previous-tweaks/previous-code

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/wjk/Code/Previous-tweaks/previous-code/build-macos

# Utility rule file for osx_bundle_dirs.

# Include any custom commands dependencies for this target.
include src/CMakeFiles/osx_bundle_dirs.dir/compiler_depend.make

# Include the progress variables for this target.
include src/CMakeFiles/osx_bundle_dirs.dir/progress.make

src/CMakeFiles/osx_bundle_dirs: /Users/wjk/Code/Previous-tweaks/previous-code/src/gui-osx/Previous.icns
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src && rm -rf /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src/Previous.app/Contents/Resources
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src && rm -rf /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src/Previous.app/Contents/Frameworks
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src && rm -rf /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src/Previous.app/Contents/MacOS
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src && mkdir -p /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src/Previous.app/Contents/Resources
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src && mkdir -p /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src/Previous.app/Contents/Frameworks
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src && mkdir -p /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src/Previous.app/Contents/MacOS
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src && cp /Users/wjk/Code/Previous-tweaks/previous-code/src/gui-osx/Previous.icns /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src/Previous.app/Contents/Resources/Previous.icns
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src && cp -R /Users/wjk/Code/Previous-tweaks/previous-code/src/gui-osx/*.lproj /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src/Previous.app/Contents/Resources/
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src && cp /Users/wjk/Code/Previous-tweaks/previous-code/src/*.BIN /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src/Previous.app/Contents/Resources/
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src && cp -R /Library/Frameworks/SDL2.framework /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src/Previous.app/Contents/Frameworks/
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src && cp -R /opt/homebrew/lib/libpng.dylib /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src/Previous.app/Contents/Frameworks/

osx_bundle_dirs: src/CMakeFiles/osx_bundle_dirs
osx_bundle_dirs: src/CMakeFiles/osx_bundle_dirs.dir/build.make
.PHONY : osx_bundle_dirs

# Rule to build all files generated by this target.
src/CMakeFiles/osx_bundle_dirs.dir/build: osx_bundle_dirs
.PHONY : src/CMakeFiles/osx_bundle_dirs.dir/build

src/CMakeFiles/osx_bundle_dirs.dir/clean:
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src && $(CMAKE_COMMAND) -P CMakeFiles/osx_bundle_dirs.dir/cmake_clean.cmake
.PHONY : src/CMakeFiles/osx_bundle_dirs.dir/clean

src/CMakeFiles/osx_bundle_dirs.dir/depend:
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build-macos && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/wjk/Code/Previous-tweaks/previous-code /Users/wjk/Code/Previous-tweaks/previous-code/src /Users/wjk/Code/Previous-tweaks/previous-code/build-macos /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src/CMakeFiles/osx_bundle_dirs.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : src/CMakeFiles/osx_bundle_dirs.dir/depend

