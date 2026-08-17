cmake_minimum_required(VERSION 3.15)
project(WebScraper LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)

# 1. Find CPR (C++ Requests)
find_package(cpr REQUIRED)

# 2. Find Lexbor (C library)
# Lexbor doesn't ship with a standard CMake package config, so we look for its headers/library manually
find_path(LEXBOR_INCLUDE_DIR NAMES lexbor/html/html.h)
find_library(LEXBOR_LIBRARY NAMES lexbor)

if(NOT LEXBOR_INCLUDE_DIR OR NOT LEXBOR_LIBRARY)
    message(FATAL_ERROR "Lexbor library or headers not found!")
endif()

# 3. Define your executable
add_executable(scraper main.cpp)

# 4. Include paths and link the libraries
target_include_directories(scraper PRIVATE ${LEXBOR_INCLUDE_DIR})
target_link_libraries(scraper PRIVATE cpr::cpr ${LEXBOR_LIBRARY})
