# Copyright 2026 Daniel Glavas
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Build file of ultrasound_scan_execution: one helper library, its gtest, and the linters
# (with cppcheck switched on for this package only).
cmake_minimum_required(VERSION 3.8)
project(ultrasound_scan_execution)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)

add_library(ultrasound_scan_execution_helpers src/path_fraction_check.cpp)
target_include_directories(ultrasound_scan_execution_helpers PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

install(DIRECTORY include/ DESTINATION include)
install(
  TARGETS ultrasound_scan_execution_helpers
  EXPORT export_ultrasound_scan_execution_helpers
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

ament_export_targets(export_ultrasound_scan_execution_helpers HAS_LIBRARY_TARGET)
ament_export_include_directories(include)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # cppcheck is added by hand below, so lint_auto must not add it a second time.
  list(APPEND AMENT_LINT_AUTO_EXCLUDE ament_cmake_cppcheck)
  ament_lint_auto_find_test_dependencies()

  find_package(ament_cmake_cppcheck REQUIRED)
  # Paths to check come first: INCLUDE_DIRS takes every argument after it as an include dir.
  ament_cppcheck(src include test LANGUAGE c++ INCLUDE_DIRS include)
  # Humble's ament_cppcheck skips cppcheck 2.x as "too slow" unless this variable is set.
  # Set it for this one test only, so the franka_* baseline (cppcheck skipped) stays unchanged.
  set_tests_properties(cppcheck PROPERTIES ENVIRONMENT "AMENT_CPPCHECK_ALLOW_SLOW_VERSIONS=1")

  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(test_path_fraction_check test/test_path_fraction_check.cpp)
  target_link_libraries(test_path_fraction_check ultrasound_scan_execution_helpers)
endif()

ament_package()
