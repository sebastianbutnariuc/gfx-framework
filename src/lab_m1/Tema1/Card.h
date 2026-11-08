#pragma once

#include <string>

#include "utils/glm_utils.h"


namespace m1
{
    enum class CardType
    {
        // Permanent
        Damage,         // red
        Heal,           // green
        Shield,         // blue
        // Temporary
        Spy,            // black
        FrostWeather,   // white
        ClearWeather,   // yellow
        WarHorn,        // orange
        Scorch,         // purple
        Count
    };

    class Card
    {
     public:
        Card(CardType type, int value);

        static Card Random();

        bool IsTemporary() const;

        // Look of the card, depends only on its type
        glm::vec3 GetColor() const;
        glm::vec3 GetSymbolColor() const;
        std::string GetSymbolMesh() const;
        float GetSymbolAngle() const;

     public:
        CardType type;
        int value;

        glm::vec2 position;     // where it is drawn now
        glm::vec2 target;       // where it should go (layout / animations)
    };
}   // namespace m1
