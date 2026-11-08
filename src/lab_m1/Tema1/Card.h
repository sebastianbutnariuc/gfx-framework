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

    // Board lanes, from the center of the table outwards. Any = can be placed on every lane.
    enum class Lane
    {
        Melee,
        Ranged,
        Siege,
        Any
    };

    const int kLaneCount = 3;

    class Card
    {
     public:
        Card(CardType type, int value, Lane lane);

        static Card Random();

        bool IsTemporary() const;
        bool CanGoOn(Lane l) const { return lane == Lane::Any || lane == l; }

        // Moves `position` a step closer to `target`; once there, turns the card face up if needed
        void Animate(float deltaTime);

        // Look of the card, depends only on its type
        glm::vec3 GetColor() const;
        glm::vec3 GetSymbolColor() const;
        std::string GetSymbolMesh() const;
        float GetSymbolAngle() const;

     public:
        CardType type;
        int value;
        Lane lane;              // where it can be placed
        Lane placedLane;        // where it was placed (when on the board)

        glm::vec2 position;     // where it is drawn now
        glm::vec2 target;       // where it should go (layout / animations)

        bool faceUp;            // should be shown face up
        float flip;             // flip animation: 0 = back visible, 1 = front visible
    };
}   // namespace m1
