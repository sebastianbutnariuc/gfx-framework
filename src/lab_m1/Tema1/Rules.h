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
        // by type in the fixed order: Scorch, Spy, Medic, Clear Weather, weather (Frost, Fog, Rain), War Horn, Shield, Damage, Heal.
        // Decoy has no effect (it only holds a place on the board).
        // Temporary cards lose 1 value after their effect; cards that reach 0 are removed.
        void ResolveTurn(Player &player, Player &enemy) const;

     private:
        void ApplyEffect(Card &card, int index, Player &owner, Player &opponent) const;

        int maxHP;
    };
}   // namespace m1
