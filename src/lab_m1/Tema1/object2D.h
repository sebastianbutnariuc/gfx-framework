#pragma once

#include <string>

#include "core/gpu/mesh.h"
#include "utils/glm_utils.h"


// All meshes are white, size 1 (fit in [-0.5, 0.5]) and centered in the origin.
// Size, position and color are chosen when drawing (model matrix + "Color" shader).
namespace tema1
{
    namespace object2D
    {
        Mesh* CreateSquare(const std::string &name);
        // Regular polygon inscribed in the circle of diameter 1 (many sides = circle)
        Mesh* CreatePolygon(const std::string &name, int sides);
        Mesh* CreateRing(const std::string &name, float innerRatio);
        Mesh* CreateTriangle(const std::string &name);
        Mesh* CreateDiamond(const std::string &name);
        Mesh* CreateStar(const std::string &name);
        Mesh* CreateCross(const std::string &name);
    }   // namespace object2D
}   // namespace tema1
