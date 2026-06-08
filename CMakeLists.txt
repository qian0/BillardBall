cmake_minimum_required(VERSION 3.20)
project(BillardBall CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# GLM — header-only, fetched at configure time
include(FetchContent)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)
FetchContent_MakeAvailable(glm)

# GLFW — system package
find_package(glfw3 3.3 REQUIRED)

# GLAD — vendored; exposed as a library so both the game and tests can link it
add_library(glad STATIC third_party/glad/src/glad.c)
target_include_directories(glad PUBLIC third_party/glad/include)

# Sources
file(GLOB_RECURSE SOURCES src/*.cpp)

add_executable(BillardBall ${SOURCES})

target_include_directories(BillardBall PRIVATE src third_party)

target_link_libraries(BillardBall PRIVATE
    glad
    glfw
    glm::glm
    dl
)

# Copy shaders and assets next to the binary so relative paths work
add_custom_command(TARGET BillardBall POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:BillardBall>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:BillardBall>/assets
)

# ── Unit tests ────────────────────────────────────────────────────────────────
# OFF by default so a plain cmake --build build only compiles the game.
# Enable with: cmake -S . -B build -DBUILD_TESTS=ON
option(BUILD_TESTS "Build milestone unit tests" OFF)
if(BUILD_TESTS)
    add_subdirectory(unit_tests)
endif()
