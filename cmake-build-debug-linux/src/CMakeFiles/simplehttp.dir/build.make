# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /tmp/tmp.vrtMHvKVaH

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /tmp/tmp.vrtMHvKVaH/cmake-build-debug-linux

# Include any dependencies generated for this target.
include src/CMakeFiles/simplehttp.dir/depend.make

# Include the progress variables for this target.
include src/CMakeFiles/simplehttp.dir/progress.make

# Include the compile flags for this target's objects.
include src/CMakeFiles/simplehttp.dir/flags.make

# Object files for target simplehttp
simplehttp_OBJECTS =

# External object files for target simplehttp
simplehttp_EXTERNAL_OBJECTS = \
"/tmp/tmp.vrtMHvKVaH/cmake-build-debug-linux/src/networking/CMakeFiles/simplehttp_networking.dir/socket.cpp.o" \
"/tmp/tmp.vrtMHvKVaH/cmake-build-debug-linux/src/networking/CMakeFiles/simplehttp_networking.dir/http_server.cpp.o" \
"/tmp/tmp.vrtMHvKVaH/cmake-build-debug-linux/src/networking/CMakeFiles/simplehttp_networking.dir/connection_manager.cpp.o" \
"/tmp/tmp.vrtMHvKVaH/cmake-build-debug-linux/src/networking/CMakeFiles/simplehttp_networking.dir/message_parser.cpp.o"

src/libsimplehttp.a: src/networking/CMakeFiles/simplehttp_networking.dir/socket.cpp.o
src/libsimplehttp.a: src/networking/CMakeFiles/simplehttp_networking.dir/http_server.cpp.o
src/libsimplehttp.a: src/networking/CMakeFiles/simplehttp_networking.dir/connection_manager.cpp.o
src/libsimplehttp.a: src/networking/CMakeFiles/simplehttp_networking.dir/message_parser.cpp.o
src/libsimplehttp.a: src/CMakeFiles/simplehttp.dir/build.make
src/libsimplehttp.a: src/CMakeFiles/simplehttp.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/tmp/tmp.vrtMHvKVaH/cmake-build-debug-linux/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Linking CXX static library libsimplehttp.a"
	cd /tmp/tmp.vrtMHvKVaH/cmake-build-debug-linux/src && $(CMAKE_COMMAND) -P CMakeFiles/simplehttp.dir/cmake_clean_target.cmake
	cd /tmp/tmp.vrtMHvKVaH/cmake-build-debug-linux/src && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/simplehttp.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/CMakeFiles/simplehttp.dir/build: src/libsimplehttp.a

.PHONY : src/CMakeFiles/simplehttp.dir/build

src/CMakeFiles/simplehttp.dir/clean:
	cd /tmp/tmp.vrtMHvKVaH/cmake-build-debug-linux/src && $(CMAKE_COMMAND) -P CMakeFiles/simplehttp.dir/cmake_clean.cmake
.PHONY : src/CMakeFiles/simplehttp.dir/clean

src/CMakeFiles/simplehttp.dir/depend:
	cd /tmp/tmp.vrtMHvKVaH/cmake-build-debug-linux && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /tmp/tmp.vrtMHvKVaH /tmp/tmp.vrtMHvKVaH/src /tmp/tmp.vrtMHvKVaH/cmake-build-debug-linux /tmp/tmp.vrtMHvKVaH/cmake-build-debug-linux/src /tmp/tmp.vrtMHvKVaH/cmake-build-debug-linux/src/CMakeFiles/simplehttp.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/CMakeFiles/simplehttp.dir/depend
