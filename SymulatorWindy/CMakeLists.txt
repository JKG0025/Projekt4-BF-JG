﻿# CMakeList.txt : CMake project for SymulatorWindy, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("SymulatorWindy")

# Add source to this project's executable.
add_executable (SymulatorWindy "SymulatorWindy.cpp" "SymulatorWindy.h" "GUI.cpp" "GUI.h" "ElevatorLogic.cpp" "ElevatorLogic.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET SymulatorWindy PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries(SymulatorWindy PRIVATE gdiplus)

file(COPY "${CMAKE_SOURCE_DIR}/zdjencia"
     DESTINATION "${CMAKE_BINARY_DIR}")