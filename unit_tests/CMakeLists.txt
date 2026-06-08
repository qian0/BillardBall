# Unit test targets — each milestone gets its own executable.
# Binaries land in build/unit_tests/; shaders are copied there from unit_tests/shaders/.
# Shared libraries (glad, glfw, glm) are defined by the parent CMakeLists.txt.
#
# To build all tests:
#   cmake -S . -B build -DBUILD_TESTS=ON
#   cmake --build build --target all
#
# To build a single test:
#   cmake --build build --target m1_triangle

# Registers a test executable that links against the shared game helpers.
# Copies the unit_tests/shaders/ directory next to the binary after build.
function(add_unit_test name)
    add_executable(${name} ${name}.cpp ${CMAKE_SOURCE_DIR}/src/Shader.cpp ${ARGN})
    target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(${name} PRIVATE glad glfw glm::glm dl)
    set_target_properties(${name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/unit_tests
    )
    add_custom_command(TARGET ${name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/unit_tests/shaders
            ${CMAKE_BINARY_DIR}/unit_tests/shaders
    )
endfunction()

add_unit_test(m1_triangle)

add_unit_test(m2_ball
    ${CMAKE_SOURCE_DIR}/src/Mesh.cpp
    ${CMAKE_SOURCE_DIR}/src/Camera.cpp
    ${CMAKE_SOURCE_DIR}/src/Texture.cpp
    ${CMAKE_SOURCE_DIR}/src/BallTexture.cpp
)
target_include_directories(m2_ball PRIVATE ${CMAKE_SOURCE_DIR}/third_party)
# Copy the font asset next to the m2_ball binary
add_custom_command(TARGET m2_ball POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        ${CMAKE_BINARY_DIR}/unit_tests/assets
)
