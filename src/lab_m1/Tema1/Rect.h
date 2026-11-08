#pragma once

#include "utils/glm_utils.h"


namespace m1
{
    // Axis-aligned rectangle (center + size): used for drawing zones and for hit tests
    struct Rect
    {
        glm::vec2 center;
        glm::vec2 size;

        Rect() : center(0), size(0) {}
        Rect(glm::vec2 center, glm::vec2 size) : center(center), size(size) {}

        bool Contains(glm::vec2 p) const
        {
            glm::vec2 half = size * 0.5f;
            return center.x - half.x <= p.x && p.x <= center.x + half.x
                && center.y - half.y <= p.y && p.y <= center.y + half.y;
        }
    };
}   // namespace m1
