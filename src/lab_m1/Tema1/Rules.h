#pragma once

#include "lab_m1/Tema1/Player.h"


namespace m1
{
    // Gwent card effects
    class Rules
    {
     public:
        explicit Rules(int maxHP) : maxHP(maxHP) {}

        // Applies the effects of all the cards on the board (new ones and older ones),
        // by type in the fixed order: Scorch, Spy, Clear Weather, Frost Weather, War Horn, Shield, Damage, Heal.
        // Temporary cards lose 1 value after their effect; cards that reach 0 are removed.
        void ResolveTurn(Player &player, Player &enemy) const;

     private:
        void ApplyEffect(Card &card, int index, Player &owner, Player &opponent) const;

        int maxHP;
    };
}   // namespace m1
