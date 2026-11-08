#pragma once

#include <string>
#include <vector>

#include "components/simple_scene.h"


namespace m1
{
    // Card types (Gwent). Permanent: Damage, Heal, Shield. The rest are temporary.
    enum class CardType
    {
        Damage,         // red
        Heal,           // green
        Shield,         // blue
        Spy,            // black
        FrostWeather,   // white
        ClearWeather,   // yellow
        WarHorn,        // orange
        Scorch,         // purple
        Count
    };

    struct Card
    {
        CardType type = CardType::Damage;
        int value = 1;

        glm::vec2 position = glm::vec2(0);  // current (drawn) center
        glm::vec2 target = glm::vec2(0);    // where it should move to (layout / animations)

        bool IsTemporary() const { return type != CardType::Damage && type != CardType::Heal && type != CardType::Shield; }
    };

    // Axis-aligned rectangle, used both for drawing and for hit tests (drag & drop)
    struct Rect
    {
        glm::vec2 center;
        glm::vec2 size;

        Rect() : center(0), size(0) {}
        Rect(glm::vec2 center, glm::vec2 size) : center(center), size(size) {}

        bool Contains(const glm::vec2 &p) const
        {
            glm::vec2 half = size * 0.5f;
            return center.x - half.x <= p.x && p.x <= center.x + half.x
                && center.y - half.y <= p.y && p.y <= center.y + half.y;
        }
    };


    class Tema1 : public gfxc::SimpleScene
    {
     public:
        Tema1();
        ~Tema1();

        void Init() override;

     private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

        // Setup
        void CreateMeshes();
        void SetupCamera();
        void DealDemoHands();

        // Coordinates: window pixels (origin top-left) -> logical scene (origin bottom-left)
        glm::vec2 ScreenToWorld(int mouseX, int mouseY) const;

        // Layout: centered horizontal row of `count` cards
        std::vector<glm::vec2> ComputeRowPositions(int count, float centerX, float y, float spacing) const;
        void LayoutRow(std::vector<Card> &cards, float centerX, float y, bool snap);

        // Drawing helpers
        void DrawMesh(const std::string &mesh, glm::vec2 center, glm::vec2 size, glm::vec3 color, float angle = 0);
        void DrawRect(const Rect &rect, glm::vec3 color);
        void DrawOutline(const Rect &rect, glm::vec3 color, float thickness);

        void DrawBoard();
        void DrawCardFront(const Card &card, glm::vec2 center, float scale = 1);
        void DrawCardBack(glm::vec2 center, float scale = 1);
        void DrawDeck(glm::vec2 center, int cardsLeft);
        void DrawLives(glm::vec2 start, int hp, int shield);

        static glm::vec3 CardColor(CardType type);
        static glm::vec3 SymbolColor(CardType type);
        static std::string CardSymbol(CardType type);

     protected:
        // Logical scene size (camera is orthographic over this, independent of window size)
        static constexpr float kSceneWidth = 1280;
        static constexpr float kSceneHeight = 720;

        static constexpr float kCardWidth = 70;
        static constexpr float kCardHeight = 100;
        static constexpr float kCardSpacing = 15;

        // Scene zones
        Rect enemyHandZone, playerHandZone;
        Rect enemyBoard, playerBoard;
        glm::vec2 enemyDeckPos, playerDeckPos;
        glm::vec2 enemyLivesPos, playerLivesPos;

        // Game state
        std::vector<Card> playerHand, enemyHand;
        std::vector<Card> playerRow, enemyRow;
        int playerDeckCount, enemyDeckCount;

        int maxHP;
        int playerHP, enemyHP;
        int playerShield, enemyShield;

        // Input
        glm::vec2 mouseWorld;
    };
}   // namespace m1
