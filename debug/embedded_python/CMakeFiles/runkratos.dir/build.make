# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

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
CMAKE_SOURCE_DIR = /home/mengjie/Kratos

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/mengjie/Kratos/cmake_build

# Include any dependencies generated for this target.
include embedded_python/CMakeFiles/runkratos.dir/depend.make

# Include the progress variables for this target.
include embedded_python/CMakeFiles/runkratos.dir/progress.make

# Include the compile flags for this target's objects.
include embedded_python/CMakeFiles/runkratos.dir/flags.make

embedded_python/CMakeFiles/runkratos.dir/krun_main.cpp.o: embedded_python/CMakeFiles/runkratos.dir/flags.make
embedded_python/CMakeFiles/runkratos.dir/krun_main.cpp.o: ../embedded_python/krun_main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mengjie/Kratos/cmake_build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object embedded_python/CMakeFiles/runkratos.dir/krun_main.cpp.o"
	cd /home/mengjie/Kratos/cmake_build/embedded_python && /usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/runkratos.dir/krun_main.cpp.o -c /home/mengjie/Kratos/embedded_python/krun_main.cpp

embedded_python/CMakeFiles/runkratos.dir/krun_main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/runkratos.dir/krun_main.cpp.i"
	cd /home/mengjie/Kratos/cmake_build/embedded_python && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/mengjie/Kratos/embedded_python/krun_main.cpp > CMakeFiles/runkratos.dir/krun_main.cpp.i

embedded_python/CMakeFiles/runkratos.dir/krun_main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/runkratos.dir/krun_main.cpp.s"
	cd /home/mengjie/Kratos/cmake_build/embedded_python && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/mengjie/Kratos/embedded_python/krun_main.cpp -o CMakeFiles/runkratos.dir/krun_main.cpp.s

embedded_python/CMakeFiles/runkratos.dir/krun_main.cpp.o.requires:

.PHONY : embedded_python/CMakeFiles/runkratos.dir/krun_main.cpp.o.requires

embedded_python/CMakeFiles/runkratos.dir/krun_main.cpp.o.provides: embedded_python/CMakeFiles/runkratos.dir/krun_main.cpp.o.requires
	$(MAKE) -f embedded_python/CMakeFiles/runkratos.dir/build.make embedded_python/CMakeFiles/runkratos.dir/krun_main.cpp.o.provides.build
.PHONY : embedded_python/CMakeFiles/runkratos.dir/krun_main.cpp.o.provides

embedded_python/CMakeFiles/runkratos.dir/krun_main.cpp.o.provides.build: embedded_python/CMakeFiles/runkratos.dir/krun_main.cpp.o


# Object files for target runkratos
runkratos_OBJECTS = \
"CMakeFiles/runkratos.dir/krun_main.cpp.o"

# External object files for target runkratos
runkratos_EXTERNAL_OBJECTS =

embedded_python/runkratos: embedded_python/CMakeFiles/runkratos.dir/krun_main.cpp.o
embedded_python/runkratos: embedded_python/CMakeFiles/runkratos.dir/build.make
embedded_python/runkratos: /usr/lib/x86_64-linux-gnu/libpython3.6m.so.1.0
embedded_python/runkratos: embedded_python/CMakeFiles/runkratos.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/mengjie/Kratos/cmake_build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable runkratos"
	cd /home/mengjie/Kratos/cmake_build/embedded_python && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/runkratos.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
embedded_python/CMakeFiles/runkratos.dir/build: embedded_python/runkratos

.PHONY : embedded_python/CMakeFiles/runkratos.dir/build

embedded_python/CMakeFiles/runkratos.dir/requires: embedded_python/CMakeFiles/runkratos.dir/krun_main.cpp.o.requires

.PHONY : embedded_python/CMakeFiles/runkratos.dir/requires

embedded_python/CMakeFiles/runkratos.dir/clean:
	cd /home/mengjie/Kratos/cmake_build/embedded_python && $(CMAKE_COMMAND) -P CMakeFiles/runkratos.dir/cmake_clean.cmake
.PHONY : embedded_python/CMakeFiles/runkratos.dir/clean

embedded_python/CMakeFiles/runkratos.dir/depend:
	cd /home/mengjie/Kratos/cmake_build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/mengjie/Kratos /home/mengjie/Kratos/embedded_python /home/mengjie/Kratos/cmake_build /home/mengjie/Kratos/cmake_build/embedded_python /home/mengjie/Kratos/cmake_build/embedded_python/CMakeFiles/runkratos.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : embedded_python/CMakeFiles/runkratos.dir/depend

