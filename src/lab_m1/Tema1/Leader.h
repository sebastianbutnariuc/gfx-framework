#pragma once

#include <string>


namespace m1
{
    // Leader abilities (values of Leader::ability), one-time per game
    const int ABILITY_CLEAR_WEATHER = 0;            // Foltest: removes all weather
    const int ABILITY_REVEAL_HAND = 1;              // Emhyr: reveals 3 random cards from the opponent's hand
    const int ABILITY_DOUBLE_MELEE = 2;             // Eredin: doubles your melee cards
    const int ABILITY_DESTROY_STRONGEST_MELEE = 3;  // Francesca: destroys the opponent's strongest melee card
    const int ABILITY_HORN_SIEGE = 4;               // Radovid: doubles your siege cards (War Horn on siege)

    class Leader
    {
     public:
        Leader()
        {
            shape = "circle";
            ability = ABILITY_CLEAR_WEATHER;
            used = false;
        }

        Leader(const std::string &shape, int ability)
        {
            this->shape = shape;
            this->ability = ability;
            used = false;
        }

     public:
        std::string shape;      // the leader's shape: meshes "leader_<shape>", "..._used", "..._highlight"
        int ability;            // ABILITY_...
        bool used;              // ability already used (leader drawn gray)
    };
}   // namespace m1
