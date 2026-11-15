#pragma once

#include <string>
#include <vector>

#include "components/simple_scene.h"
#include "lab_m1/Tema1/Card.h"
#include "lab_m1/Tema1/Leader.h"
#include "lab_m1/Tema1/Player.h"
#include "lab_m1/Tema1/Rect.h"


namespace m1
{
    // Game phases (values of Tema1::phase)
    const int PHASE_LEADER_SELECT = 0;  // menu: choose a leader, click it again to start
    const int PHASE_DEALING = 1;        // hands are being dealt
    const int PHASE_PLAYER_TURN = 2;    // waiting for the player to drop a card
    const int PHASE_ENEMY_TURN = 3;     // short pause, then the enemy plays a card
    const int PHASE_REVEAL = 4;         // enemy card is moving to the board and flipping
    const int PHASE_ROUND_OVER = 5;     // round result shown for a moment, then the next round starts
    const int PHASE_GAME_OVER = 6;      // result shown, R starts a new game

    class Tema1 : public gfxc::SimpleScene
    {
     public:
        void Init() override;

     private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;

        void OnKeyPress(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;

        void ShowLeaderMenu();
        Rect LeaderMenuSlot(int index) const;   // where leader `index` is drawn in the menu
        void NewGame();
        Card RandomCard();  // random type, value 1..4, lane allowed by its type
        void DealCard(Player &p);
        void EnemyPlayCard();
        bool IsRoundOver() const;
        void EndRound();
        void StartRound();

        // Gwent card effects
        // Applies the effects of all the cards on the board (new ones and older ones),
        // by type in the fixed order: Scorch, Spy, Medic, Clear Weather, weather (Frost, Fog, Rain), War Horn, Shield, Damage, Heal.
        // Temporary cards lose 1 value after their effect (War Horn: only when it doubled a card);
        // cards that reach 0 are removed.
        void ResolveTurn();
        // Applies the owner's leader ability (once per game) and marks the leader as used.
        // Called only between turns (no card of the current turn on the board).
        void UseLeaderAbility(Player &owner, Player &opponent);

        // Window pixels (origin top-left) -> scene coordinates (origin bottom-left)
        glm::vec2 ScreenToWorld(int mouseX, int mouseY) const;

        // Sets the cards' targets on one centered row (they slide there through the animation)
        void LayoutRow(const std::vector<Card*> &cards, glm::vec2 center, float step);
        void LayoutAll();   // hands and board lanes of both players

        // All the meshes of the scene: one for each shape + color combination (the color is in the vertices)
        void CreateMeshes();

        // Drawing: each part of the scene builds its modelMatrix and calls RenderMesh2D (as in lab3)
        void DrawPlayer(const Player &p);
        void DrawCard(const Card &card, float scale = 1);               // front or back, with flip
        void DrawLaneMarker(int lane, glm::vec2 center, const std::string &mesh, glm::mat3 baseMatrix);
        void DrawCardFront(const Card &card, glm::mat3 cardMatrix);     // cardMatrix = the card's transform
        void DrawCardBack(glm::mat3 cardMatrix);
        void DrawDeck(glm::vec2 center, int cardsLeft);
        void DrawLives(glm::vec2 start, int hp, int shield);
        void DrawCrystals(glm::vec2 start, int crystals);
        void DrawLeader(const Leader &leader, glm::vec2 center, float size, bool highlight);
        void DrawLeaderMenu();
        // result: 1 = player won, -1 = player lost, 0 = draw
        void DrawResultBanner(int result, glm::vec2 size);

     private:
        // Sizes and game settings, set in Init()
        // Logical scene size: the camera always shows this, whatever the window size
        float sceneWidth, sceneHeight;

        float cardWidth, cardHeight;
        float cardSpacing;
        float boardScale;       // cards on the board are drawn smaller

        // Board: centered on the middle line, each half has 3 lanes
        float boardCenterX;
        float boardWidth;
        float middleY;
        float laneHeight;

        int maxHP;
        int maxCrystals;
        float roundPause;       // seconds the round result stays on screen
        int deckSize;
        int handSize;           // cards dealt to each player at the start
        float dealInterval;     // seconds between two dealt cards
        float enemyDelay;       // seconds before the enemy plays

        glm::mat3 modelMatrix;  // transform of the mesh being drawn

        Player player;
        Player enemy;

        glm::vec2 mouseWorld;
        int draggedIndex;   // index in player.hand of the card being dragged, -1 if none

        int phase;          // PHASE_...
        float timer;        // time until the next action (next dealt card / enemy move)
        int cardsToDeal;    // cards left to deal (both players)
        int roundResult;    // result of the last round, shown during RoundOver

        std::vector<Leader> leaders;    // the choices in the menu
        int selectedLeader;             // index in `leaders`, -1 if none
    };
}   // namespace m1
