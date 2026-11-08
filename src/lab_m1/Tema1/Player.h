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
        Player();

        // Moves the top card of the deck into the hand; it starts at the deck position
        void DrawFromDeck();

        // Moves a card from the hand to the board row and remembers it as this turn's card
        void PlayCard(int handIndex);

        // Damage is absorbed by the shield first
        void TakeDamage(int amount);
        void Heal(int amount, int maxHP);

        // Removes the board cards whose value reached 0 and forgets this turn's card
        void EndTurn();

        void Animate(float deltaTime);

     public:
        std::vector<Card> deck;
        std::vector<Card> hand;
        std::vector<Card> row;      // cards placed on this player's half of the board

        int hp;
        int shield;
        bool hiddenHand;            // the enemy's hand is shown face down

        // Card played in the current turn (index in `row`, -1 if none) and whether Scorch cancelled it
        int playedIndex;
        bool playedCancelled;

        // Where this side is drawn
        Rect handZone;
        Rect boardZone;
        glm::vec2 deckPos;
        glm::vec2 livesPos;
    };
}   // namespace m1
