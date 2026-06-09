# Unit test targets — each milestone lives in its own subfolder (m1/, m2/, m3/).
# Binaries land in build/unit_tests/<mX>/; shaders are copied there from unit_tests/<mX>/shaders/.
# Shared libraries (glad, glfw, glm) are defined by the parent CMakeLists.txt.
#
# To build all tests:
#   cmake -S . -B build -DBUILD_TESTS=ON
#   cmake --build build --target all
#
# To build a single test:
#   cmake --build build --target m1_triangle

# Registers a test executable under unit_tests/<subdir>/.
# Copies that subdir's shaders/ next to the binary after build.
function(add_unit_test name subdir)
    add_executable(${name}
        ${CMAKE_SOURCE_DIR}/unit_tests/${subdir}/${name}.cpp
        ${CMAKE_SOURCE_DIR}/src/Shader.cpp
        ${ARGN}
    )
    target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(${name} PRIVATE glad glfw glm::glm dl)
    set_target_properties(${name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/unit_tests/${subdir}
    )
    add_custom_command(TARGET ${name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/unit_tests/${subdir}/shaders
            ${CMAKE_BINARY_DIR}/unit_tests/${subdir}/shaders
    )
endfunction()

# Helper: also copies the assets/ folder next to the binary (needed for font files).
function(copy_assets name subdir)
    add_custom_command(TARGET ${name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            ${CMAKE_BINARY_DIR}/unit_tests/${subdir}/assets
    )
endfunction()

# --- M1 ---
add_unit_test(m1_triangle m1)

# --- M2 ---
add_unit_test(m2_ball m2
    ${CMAKE_SOURCE_DIR}/src/Mesh.cpp
    ${CMAKE_SOURCE_DIR}/src/Camera.cpp
    ${CMAKE_SOURCE_DIR}/src/Texture.cpp
    ${CMAKE_SOURCE_DIR}/src/BallTexture.cpp
)
target_include_directories(m2_ball PRIVATE ${CMAKE_SOURCE_DIR}/third_party)
copy_assets(m2_ball m2)

# --- M3 ---
add_unit_test(m3_1_ballOnTable m3
    ${CMAKE_SOURCE_DIR}/src/Mesh.cpp
    ${CMAKE_SOURCE_DIR}/src/Camera.cpp
    ${CMAKE_SOURCE_DIR}/src/Texture.cpp
    ${CMAKE_SOURCE_DIR}/src/BallTexture.cpp
)
target_include_directories(m3_1_ballOnTable PRIVATE ${CMAKE_SOURCE_DIR}/third_party)
copy_assets(m3_1_ballOnTable m3)

# m3_5: full-scene archive — BallScene and TableScene are frozen copies in unit_tests/m3/
# so that M4 changes to src/ do not affect this regression test.
add_unit_test(m3_5_fullScene m3
    ${CMAKE_SOURCE_DIR}/unit_tests/m3/BallScene.cpp
    ${CMAKE_SOURCE_DIR}/unit_tests/m3/TableScene.cpp
    ${CMAKE_SOURCE_DIR}/src/Mesh.cpp
    ${CMAKE_SOURCE_DIR}/src/Camera.cpp
    ${CMAKE_SOURCE_DIR}/src/Texture.cpp
    ${CMAKE_SOURCE_DIR}/src/BallTexture.cpp
)
# unit_tests/m3 listed first so local BallScene.hpp/TableScene.hpp shadow src/ versions.
target_include_directories(m3_5_fullScene BEFORE PRIVATE
    ${CMAKE_SOURCE_DIR}/unit_tests/m3
    ${CMAKE_SOURCE_DIR}/third_party
)
copy_assets(m3_5_fullScene m3)

# --- M4 ---
add_unit_test(m4_1_movingBalls m4
    ${CMAKE_SOURCE_DIR}/src/Physics.cpp
    ${CMAKE_SOURCE_DIR}/src/Mesh.cpp
    ${CMAKE_SOURCE_DIR}/src/Camera.cpp
    ${CMAKE_SOURCE_DIR}/src/Texture.cpp
    ${CMAKE_SOURCE_DIR}/src/BallTexture.cpp
)
target_include_directories(m4_1_movingBalls PRIVATE ${CMAKE_SOURCE_DIR}/third_party)
copy_assets(m4_1_movingBalls m4)
