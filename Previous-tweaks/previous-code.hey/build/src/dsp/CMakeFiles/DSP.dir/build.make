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
CMAKE_BINARY_DIR = /Users/wjk/Code/Previous-tweaks/previous-code/build

# Include any dependencies generated for this target.
include src/dsp/CMakeFiles/DSP.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/dsp/CMakeFiles/DSP.dir/compiler_depend.make

# Include the progress variables for this target.
include src/dsp/CMakeFiles/DSP.dir/progress.make

# Include the compile flags for this target's objects.
include src/dsp/CMakeFiles/DSP.dir/flags.make

src/dsp/CMakeFiles/DSP.dir/dsp.c.o: src/dsp/CMakeFiles/DSP.dir/flags.make
src/dsp/CMakeFiles/DSP.dir/dsp.c.o: /Users/wjk/Code/Previous-tweaks/previous-code/src/dsp/dsp.c
src/dsp/CMakeFiles/DSP.dir/dsp.c.o: src/dsp/CMakeFiles/DSP.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/wjk/Code/Previous-tweaks/previous-code/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object src/dsp/CMakeFiles/DSP.dir/dsp.c.o"
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build/src/dsp && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT src/dsp/CMakeFiles/DSP.dir/dsp.c.o -MF CMakeFiles/DSP.dir/dsp.c.o.d -o CMakeFiles/DSP.dir/dsp.c.o -c /Users/wjk/Code/Previous-tweaks/previous-code/src/dsp/dsp.c

src/dsp/CMakeFiles/DSP.dir/dsp.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/DSP.dir/dsp.c.i"
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build/src/dsp && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/wjk/Code/Previous-tweaks/previous-code/src/dsp/dsp.c > CMakeFiles/DSP.dir/dsp.c.i

src/dsp/CMakeFiles/DSP.dir/dsp.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/DSP.dir/dsp.c.s"
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build/src/dsp && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/wjk/Code/Previous-tweaks/previous-code/src/dsp/dsp.c -o CMakeFiles/DSP.dir/dsp.c.s

src/dsp/CMakeFiles/DSP.dir/dsp_core.c.o: src/dsp/CMakeFiles/DSP.dir/flags.make
src/dsp/CMakeFiles/DSP.dir/dsp_core.c.o: /Users/wjk/Code/Previous-tweaks/previous-code/src/dsp/dsp_core.c
src/dsp/CMakeFiles/DSP.dir/dsp_core.c.o: src/dsp/CMakeFiles/DSP.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/wjk/Code/Previous-tweaks/previous-code/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object src/dsp/CMakeFiles/DSP.dir/dsp_core.c.o"
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build/src/dsp && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT src/dsp/CMakeFiles/DSP.dir/dsp_core.c.o -MF CMakeFiles/DSP.dir/dsp_core.c.o.d -o CMakeFiles/DSP.dir/dsp_core.c.o -c /Users/wjk/Code/Previous-tweaks/previous-code/src/dsp/dsp_core.c

src/dsp/CMakeFiles/DSP.dir/dsp_core.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/DSP.dir/dsp_core.c.i"
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build/src/dsp && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/wjk/Code/Previous-tweaks/previous-code/src/dsp/dsp_core.c > CMakeFiles/DSP.dir/dsp_core.c.i

src/dsp/CMakeFiles/DSP.dir/dsp_core.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/DSP.dir/dsp_core.c.s"
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build/src/dsp && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/wjk/Code/Previous-tweaks/previous-code/src/dsp/dsp_core.c -o CMakeFiles/DSP.dir/dsp_core.c.s

src/dsp/CMakeFiles/DSP.dir/dsp_cpu.c.o: src/dsp/CMakeFiles/DSP.dir/flags.make
src/dsp/CMakeFiles/DSP.dir/dsp_cpu.c.o: /Users/wjk/Code/Previous-tweaks/previous-code/src/dsp/dsp_cpu.c
src/dsp/CMakeFiles/DSP.dir/dsp_cpu.c.o: src/dsp/CMakeFiles/DSP.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/wjk/Code/Previous-tweaks/previous-code/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object src/dsp/CMakeFiles/DSP.dir/dsp_cpu.c.o"
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build/src/dsp && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT src/dsp/CMakeFiles/DSP.dir/dsp_cpu.c.o -MF CMakeFiles/DSP.dir/dsp_cpu.c.o.d -o CMakeFiles/DSP.dir/dsp_cpu.c.o -c /Users/wjk/Code/Previous-tweaks/previous-code/src/dsp/dsp_cpu.c

src/dsp/CMakeFiles/DSP.dir/dsp_cpu.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/DSP.dir/dsp_cpu.c.i"
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build/src/dsp && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/wjk/Code/Previous-tweaks/previous-code/src/dsp/dsp_cpu.c > CMakeFiles/DSP.dir/dsp_cpu.c.i

src/dsp/CMakeFiles/DSP.dir/dsp_cpu.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/DSP.dir/dsp_cpu.c.s"
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build/src/dsp && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/wjk/Code/Previous-tweaks/previous-code/src/dsp/dsp_cpu.c -o CMakeFiles/DSP.dir/dsp_cpu.c.s

src/dsp/CMakeFiles/DSP.dir/dsp_disasm.c.o: src/dsp/CMakeFiles/DSP.dir/flags.make
src/dsp/CMakeFiles/DSP.dir/dsp_disasm.c.o: /Users/wjk/Code/Previous-tweaks/previous-code/src/dsp/dsp_disasm.c
src/dsp/CMakeFiles/DSP.dir/dsp_disasm.c.o: src/dsp/CMakeFiles/DSP.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/wjk/Code/Previous-tweaks/previous-code/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building C object src/dsp/CMakeFiles/DSP.dir/dsp_disasm.c.o"
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build/src/dsp && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT src/dsp/CMakeFiles/DSP.dir/dsp_disasm.c.o -MF CMakeFiles/DSP.dir/dsp_disasm.c.o.d -o CMakeFiles/DSP.dir/dsp_disasm.c.o -c /Users/wjk/Code/Previous-tweaks/previous-code/src/dsp/dsp_disasm.c

src/dsp/CMakeFiles/DSP.dir/dsp_disasm.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/DSP.dir/dsp_disasm.c.i"
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build/src/dsp && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/wjk/Code/Previous-tweaks/previous-code/src/dsp/dsp_disasm.c > CMakeFiles/DSP.dir/dsp_disasm.c.i

src/dsp/CMakeFiles/DSP.dir/dsp_disasm.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/DSP.dir/dsp_disasm.c.s"
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build/src/dsp && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/wjk/Code/Previous-tweaks/previous-code/src/dsp/dsp_disasm.c -o CMakeFiles/DSP.dir/dsp_disasm.c.s

# Object files for target DSP
DSP_OBJECTS = \
"CMakeFiles/DSP.dir/dsp.c.o" \
"CMakeFiles/DSP.dir/dsp_core.c.o" \
"CMakeFiles/DSP.dir/dsp_cpu.c.o" \
"CMakeFiles/DSP.dir/dsp_disasm.c.o"

# External object files for target DSP
DSP_EXTERNAL_OBJECTS =

src/dsp/libDSP.a: src/dsp/CMakeFiles/DSP.dir/dsp.c.o
src/dsp/libDSP.a: src/dsp/CMakeFiles/DSP.dir/dsp_core.c.o
src/dsp/libDSP.a: src/dsp/CMakeFiles/DSP.dir/dsp_cpu.c.o
src/dsp/libDSP.a: src/dsp/CMakeFiles/DSP.dir/dsp_disasm.c.o
src/dsp/libDSP.a: src/dsp/CMakeFiles/DSP.dir/build.make
src/dsp/libDSP.a: src/dsp/CMakeFiles/DSP.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/Users/wjk/Code/Previous-tweaks/previous-code/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Linking C static library libDSP.a"
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build/src/dsp && $(CMAKE_COMMAND) -P CMakeFiles/DSP.dir/cmake_clean_target.cmake
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build/src/dsp && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/DSP.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/dsp/CMakeFiles/DSP.dir/build: src/dsp/libDSP.a
.PHONY : src/dsp/CMakeFiles/DSP.dir/build

src/dsp/CMakeFiles/DSP.dir/clean:
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build/src/dsp && $(CMAKE_COMMAND) -P CMakeFiles/DSP.dir/cmake_clean.cmake
.PHONY : src/dsp/CMakeFiles/DSP.dir/clean

src/dsp/CMakeFiles/DSP.dir/depend:
	cd /Users/wjk/Code/Previous-tweaks/previous-code/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/wjk/Code/Previous-tweaks/previous-code /Users/wjk/Code/Previous-tweaks/previous-code/src/dsp /Users/wjk/Code/Previous-tweaks/previous-code/build /Users/wjk/Code/Previous-tweaks/previous-code/build/src/dsp /Users/wjk/Code/Previous-tweaks/previous-code/build/src/dsp/CMakeFiles/DSP.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : src/dsp/CMakeFiles/DSP.dir/depend

