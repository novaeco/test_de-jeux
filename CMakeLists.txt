# CMakeLists.txt principal pour ESP32-S3
cmake_minimum_required(VERSION 3.16)

# Configuration du target ESP32-S3
set(IDF_TARGET esp32s3)

# Inclusion du système de build ESP-IDF
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Définition du projet
project(reptile_keeper)