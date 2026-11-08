#include "lab_m1/Tema1/object2D.h"

#include <vector>

#include "core/engine.h"
#include "utils/gl_utils.h"


namespace
{
    const int kSegments = 40;

    // Builds a white mesh from 2D points and triangle indices
    Mesh* Build(const std::string &name, const std::vector<glm::vec2> &points, const std::vector<unsigned int> &indices)
    {
        std::vector<VertexFormat> vertices;
        for (glm::vec2 p : points) {
            vertices.emplace_back(glm::vec3(p, 0), glm::vec3(1));
        }

        Mesh* mesh = new Mesh(name);
        mesh->InitFromData(vertices, indices);
        return mesh;
    }

    // Convex polygon around the origin: center (index 0) + outline points, as a triangle fan
    Mesh* BuildFan(const std::string &name, const std::vector<glm::vec2> &outline)
    {
        std::vector<glm::vec2> points = { glm::vec2(0) };
        points.insert(points.end(), outline.begin(), outline.end());

        std::vector<unsigned int> indices;
        unsigned int n = (unsigned int)outline.size();
        for (unsigned int i = 1; i <= n; i++) {
            indices.insert(indices.end(), { 0, i, i % n + 1 });
        }
        return Build(name, points, indices);
    }

    glm::vec2 OnCircle(float radius, float angle)
    {
        return radius * glm::vec2(glm::cos(angle), glm::sin(angle));
    }
}


Mesh* tema1::object2D::CreateSquare(const std::string &name)
{
    return BuildFan(name, { {-0.5f, -0.5f}, {0.5f, -0.5f}, {0.5f, 0.5f}, {-0.5f, 0.5f} });
}


Mesh* tema1::object2D::CreateTriangle(const std::string &name)
{
    return BuildFan(name, { {-0.5f, -0.5f}, {0.5f, -0.5f}, {0, 0.5f} });
}


Mesh* tema1::object2D::CreateDiamond(const std::string &name)
{
    return BuildFan(name, { {0, -0.5f}, {0.5f, 0}, {0, 0.5f}, {-0.5f, 0} });
}


Mesh* tema1::object2D::CreateCircle(const std::string &name)
{
    std::vector<glm::vec2> outline;
    for (int i = 0; i < kSegments; i++) {
        outline.push_back(OnCircle(0.5f, glm::two_pi<float>() * i / kSegments));
    }
    return BuildFan(name, outline);
}


Mesh* tema1::object2D::CreateStar(const std::string &name)
{
    // 10 points, alternating outer / inner radius, first point facing up
    std::vector<glm::vec2> outline;
    for (int i = 0; i < 10; i++) {
        float radius = (i % 2 == 0) ? 0.5f : 0.2f;
        outline.push_back(OnCircle(radius, glm::half_pi<float>() + glm::pi<float>() * i / 5));
    }
    return BuildFan(name, outline);
}


Mesh* tema1::object2D::CreateRing(const std::string &name, float innerRatio)
{
    // Pairs of points (outer, inner), joined by 2 triangles per segment
    std::vector<glm::vec2> points;
    std::vector<unsigned int> indices;
    unsigned int n = 2 * kSegments;

    for (int i = 0; i < kSegments; i++) {
        float angle = glm::two_pi<float>() * i / kSegments;
        points.push_back(OnCircle(0.5f, angle));
        points.push_back(OnCircle(0.5f * innerRatio, angle));

        unsigned int o0 = 2 * i, i0 = o0 + 1, o1 = (o0 + 2) % n, i1 = (o0 + 3) % n;
        indices.insert(indices.end(), { o0, o1, i1, o0, i1, i0 });
    }
    return Build(name, points, indices);
}


Mesh* tema1::object2D::CreateCross(const std::string &name)
{
    // Two overlapping bars
    const float t = 0.15f;
    std::vector<glm::vec2> points =
    {
        {-0.5f, -t}, {0.5f, -t}, {0.5f, t}, {-0.5f, t},     // horizontal
        {-t, -0.5f}, {t, -0.5f}, {t, 0.5f}, {-t, 0.5f}      // vertical
    };
    return Build(name, points, { 0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7 });
}
