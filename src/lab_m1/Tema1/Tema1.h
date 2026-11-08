#pragma once

#include <string>
#include <vector>

#include "components/simple_scene.h"
#include "lab_m1/Tema1/Card.h"
#include "lab_m1/Tema1/Player.h"
#include "lab_m1/Tema1/Rect.h"


namespace m1
{
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

        // Window pixels (origin top-left) -> scene coordinates (origin bottom-left)
        glm::vec2 ScreenToWorld(int mouseX, int mouseY) const;

        // Puts the cards on one centered row; snap = jump there, otherwise only set the target
        void LayoutRow(std::vector<Card> &cards, glm::vec2 center, bool snap);

        // Drawing
        void DrawMesh(const std::string &mesh, glm::vec2 center, glm::vec2 size, glm::vec3 color, float angle = 0);
        void DrawRect(const Rect &rect, glm::vec3 color);
        void DrawOutline(const Rect &rect, glm::vec3 color, float thickness);

        void DrawPlayer(const Player &p, glm::vec3 boardColor, bool showHand);
        void DrawCardFront(const Card &card, glm::vec2 center);
        void DrawCardBack(glm::vec2 center);
        void DrawDeck(glm::vec2 center, int cardsLeft);
        void DrawLives(glm::vec2 start, int hp, int shield);

     private:
        // Logical scene size: the camera always shows this, whatever the window size
        static constexpr float kSceneWidth = 1280;
        static constexpr float kSceneHeight = 720;

        static constexpr float kCardWidth = 70;
        static constexpr float kCardHeight = 100;
        static constexpr float kCardSpacing = 15;

        static constexpr int kMaxHP = 15;
        static constexpr int kDeckSize = 20;
        static constexpr int kHandSize = 10;            // cards dealt to each player at the start
        static constexpr float kDealInterval = 0.15f;   // seconds between two dealt cards

        Player player;
        Player enemy;

        glm::vec2 mouseWorld;
        int draggedIndex;   // index in player.hand of the card being dragged, -1 if none

        int cardsToDeal;    // cards left to deal at the start (both players)
        float dealTimer;    // time until the next card is dealt
    };
}   // namespace m1
