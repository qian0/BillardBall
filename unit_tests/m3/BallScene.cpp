#include "BallScene.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

#include "BallTexture.hpp"
#include "Table.hpp"

// Base colour for each ball number (0 = cue ball, 1–15 = object balls).
// Stripes 9–15 share RGB with their solid counterparts; visual distinction comes in M8.
static const glm::vec3 kBallColor[16] = {
    {1.00f, 1.00f, 1.00f},  // 0  cue
    {0.95f, 0.80f, 0.00f},  // 1  yellow
    {0.05f, 0.15f, 0.80f},  // 2  blue
    {0.80f, 0.05f, 0.05f},  // 3  red
    {0.40f, 0.00f, 0.55f},  // 4  purple
    {0.90f, 0.40f, 0.00f},  // 5  orange
    {0.05f, 0.45f, 0.10f},  // 6  green
    {0.50f, 0.05f, 0.10f},  // 7  maroon
    {0.08f, 0.08f, 0.08f},  // 8  black
    {0.95f, 0.80f, 0.00f},  // 9  yellow stripe
    {0.05f, 0.15f, 0.80f},  // 10 blue stripe
    {0.80f, 0.05f, 0.05f},  // 11 red stripe
    {0.40f, 0.00f, 0.55f},  // 12 purple stripe
    {0.90f, 0.40f, 0.00f},  // 13 orange stripe
    {0.05f, 0.45f, 0.10f},  // 14 green stripe
    {0.50f, 0.05f, 0.10f},  // 15 maroon stripe
};

// Ball numbers in rack order, reading left-to-right within each row (row 0 = apex).
// The 8-ball is fixed at the centre of row 2; corners of row 4 are one solid and one stripe.
static const int kRackOrder[15] = {
    1,
    2,  3,
    4,  8,  5,
    6,  9,  7,  10,
    15, 11, 13, 12, 14,
};

// Creates the sphere mesh, generates 16 ball textures, and places balls in rack formation.
BallScene BallScene::create(const std::string &fontPath)
{
    BallScene s;
    s.sphere = Mesh::uvSphere(32, 32);

    for (int i = 0; i < 16; ++i) {
        s.textures[i] = BallTexture::generate(i, kBallColor[i], fontPath);
    }

    // Cue ball at the head spot
    s.balls[0] = {
        .number = 0,
        .color = kBallColor[0],
        .pos = {-Table::kLength / 4.0f, Table::kBallR, 0.0f},
    };

    // Rack: apex at the foot spot, rows extend toward the foot rail (+X)
    const float footX = Table::kLength / 4.0f;
    const float dRow  = Table::kBallR * std::sqrt(3.0f);

    int rackIdx = 0;
    for (int row = 0; row < 5; ++row) {
        for (int col = 0; col <= row; ++col) {
            int n   = kRackOrder[rackIdx];
            float x = footX + row * dRow;
            float z = static_cast<float>(-row + 2 * col) * Table::kBallR;
            s.balls[1 + rackIdx] = {
                .number = n,
                .color = kBallColor[n],
                .pos = {x, Table::kBallR, z},
            };
            ++rackIdx;
        }
    }

    return s;
}

// Draws all 16 balls using the Phong shader with decal projection.
// The sphere mesh is a unit sphere scaled to kBallR; uniform scale leaves normals intact.
void BallScene::draw(Shader &shader, const glm::mat4 &view, const glm::mat4 &projection,
                     const glm::vec3 &lightDir, const glm::vec3 &cameraPos) const
{
    shader.use();
    shader.setMat4("uView", view);
    shader.setMat4("uProjection", projection);
    shader.setVec3("uLightDir", lightDir);
    shader.setVec3("uCameraPos", cameraPos);

    for (int i = 0; i < 16; ++i) {
        textures[i].bind(0);
        shader.setVec3("uColor", balls[i].color);
        glm::mat4 model = glm::translate(glm::mat4(1.0f), balls[i].pos);
        model = glm::scale(model, glm::vec3(Table::kBallR));
        shader.setMat4("uModel", model);
        shader.setMat3("uNormalMatrix", glm::mat3(glm::transpose(glm::inverse(model))));
        sphere.draw();
    }
}

// Releases all GPU resources.
void BallScene::destroy()
{
    for (int i = 0; i < 16; ++i) { textures[i].destroy(); }
    sphere.destroy();
}
