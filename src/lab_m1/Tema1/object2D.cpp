#include "lab_m1/Tema1/object2D.h"

#include <vector>

#include "core/engine.h"
#include "utils/gl_utils.h"


namespace
{
    const glm::vec3 kWhite = glm::vec3(1);

    Mesh* Build(const std::string &name, const std::vector<VertexFormat> &vertices,
                const std::vector<unsigned int> &indices, GLenum drawMode = GL_TRIANGLES)
    {
        Mesh* mesh = new Mesh(name);
        mesh->SetDrawMode(drawMode);
        mesh->InitFromData(vertices, indices);
        return mesh;
    }

    // Triangle fan around vertex 0 (the center), as a plain GL_TRIANGLES list
    std::vector<unsigned int> FanIndices(unsigned int outerCount)
    {
        std::vector<unsigned int> indices;
        for (unsigned int i = 1; i <= outerCount; i++) {
            indices.push_back(0);
            indices.push_back(i);
            indices.push_back(i % outerCount + 1);
        }
        return indices;
    }
}


Mesh* tema1::object2D::CreateSquare(const std::string &name, bool fill)
{
    std::vector<VertexFormat> vertices =
    {
        VertexFormat(glm::vec3(-0.5f, -0.5f, 0), kWhite),
        VertexFormat(glm::vec3( 0.5f, -0.5f, 0), kWhite),
        VertexFormat(glm::vec3( 0.5f,  0.5f, 0), kWhite),
        VertexFormat(glm::vec3(-0.5f,  0.5f, 0), kWhite)
    };

    if (!fill) {
        return Build(name, vertices, { 0, 1, 2, 3 }, GL_LINE_LOOP);
    }
    return Build(name, vertices, { 0, 1, 2, 0, 2, 3 });
}


Mesh* tema1::object2D::CreateCircle(const std::string &name, int segments, bool fill)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    if (fill) {
        vertices.emplace_back(glm::vec3(0), kWhite);
    }
    for (int i = 0; i < segments; i++) {
        float angle = 2 * glm::pi<float>() * i / segments;
        vertices.emplace_back(glm::vec3(0.5f * glm::cos(angle), 0.5f * glm::sin(angle), 0), kWhite);
    }

    if (!fill) {
        for (int i = 0; i < segments; i++) {
            indices.push_back(i);
        }
        return Build(name, vertices, indices, GL_LINE_LOOP);
    }
    return Build(name, vertices, FanIndices(segments));
}


Mesh* tema1::object2D::CreateRing(const std::string &name, float innerRadius, int segments)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    for (int i = 0; i < segments; i++) {
        float angle = 2 * glm::pi<float>() * i / segments;
        glm::vec3 dir = glm::vec3(0.5f * glm::cos(angle), 0.5f * glm::sin(angle), 0);
        vertices.emplace_back(dir, kWhite);
        vertices.emplace_back(dir * innerRadius, kWhite);
    }
    for (int i = 0; i < segments; i++) {
        unsigned int o0 = 2 * i, i0 = 2 * i + 1;
        unsigned int o1 = (2 * i + 2) % (2 * segments), i1 = (2 * i + 3) % (2 * segments);
        indices.insert(indices.end(), { o0, o1, i1, o0, i1, i0 });
    }
    return Build(name, vertices, indices);
}


Mesh* tema1::object2D::CreateTriangle(const std::string &name)
{
    std::vector<VertexFormat> vertices =
    {
        VertexFormat(glm::vec3(-0.5f, -0.5f, 0), kWhite),
        VertexFormat(glm::vec3( 0.5f, -0.5f, 0), kWhite),
        VertexFormat(glm::vec3( 0.0f,  0.5f, 0), kWhite)
    };
    return Build(name, vertices, { 0, 1, 2 });
}


Mesh* tema1::object2D::CreateDiamond(const std::string &name)
{
    std::vector<VertexFormat> vertices =
    {
        VertexFormat(glm::vec3( 0.0f, -0.5f, 0), kWhite),
        VertexFormat(glm::vec3( 0.5f,  0.0f, 0), kWhite),
        VertexFormat(glm::vec3( 0.0f,  0.5f, 0), kWhite),
        VertexFormat(glm::vec3(-0.5f,  0.0f, 0), kWhite)
    };
    return Build(name, vertices, { 0, 1, 2, 0, 2, 3 });
}


Mesh* tema1::object2D::CreateStar(const std::string &name, float innerRatio)
{
    std::vector<VertexFormat> vertices;
    vertices.emplace_back(glm::vec3(0), kWhite);

    const int points = 5;
    for (int i = 0; i < 2 * points; i++) {
        float angle = glm::half_pi<float>() + glm::pi<float>() * i / points;
        float r = (i % 2 == 0) ? 0.5f : 0.5f * innerRatio;
        vertices.emplace_back(glm::vec3(r * glm::cos(angle), r * glm::sin(angle), 0), kWhite);
    }
    return Build(name, vertices, FanIndices(2 * points));
}


Mesh* tema1::object2D::CreateCross(const std::string &name, float thickness)
{
    float t = thickness / 2;
    std::vector<VertexFormat> vertices =
    {
        // horizontal bar
        VertexFormat(glm::vec3(-0.5f, -t, 0), kWhite),
        VertexFormat(glm::vec3( 0.5f, -t, 0), kWhite),
        VertexFormat(glm::vec3( 0.5f,  t, 0), kWhite),
        VertexFormat(glm::vec3(-0.5f,  t, 0), kWhite),
        // vertical bar
        VertexFormat(glm::vec3(-t, -0.5f, 0), kWhite),
        VertexFormat(glm::vec3( t, -0.5f, 0), kWhite),
        VertexFormat(glm::vec3( t,  0.5f, 0), kWhite),
        VertexFormat(glm::vec3(-t,  0.5f, 0), kWhite)
    };
    return Build(name, vertices, { 0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7 });
}
