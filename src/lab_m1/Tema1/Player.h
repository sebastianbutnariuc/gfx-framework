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

        // Moves a card from the hand to a board lane and remembers it as this turn's card
        void PlayCard(int handIndex, Lane lane);

        // Decoy: takes the place of a board card, which goes back to the hand
        void PlayDecoy(int handIndex, int rowIndex);

        // Medic: a random card from the graveyard goes back to the hand, with its starting value
        void ReviveRandom();

        // Placement rules: normal cards go on their lane, a Decoy goes over one of your board cards
        bool CanPlay(const Card &card) const;
        bool CanPlayOn(const Card &card, Lane lane) const;
        bool HasPlayableCard() const;

        // Lane / board card under the given point, or -1 if none
        int LaneAt(glm::vec2 p) const;
        int RowCardAt(glm::vec2 p, glm::vec2 cardSize) const;

        // Damage is absorbed by the shield first
        void TakeDamage(int amount);
        void Heal(int amount, int maxHP);

        // Moves the board cards whose value reached 0 to the graveyard and forgets this turn's card
        void EndTurn();

        // Start of a round: the board goes to the graveyard, life and shield are reset
        void NewRound(int maxHP);

        void Animate(float deltaTime);

     public:
        std::vector<Card> deck;
        std::vector<Card> hand;
        std::vector<Card> row;      // cards placed on this player's half of the board (all lanes)
        std::vector<Card> graveyard;    // destroyed cards

        int hp;
        int shield;
        int crystals;               // rounds that can still be lost; 0 = game lost
        bool hiddenHand;            // the enemy's hand is shown face down

        // Card played in the current turn (index in `row`, -1 if none) and whether Scorch cancelled it
        int playedIndex;
        bool playedCancelled;

        // Where this side is drawn
        Rect handZone;
        Rect lanes[kLaneCount];     // indexed by Lane (Melee, Ranged, Siege)
        glm::vec2 deckPos;
        glm::vec2 livesPos;
        glm::vec2 crystalsPos;
    };
}   // namespace m1
