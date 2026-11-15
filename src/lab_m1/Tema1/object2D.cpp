#include "lab_m1/Tema1/object2D.h"

#include <vector>

#include "core/engine.h"
#include "utils/gl_utils.h"


Mesh* tema1::object2D::CreateSquare(const std::string &name, glm::vec3 color, bool fill)
{
    std::vector<VertexFormat> vertices =
    {
        VertexFormat(glm::vec3(-0.5f, -0.5f, 0), color),
        VertexFormat(glm::vec3( 0.5f, -0.5f, 0), color),
        VertexFormat(glm::vec3( 0.5f,  0.5f, 0), color),
        VertexFormat(glm::vec3(-0.5f,  0.5f, 0), color)
    };

    Mesh* square = new Mesh(name);
    std::vector<unsigned int> indices = { 0, 1, 2, 3 };

    if (!fill) {
        // Only the outline: the 4 corners joined by lines
        square->SetDrawMode(GL_LINE_LOOP);
    } else {
        // Draw 2 triangles: (0, 1, 2) and (3, 0, 2)
        indices.push_back(0);
        indices.push_back(2);
    }

    square->InitFromData(vertices, indices);
    return square;
}


Mesh* tema1::object2D::CreateTriangle(const std::string &name, glm::vec3 color)
{
    std::vector<VertexFormat> vertices =
    {
        VertexFormat(glm::vec3(-0.5f, -0.5f, 0), color),
        VertexFormat(glm::vec3( 0.5f, -0.5f, 0), color),
        VertexFormat(glm::vec3( 0,     0.5f, 0), color)
    };
    std::vector<unsigned int> indices = { 0, 1, 2 };

    Mesh* triangle = new Mesh(name);
    triangle->InitFromData(vertices, indices);
    return triangle;
}


Mesh* tema1::object2D::CreateDiamond(const std::string &name, glm::vec3 color)
{
    std::vector<VertexFormat> vertices =
    {
        VertexFormat(glm::vec3( 0,    -0.5f, 0), color),
        VertexFormat(glm::vec3( 0.5f,  0,    0), color),
        VertexFormat(glm::vec3( 0,     0.5f, 0), color),
        VertexFormat(glm::vec3(-0.5f,  0,    0), color)
    };
    std::vector<unsigned int> indices = { 0, 1, 2, 0, 2, 3 };

    Mesh* diamond = new Mesh(name);
    diamond->InitFromData(vertices, indices);
    return diamond;
}


Mesh* tema1::object2D::CreatePolygon(const std::string &name, int sides, glm::vec3 color)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // Vertex 0 = center, vertices 1..sides = points on the circle of radius 0.5
    vertices.push_back(VertexFormat(glm::vec3(0, 0, 0), color));
    for (int i = 0; i < sides; i++) {
        float angle = 2 * glm::pi<float>() * i / sides;
        vertices.push_back(VertexFormat(glm::vec3(0.5f * glm::cos(angle), 0.5f * glm::sin(angle), 0), color));
    }

    // One triangle for each side: center, current point, next point
    for (int i = 1; i <= sides; i++) {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i % sides + 1);
    }

    Mesh* polygon = new Mesh(name);
    polygon->InitFromData(vertices, indices);
    return polygon;
}


Mesh* tema1::object2D::CreateStar(const std::string &name, glm::vec3 color)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // Vertex 0 = center, vertices 1..10 = tips (radius 0.5) alternating with inner corners (radius 0.2),
    // the first tip facing up
    vertices.push_back(VertexFormat(glm::vec3(0, 0, 0), color));
    for (int i = 0; i < 10; i++) {
        float radius = (i % 2 == 0) ? 0.5f : 0.2f;
        float angle = glm::pi<float>() / 2 + glm::pi<float>() * i / 5;
        vertices.push_back(VertexFormat(glm::vec3(radius * glm::cos(angle), radius * glm::sin(angle), 0), color));
    }

    // One triangle for each pair of neighbour points: center, current point, next point
    for (int i = 1; i <= 10; i++) {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i % 10 + 1);
    }

    Mesh* star = new Mesh(name);
    star->InitFromData(vertices, indices);
    return star;
}


Mesh* tema1::object2D::CreateRing(const std::string &name, float innerRatio, glm::vec3 color)
{
    int segments = 40;
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // For each angle: an outer point (index 2 * i) and an inner point (index 2 * i + 1)
    for (int i = 0; i < segments; i++) {
        float angle = 2 * glm::pi<float>() * i / segments;
        glm::vec3 direction = glm::vec3(glm::cos(angle), glm::sin(angle), 0);
        vertices.push_back(VertexFormat(0.5f * direction, color));
        vertices.push_back(VertexFormat(0.5f * innerRatio * direction, color));
    }

    // Each segment (between angle i and the next one) = 2 triangles
    for (int i = 0; i < segments; i++) {
        int next = (i + 1) % segments;
        int outer = 2 * i, inner = 2 * i + 1;
        int nextOuter = 2 * next, nextInner = 2 * next + 1;

        indices.push_back(outer);
        indices.push_back(nextOuter);
        indices.push_back(nextInner);

        indices.push_back(outer);
        indices.push_back(nextInner);
        indices.push_back(inner);
    }

    Mesh* ring = new Mesh(name);
    ring->InitFromData(vertices, indices);
    return ring;
}


Mesh* tema1::object2D::CreateCross(const std::string &name, glm::vec3 color)
{
    float t = 0.15f;    // half of the bar thickness

    // Two overlapping bars: horizontal (vertices 0-3) and vertical (vertices 4-7)
    std::vector<VertexFormat> vertices =
    {
        VertexFormat(glm::vec3(-0.5f, -t, 0), color),
        VertexFormat(glm::vec3( 0.5f, -t, 0), color),
        VertexFormat(glm::vec3( 0.5f,  t, 0), color),
        VertexFormat(glm::vec3(-0.5f,  t, 0), color),

        VertexFormat(glm::vec3(-t, -0.5f, 0), color),
        VertexFormat(glm::vec3( t, -0.5f, 0), color),
        VertexFormat(glm::vec3( t,  0.5f, 0), color),
        VertexFormat(glm::vec3(-t,  0.5f, 0), color)
    };
    std::vector<unsigned int> indices = { 0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7 };

    Mesh* cross = new Mesh(name);
    cross->InitFromData(vertices, indices);
    return cross;
}
