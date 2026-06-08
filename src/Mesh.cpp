#include "Mesh.hpp"

#include <glm/glm.hpp>
#include <vector>
#include <cmath>

// Uploads interleaved vertex data (pos+normal+uv, 8 floats/vertex) and an index list to the GPU.
static Mesh uploadMesh(const std::vector<float> &vertices, const std::vector<unsigned int> &indices)
{
    Mesh mesh;
    mesh.indexCount = static_cast<int>(indices.size());

    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);
    glGenBuffers(1, &mesh.ebo);

    glBindVertexArray(mesh.vao);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    const int stride = 8 * sizeof(float);
    // location 0 — position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *)0);
    glEnableVertexAttribArray(0);
    // location 1 — normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // location 2 — UV texcoord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    return mesh;
}

// Generates a unit UV sphere by subdividing the surface into a grid of rings × sectors.
// Each vertex stores position (xyz), normal (xyz), and UV texcoord (uv) interleaved in one VBO.
Mesh Mesh::uvSphere(int rings, int sectors)
{
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    vertices.reserve((rings + 1) * (sectors + 1) * 8);
    indices.reserve(rings * sectors * 6);

    for (int r = 0; r <= rings; ++r)
    {
        // phi sweeps from 0 (north pole) to PI (south pole)
        float phi = static_cast<float>(M_PI) * r / rings;
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);

        for (int s = 0; s <= sectors; ++s)
        {
            // theta sweeps a full circle around the Y axis
            float theta = 2.0f * static_cast<float>(M_PI) * s / sectors;
            float sinTheta = std::sin(theta);
            float cosTheta = std::cos(theta);

            // On a unit sphere, position and outward normal are identical
            float x = sinPhi * cosTheta;
            float y = cosPhi;
            float z = sinPhi * sinTheta;

            // position
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            // normal
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            // UV — equirectangular: u wraps around longitude, v from top to bottom
            vertices.push_back(static_cast<float>(s) / sectors);
            vertices.push_back(static_cast<float>(r) / rings);
        }
    }

    // Two triangles per quad, wound counter-clockwise (OpenGL front-face default)
    for (int r = 0; r < rings; ++r)
    {
        for (int s = 0; s < sectors; ++s)
        {
            unsigned int tl = r * (sectors + 1) + s;
            unsigned int tr = tl + 1;
            unsigned int bl = tl + (sectors + 1);
            unsigned int br = bl + 1;

            indices.push_back(tl);
            indices.push_back(bl);
            indices.push_back(tr);

            indices.push_back(tr);
            indices.push_back(bl);
            indices.push_back(br);
        }
    }

    return uploadMesh(vertices, indices);
}

// Uploads position-only vertex data (3 floats/vertex) for use with the flat shader.
// No normals or UVs needed — the flat shader derives the normal from position derivatives.
static Mesh uploadFlatMesh(const std::vector<float> &vertices, const std::vector<unsigned int> &indices)
{
    Mesh mesh;
    mesh.indexCount = static_cast<int>(indices.size());

    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);
    glGenBuffers(1, &mesh.ebo);

    glBindVertexArray(mesh.vao);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // location 0 — position only
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    return mesh;
}

// Generates a flat quad in the XZ plane (Y=0) for use with the flat shader.
// 4 unique vertices, normal derived per-fragment via dFdx/dFdy.
Mesh Mesh::quad(float halfW, float halfH)
{
    // CCW winding from above (+Y) so dFdx/dFdy gives an upward normal
    std::vector<float> vertices = {
        -halfW, 0.0f, -halfH,
         halfW, 0.0f, -halfH,
         halfW, 0.0f,  halfH,
        -halfW, 0.0f,  halfH,
    };
    std::vector<unsigned int> indices = {0, 3, 2,  0, 2, 1};
    return uploadFlatMesh(vertices, indices);
}

// Generates an axis-aligned box centered at the origin for use with the flat shader.
// 8 unique corner vertices — no duplication needed since normals come from derivatives.
Mesh Mesh::box(float halfW, float halfH, float halfD)
{
    float W = halfW, H = halfH, D = halfD;

    // 8 unique corners
    std::vector<float> vertices = {
        -W, -H, -D,  // 0
        +W, -H, -D,  // 1
        +W, +H, -D,  // 2
        -W, +H, -D,  // 3
        -W, -H, +D,  // 4
        +W, -H, +D,  // 5
        +W, +H, +D,  // 6
        -W, +H, +D,  // 7
    };

    // Each face wound CCW from outside so dFdx/dFdy gives an outward normal
    std::vector<unsigned int> indices = {
        3, 7, 6,  3, 6, 2,  // top    (+Y)
        4, 0, 1,  4, 1, 5,  // bottom (-Y)
        1, 2, 6,  1, 6, 5,  // right  (+X)
        4, 7, 3,  4, 3, 0,  // left   (-X)
        5, 6, 7,  5, 7, 4,  // front  (+Z)
        0, 3, 2,  0, 2, 1,  // back   (-Z)
    };

    return uploadFlatMesh(vertices, indices);
}

// Binds the VAO and issues an indexed draw call for the full mesh.
void Mesh::draw() const
{
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

// Deletes the VAO and both buffer objects from GPU memory.
void Mesh::destroy()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
}
