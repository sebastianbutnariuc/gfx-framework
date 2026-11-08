#pragma once

#include <string>
#include <vector>

#include "components/simple_scene.h"
#include "lab_m1/Tema1/Card.h"
#include "lab_m1/Tema1/Player.h"
#include "lab_m1/Tema1/Rect.h"


namespace m1
{
    enum class Phase
    {
        Dealing,        // hands are being dealt
        PlayerTurn,     // waiting for the player to drop a card
        EnemyTurn,      // short pause, then the enemy plays a card
        Reveal,         // enemy card is moving to the board and flipping
        GameOver        // result shown, R starts a new game
    };

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

        void NewGame();
        void DealCard(Player &p);
        void EnemyPlayCard();
        bool IsGameOver() const;

        // Window pixels (origin top-left) -> scene coordinates (origin bottom-left)
        glm::vec2 ScreenToWorld(int mouseX, int mouseY) const;

        // Sets the cards' targets on one centered row (they slide there through the animation)
        void LayoutRow(const std::vector<Card*> &cards, glm::vec2 center, float step);
        void LayoutAll();   // hands and board lanes of both players

        // Drawing
        // `parent` = transform applied on top (e.g. the card's own matrix, for the card's details)
        void DrawMesh(const std::string &mesh, glm::vec2 center, glm::vec2 size, glm::vec3 color,
                      float angle = 0, const glm::mat3 &parent = glm::mat3(1));
        void DrawRect(const Rect &rect, glm::vec3 color);
        void DrawOutline(const Rect &rect, glm::vec3 color, float thickness);

        void DrawPlayer(const Player &p, glm::vec3 boardColor);
        void DrawCard(const Card &card, float scale = 1);               // front or back, with flip
        void DrawLaneMarker(Lane lane, glm::vec2 center, glm::vec3 color, const glm::mat3 &parent = glm::mat3(1));
        void DrawCardFront(const Card &card, const glm::mat3 &model);   // model = card's transform
        void DrawCardBack(const glm::mat3 &model);
        void DrawDeck(glm::vec2 center, int cardsLeft);
        void DrawLives(glm::vec2 start, int hp, int shield);
        void DrawGameOver();

     private:
        // Logical scene size: the camera always shows this, whatever the window size
        static constexpr float kSceneWidth = 1280;
        static constexpr float kSceneHeight = 720;

        static constexpr float kCardWidth = 70;
        static constexpr float kCardHeight = 100;
        static constexpr float kCardSpacing = 15;
        static constexpr float kBoardScale = 0.65f;     // cards on the board are drawn smaller

        // Board: centered on the middle line, each half has 3 lanes
        static constexpr float kBoardCenterX = 630;
        static constexpr float kBoardWidth = 760;
        static constexpr float kMiddleY = 360;
        static constexpr float kLaneHeight = 76;

        static constexpr int kMaxHP = 15;
        static constexpr int kDeckSize = 20;
        static constexpr int kHandSize = 10;            // cards dealt to each player at the start
        static constexpr float kDealInterval = 0.15f;   // seconds between two dealt cards
        static constexpr float kEnemyDelay = 0.5f;      // seconds before the enemy plays

        Player player;
        Player enemy;

        glm::vec2 mouseWorld;
        int draggedIndex;   // index in player.hand of the card being dragged, -1 if none

        Phase phase;
        float timer;        // time until the next action (next dealt card / enemy move)
        int cardsToDeal;    // cards left to deal at the start (both players)
    };
}   // namespace m1
