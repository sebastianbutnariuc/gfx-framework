#pragma once

#include <string>

#include "core/gpu/mesh.h"
#include "utils/glm_utils.h"


// All meshes are "unit" shapes (fit in [-0.5, 0.5]) centered in the origin, colored white.
// They are drawn with RenderMesh2D(mesh, modelMatrix, color), so the color
// is chosen at draw time and the size/position come from the model matrix.
namespace tema1
{
    namespace object2D
    {
        // Square with side 1, centered in (0, 0)
        Mesh* CreateSquare(const std::string &name, bool fill = true);

        // Circle with diameter 1 (radius 0.5), centered in (0, 0)
        Mesh* CreateCircle(const std::string &name, int segments = 40, bool fill = true);

        // Ring with outer diameter 1; innerRadius is a ratio of the outer radius (0..1)
        Mesh* CreateRing(const std::string &name, float innerRadius, int segments = 40);

        // Isosceles triangle inscribed in the unit square [-0.5, 0.5]
        Mesh* CreateTriangle(const std::string &name);

        // Diamond (rhombus) inscribed in the unit square [-0.5, 0.5]
        Mesh* CreateDiamond(const std::string &name);

        // 5-point star, outer radius 0.5
        Mesh* CreateStar(const std::string &name, float innerRatio = 0.4f);

        // Plus/cross, arm thickness given relative to size 1
        Mesh* CreateCross(const std::string &name, float thickness = 0.3f);
    }   // namespace object2D
}   // namespace tema1
