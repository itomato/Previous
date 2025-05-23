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

# Include any dependencies generated for this target.
include src/cpu/CMakeFiles/gencpu.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/cpu/CMakeFiles/gencpu.dir/compiler_depend.make

# Include the progress variables for this target.
include src/cpu/CMakeFiles/gencpu.dir/progress.make

# Include the compile flags for this target's objects.
include src/cpu/CMakeFiles/gencpu.dir/flags.make

src/cpu/cpudefs.c: /Users/wjk/Code/Previous-tweaks/previous-code/src/cpu/table68k
src/cpu/cpudefs.c: src/cpu/build68k
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --blue --bold --progress-dir=/Users/wjk/Code/Previous-tweaks/previous-code/build-macos/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Generating cpudefs.c"
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src/cpu && ./build68k < /Users/wjk/Code/Previous-tweaks/previous-code/src/cpu/table68k >cpudefs.c

src/cpu/CMakeFiles/gencpu.dir/gencpu.c.o: src/cpu/CMakeFiles/gencpu.dir/flags.make
src/cpu/CMakeFiles/gencpu.dir/gencpu.c.o: /Users/wjk/Code/Previous-tweaks/previous-code/src/cpu/gencpu.c
src/cpu/CMakeFiles/gencpu.dir/gencpu.c.o: src/cpu/CMakeFiles/gencpu.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/wjk/Code/Previous-tweaks/previous-code/build-macos/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object src/cpu/CMakeFiles/gencpu.dir/gencpu.c.o"
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src/cpu && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT src/cpu/CMakeFiles/gencpu.dir/gencpu.c.o -MF CMakeFiles/gencpu.dir/gencpu.c.o.d -o CMakeFiles/gencpu.dir/gencpu.c.o -c /Users/wjk/Code/Previous-tweaks/previous-code/src/cpu/gencpu.c

src/cpu/CMakeFiles/gencpu.dir/gencpu.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/gencpu.dir/gencpu.c.i"
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src/cpu && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/wjk/Code/Previous-tweaks/previous-code/src/cpu/gencpu.c > CMakeFiles/gencpu.dir/gencpu.c.i

src/cpu/CMakeFiles/gencpu.dir/gencpu.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/gencpu.dir/gencpu.c.s"
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src/cpu && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/wjk/Code/Previous-tweaks/previous-code/src/cpu/gencpu.c -o CMakeFiles/gencpu.dir/gencpu.c.s

src/cpu/CMakeFiles/gencpu.dir/readcpu.c.o: src/cpu/CMakeFiles/gencpu.dir/flags.make
src/cpu/CMakeFiles/gencpu.dir/readcpu.c.o: /Users/wjk/Code/Previous-tweaks/previous-code/src/cpu/readcpu.c
src/cpu/CMakeFiles/gencpu.dir/readcpu.c.o: src/cpu/CMakeFiles/gencpu.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/wjk/Code/Previous-tweaks/previous-code/build-macos/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object src/cpu/CMakeFiles/gencpu.dir/readcpu.c.o"
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src/cpu && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -Wno-unused-variable -Wno-unused-function -Wno-unused-label -Wno-missing-braces -Wno-sign-compare -Wno-unused-but-set-variable -Wno-bad-function-cast -MD -MT src/cpu/CMakeFiles/gencpu.dir/readcpu.c.o -MF CMakeFiles/gencpu.dir/readcpu.c.o.d -o CMakeFiles/gencpu.dir/readcpu.c.o -c /Users/wjk/Code/Previous-tweaks/previous-code/src/cpu/readcpu.c

src/cpu/CMakeFiles/gencpu.dir/readcpu.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/gencpu.dir/readcpu.c.i"
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src/cpu && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -Wno-unused-variable -Wno-unused-function -Wno-unused-label -Wno-missing-braces -Wno-sign-compare -Wno-unused-but-set-variable -Wno-bad-function-cast -E /Users/wjk/Code/Previous-tweaks/previous-code/src/cpu/readcpu.c > CMakeFiles/gencpu.dir/readcpu.c.i

src/cpu/CMakeFiles/gencpu.dir/readcpu.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/gencpu.dir/readcpu.c.s"
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src/cpu && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -Wno-unused-variable -Wno-unused-function -Wno-unused-label -Wno-missing-braces -Wno-sign-compare -Wno-unused-but-set-variable -Wno-bad-function-cast -S /Users/wjk/Code/Previous-tweaks/previous-code/src/cpu/readcpu.c -o CMakeFiles/gencpu.dir/readcpu.c.s

src/cpu/CMakeFiles/gencpu.dir/cpudefs.c.o: src/cpu/CMakeFiles/gencpu.dir/flags.make
src/cpu/CMakeFiles/gencpu.dir/cpudefs.c.o: src/cpu/cpudefs.c
src/cpu/CMakeFiles/gencpu.dir/cpudefs.c.o: src/cpu/CMakeFiles/gencpu.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/wjk/Code/Previous-tweaks/previous-code/build-macos/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building C object src/cpu/CMakeFiles/gencpu.dir/cpudefs.c.o"
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src/cpu && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -Wno-unused-variable -Wno-unused-function -Wno-unused-label -Wno-missing-braces -Wno-sign-compare -Wno-unused-but-set-variable -Wno-bad-function-cast -MD -MT src/cpu/CMakeFiles/gencpu.dir/cpudefs.c.o -MF CMakeFiles/gencpu.dir/cpudefs.c.o.d -o CMakeFiles/gencpu.dir/cpudefs.c.o -c /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src/cpu/cpudefs.c

src/cpu/CMakeFiles/gencpu.dir/cpudefs.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/gencpu.dir/cpudefs.c.i"
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src/cpu && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -Wno-unused-variable -Wno-unused-function -Wno-unused-label -Wno-missing-braces -Wno-sign-compare -Wno-unused-but-set-variable -Wno-bad-function-cast -E /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src/cpu/cpudefs.c > CMakeFiles/gencpu.dir/cpudefs.c.i

src/cpu/CMakeFiles/gencpu.dir/cpudefs.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/gencpu.dir/cpudefs.c.s"
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src/cpu && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -Wno-unused-variable -Wno-unused-function -Wno-unused-label -Wno-missing-braces -Wno-sign-compare -Wno-unused-but-set-variable -Wno-bad-function-cast -S /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src/cpu/cpudefs.c -o CMakeFiles/gencpu.dir/cpudefs.c.s

# Object files for target gencpu
gencpu_OBJECTS = \
"CMakeFiles/gencpu.dir/gencpu.c.o" \
"CMakeFiles/gencpu.dir/readcpu.c.o" \
"CMakeFiles/gencpu.dir/cpudefs.c.o"

# External object files for target gencpu
gencpu_EXTERNAL_OBJECTS =

src/cpu/gencpu: src/cpu/CMakeFiles/gencpu.dir/gencpu.c.o
src/cpu/gencpu: src/cpu/CMakeFiles/gencpu.dir/readcpu.c.o
src/cpu/gencpu: src/cpu/CMakeFiles/gencpu.dir/cpudefs.c.o
src/cpu/gencpu: src/cpu/CMakeFiles/gencpu.dir/build.make
src/cpu/gencpu: src/cpu/CMakeFiles/gencpu.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/Users/wjk/Code/Previous-tweaks/previous-code/build-macos/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Linking C executable gencpu"
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src/cpu && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/gencpu.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/cpu/CMakeFiles/gencpu.dir/build: src/cpu/gencpu
.PHONY : src/cpu/CMakeFiles/gencpu.dir/build

src/cpu/CMakeFiles/gencpu.dir/clean:
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src/cpu && $(CMAKE_COMMAND) -P CMakeFiles/gencpu.dir/cmake_clean.cmake
.PHONY : src/cpu/CMakeFiles/gencpu.dir/clean

src/cpu/CMakeFiles/gencpu.dir/depend: src/cpu/cpudefs.c
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build-macos && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/wjk/Code/Previous-tweaks/previous-code /Users/wjk/Code/Previous-tweaks/previous-code/src/cpu /Users/wjk/Code/Previous-tweaks/previous-code/build-macos /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src/cpu /Users/wjk/Code/Previous-tweaks/previous-code/build-macos/src/cpu/CMakeFiles/gencpu.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : src/cpu/CMakeFiles/gencpu.dir/depend

