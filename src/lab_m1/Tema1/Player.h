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
        Player() : hp(0), shield(0) {}

        // Moves the top card of the deck into the hand; it starts at the deck position
        void DrawFromDeck()
        {
            if (deck.empty()) {
                return;
            }
            Card card = deck.back();
            deck.pop_back();
            card.position = deckPos;
            hand.push_back(card);
        }

        void Animate(float deltaTime)
        {
            for (Card &c : hand) c.MoveTowardsTarget(deltaTime);
            for (Card &c : row)  c.MoveTowardsTarget(deltaTime);
        }

     public:
        std::vector<Card> deck;
        std::vector<Card> hand;
        std::vector<Card> row;      // cards placed on this player's half of the board

        int hp;
        int shield;

        // Where this side is drawn
        Rect handZone;
        Rect boardZone;
        glm::vec2 deckPos;
        glm::vec2 livesPos;
    };
}   // namespace m1
