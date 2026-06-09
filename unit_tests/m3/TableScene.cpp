#include "TableScene.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "Table.hpp"

static const glm::vec3 kFeltColor    = {0.08f, 0.38f, 0.08f};
static const glm::vec3 kCushionColor = {0.05f, 0.28f, 0.05f};

// Creates the table surface and cushion meshes from Table constants.
TableScene TableScene::create()
{
    TableScene s;
    s.surface      = Mesh::quad(Table::kLength / 2.0f, Table::kWidth / 2.0f);
    s.longCushion  = Mesh::box(Table::kLength / 2.0f, Table::kCushionH / 2.0f, Table::kCushionT / 2.0f);
    s.shortCushion = Mesh::box(Table::kCushionT / 2.0f, Table::kCushionH / 2.0f, Table::kWidth / 2.0f);
    return s;
}

// Draws the table surface and four cushions using the flat shader.
void TableScene::draw(Shader &shader, const glm::mat4 &view, const glm::mat4 &projection,
                      const glm::vec3 &lightDir) const
{
    shader.use();
    shader.setMat4("uView", view);
    shader.setMat4("uProjection", projection);
    shader.setVec3("uLightDir", lightDir);

    // Felt surface
    shader.setVec3("uColor", kFeltColor);
    shader.setMat4("uModel", glm::mat4(1.0f));
    surface.draw();

    // Four cushions: long ones run along X, short ones along Z
    const float cushionY      = Table::kCushionH / 2.0f;
    const float longCushionZ  = Table::kWidth  / 2.0f - Table::kCushionT / 2.0f;
    const float shortCushionX = Table::kLength / 2.0f - Table::kCushionT / 2.0f;

    shader.setVec3("uColor", kCushionColor);

    shader.setMat4("uModel", glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, cushionY, -longCushionZ)));
    longCushion.draw();

    shader.setMat4("uModel", glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, cushionY,  longCushionZ)));
    longCushion.draw();

    shader.setMat4("uModel", glm::translate(glm::mat4(1.0f), glm::vec3(-shortCushionX, cushionY, 0.0f)));
    shortCushion.draw();

    shader.setMat4("uModel", glm::translate(glm::mat4(1.0f), glm::vec3( shortCushionX, cushionY, 0.0f)));
    shortCushion.draw();
}

// Releases all GPU resources.
void TableScene::destroy()
{
    surface.destroy();
    longCushion.destroy();
    shortCushion.destroy();
}
