# CMakeLists.txt principal
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(EXTRA_COMPONENT_DIRS components)
set(SUPPORTED_TARGETS esp32s3)

project(reptile_keeper)