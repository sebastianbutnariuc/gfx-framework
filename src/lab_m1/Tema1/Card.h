#pragma once

#include <string>

#include "utils/glm_utils.h"


namespace m1
{
    // Card types (values of Card::type)
    // Permanent
    const int CARD_DAMAGE = 0;          // red
    const int CARD_HEAL = 1;            // green
    const int CARD_SHIELD = 2;          // blue
    // Temporary
    const int CARD_SPY = 3;             // black
    const int CARD_FROST_WEATHER = 4;   // white, melee
    const int CARD_CLEAR_WEATHER = 5;   // yellow
    const int CARD_WAR_HORN = 6;        // orange
    const int CARD_SCORCH = 7;          // purple
    const int CARD_FOG = 8;             // gray, ranged
    const int CARD_RAIN = 9;            // teal, siege
    const int CARD_MEDIC = 10;          // pink
    const int CARD_DECOY = 11;          // brown
    const int CARD_TYPE_COUNT = 12;     // number of card types

    // Board lanes (values of Card::lane), from the center of the table outwards
    const int LANE_MELEE = 0;
    const int LANE_RANGED = 1;
    const int LANE_SIEGE = 2;
    const int LANE_COUNT = 3;           // number of lanes on each half of the board
    const int LANE_ANY = 3;             // the card can be placed on every lane

    class Card
    {
     public:
        Card(int type, int value, int lane);

        bool IsTemporary() const;
        bool IsWeather() const;
        bool CanGoOn(int l) const;

        // Moves `position` a step closer to `target`; once there, turns the card face up if needed
        void Animate(float deltaTime);

        // Look of the card, depends only on its type
        // (mesh names, see Tema1::CreateMeshes)
        std::string GetBodyMesh() const;        // "card_<type>"
        std::string GetDetailColor() const;     // color of the symbol, corner diamond and lane dots
        std::string GetSymbolShape() const;     // symbol mesh = "<shape>_<detail color>", e.g. "star_blue"
        float GetSymbolAngle() const;

     public:
        int type;               // CARD_...
        int value;
        int baseValue;          // starting value, restored when the Medic brings the card back
        int lane;               // where it can be placed (LANE_...)
        int placedLane;         // where it was placed (when on the board)

        glm::vec2 position;     // where it is drawn now
        glm::vec2 target;       // where it should go (layout / animations)

        bool faceUp;            // should be shown face up
        float flip;             // flip animation: 0 = back visible, 1 = front visible
    };
}   // namespace m1
