cmake_minimum_required(VERSION 3.10.0)
project(BallJump VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/release/${CMAKE_SYSTEM_NAME})

set(TARGET ${PROJECT_NAME}-${PROJECT_VERSION})

set(PARENT_DIR ${CMAKE_SOURCE_DIR})

find_package(SDL3 REQUIRED)
# find_package(SDL3_image REQUIRED)
find_package(SDL3_ttf REQUIRED)
find_package(fmt REQUIRED)

add_executable(${TARGET} main.cpp)

target_link_libraries(${TARGET}
                        ${SDL3_LIBRARIES}
                        # SDL3_image::SDL3_image
                        SDL3_ttf::SDL3_ttf
                        fmt::fmt
                    )