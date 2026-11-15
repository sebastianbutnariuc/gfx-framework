#pragma once

#include <string>

#include "core/gpu/mesh.h"
#include "utils/glm_utils.h"


// All meshes have size 1 (fit in [-0.5, 0.5]) and are centered in the origin.
// The color is stored in the vertices (drawn with the "VertexColor" shader);
// size, position and rotation are chosen when drawing (model matrix).
namespace tema1
{
    namespace object2D
    {
        // fill = false: only the outline (GL_LINE_LOOP), as in lab3
        Mesh* CreateSquare(const std::string &name, glm::vec3 color, bool fill = true);
        // Regular polygon inscribed in the circle of diameter 1 (many sides = circle)
        Mesh* CreatePolygon(const std::string &name, int sides, glm::vec3 color);
        Mesh* CreateRing(const std::string &name, float innerRatio, glm::vec3 color);
        Mesh* CreateTriangle(const std::string &name, glm::vec3 color);
        Mesh* CreateDiamond(const std::string &name, glm::vec3 color);
        Mesh* CreateStar(const std::string &name, glm::vec3 color);
        Mesh* CreateCross(const std::string &name, glm::vec3 color);
    }   // namespace object2D
}   // namespace tema1
