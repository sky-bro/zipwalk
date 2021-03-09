cmake_minimum_required(VERSION 3.5)

project(zipwalk
        VERSION 1.0
        LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

# or: cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ../src
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

configure_file(config.h.in config.h)

find_package(ZLIB)

add_subdirectory(lib)
add_subdirectory(test)

add_executable(zipwalk main.c)

target_link_libraries(zipwalk PUBLIC zipwalk-core ZLIB::ZLIB)

target_include_directories(zipwalk PUBLIC ${PROJECT_BINARY_DIR})

message(${CMAKE_INSTALL_PREFIX}) # /usr/local

install(TARGETS zipwalk RUNTIME DESTINATION bin)
