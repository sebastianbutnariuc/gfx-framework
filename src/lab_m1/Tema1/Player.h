#pragma once

#include <vector>

#include "lab_m1/Tema1/Card.h"
#include "lab_m1/Tema1/Rect.h"


namespace m1
{
    // Everything that belongs to one side of the table (the player or the enemy)
    class Player
    {
     public:
        Player() : hp(0), shield(0), deckCount(0) {}

     public:
        std::vector<Card> hand;
        std::vector<Card> row;      // cards placed on this player's half of the board

        int hp;
        int shield;
        int deckCount;

        // Where this side is drawn
        Rect handZone;
        Rect boardZone;
        glm::vec2 deckPos;
        glm::vec2 livesPos;
    };
}   // namespace m1
