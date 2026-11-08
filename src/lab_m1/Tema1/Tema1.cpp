#include "lab_m1/Tema1/Tema1.h"

#include <cstdlib>
#include <iostream>

#include "lab_m1/Tema1/transform2D.h"
#include "lab_m1/Tema1/object2D.h"

using namespace std;
using namespace m1;

namespace t2d = tema1::transform2D;


/*
 *  Scene coordinates are logical: (0, 0) bottom-left, (1280, 720) top-right,
 *  independent of the actual window size. Everything is drawn with depth test
 *  disabled, so draw order = layering (later calls appear on top).
 */


Tema1::Tema1()
{
}


Tema1::~Tema1()
{
}


void Tema1::Init()
{
    SetupCamera();
    CreateMeshes();

    // Zones (center, size)
    enemyHandZone  = { glm::vec2(640, 645), glm::vec2(900, 120) };
    enemyBoard     = { glm::vec2(600, 450), glm::vec2(800, 170) };
    playerBoard    = { glm::vec2(600, 280), glm::vec2(800, 170) };
    playerHandZone = { glm::vec2(640, 75),  glm::vec2(900, 120) };

    enemyDeckPos  = glm::vec2(1120, 470);
    playerDeckPos = glm::vec2(1120, 280);

    enemyLivesPos  = glm::vec2(20, 565);
    playerLivesPos = glm::vec2(20, 170);

    // Game state
    maxHP = 15;
    playerHP = enemyHP = maxHP;
    playerShield = enemyShield = 0;

    mouseWorld = glm::vec2(0);

    DealDemoHands();
}


void Tema1::SetupCamera()
{
    auto camera = GetSceneCamera();
    camera->SetOrthographic(0, kSceneWidth, 0, kSceneHeight, 0.01f, 400);
    camera->SetPosition(glm::vec3(0, 0, 50));
    camera->SetRotation(glm::vec3(0, 0, 0));
    camera->Update();
    GetCameraInput()->SetActive(false);
}


void Tema1::CreateMeshes()
{
    AddMeshToList(tema1::object2D::CreateSquare("square"));
    AddMeshToList(tema1::object2D::CreateCircle("circle"));
    AddMeshToList(tema1::object2D::CreateRing("ring", 0.6f));
    AddMeshToList(tema1::object2D::CreateTriangle("triangle"));
    AddMeshToList(tema1::object2D::CreateDiamond("diamond"));
    AddMeshToList(tema1::object2D::CreateStar("star"));
    AddMeshToList(tema1::object2D::CreateCross("cross"));
}


void Tema1::DealDemoHands()
{
    // Placeholder: random hands so the layout can be checked visually.
    // Will be replaced by real deck / dealing logic.
    playerHand.clear();
    enemyHand.clear();

    const int handSize = 10;
    for (int i = 0; i < handSize; i++) {
        Card c;
        c.type = (CardType)(rand() % (int)CardType::Count);
        c.value = 1 + rand() % 4;
        playerHand.push_back(c);

        Card e;
        e.type = (CardType)(rand() % (int)CardType::Count);
        e.value = 1 + rand() % 4;
        enemyHand.push_back(e);
    }

    playerDeckCount = 10;
    enemyDeckCount = 10;

    LayoutRow(playerHand, playerHandZone.center.x, playerHandZone.center.y, true);
    LayoutRow(enemyHand, enemyHandZone.center.x, enemyHandZone.center.y, true);
}


void Tema1::FrameStart()
{
    glClearColor(0.05f, 0.07f, 0.12f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 2D scene: rely on draw order instead of depth
    glDisable(GL_DEPTH_TEST);

    glm::ivec2 resolution = window->GetResolution();
    glViewport(0, 0, resolution.x, resolution.y);
}


void Tema1::Update(float deltaTimeSeconds)
{
    // Back to front
    DrawBoard();

    DrawDeck(enemyDeckPos, enemyDeckCount);
    DrawDeck(playerDeckPos, playerDeckCount);

    DrawLives(enemyLivesPos, enemyHP, enemyShield);
    DrawLives(playerLivesPos, playerHP, playerShield);

    for (const Card &c : enemyRow)   DrawCardFront(c, c.position);
    for (const Card &c : playerRow)  DrawCardFront(c, c.position);
    for (const Card &c : enemyHand)  DrawCardBack(c.position);
    for (const Card &c : playerHand) DrawCardFront(c, c.position);

    // TODO: dragged card is drawn last, on top of everything
}


void Tema1::FrameEnd()
{
    glEnable(GL_DEPTH_TEST);
}


/*
 *  Coordinates & layout
 */


glm::vec2 Tema1::ScreenToWorld(int mouseX, int mouseY) const
{
    glm::ivec2 res = window->GetResolution();
    float x = mouseX * kSceneWidth / res.x;
    float y = (res.y - mouseY) * kSceneHeight / res.y;
    return glm::vec2(x, y);
}


std::vector<glm::vec2> Tema1::ComputeRowPositions(int count, float centerX, float y, float spacing) const
{
    std::vector<glm::vec2> positions;
    if (count <= 0) {
        return positions;
    }

    float step = kCardWidth + spacing;
    float totalWidth = count * kCardWidth + (count - 1) * spacing;
    float firstX = centerX - totalWidth / 2 + kCardWidth / 2;

    for (int i = 0; i < count; i++) {
        positions.emplace_back(firstX + i * step, y);
    }
    return positions;
}


void Tema1::LayoutRow(std::vector<Card> &cards, float centerX, float y, bool snap)
{
    std::vector<glm::vec2> positions = ComputeRowPositions((int)cards.size(), centerX, y, kCardSpacing);
    for (size_t i = 0; i < cards.size(); i++) {
        cards[i].target = positions[i];
        if (snap) {
            cards[i].position = positions[i];
        }
    }
}


/*
 *  Drawing helpers
 */


void Tema1::DrawMesh(const std::string &mesh, glm::vec2 center, glm::vec2 size, glm::vec3 color, float angle)
{
    glm::mat3 model = glm::mat3(1);
    model *= t2d::Translate(center.x, center.y);
    model *= t2d::Rotate(angle);
    model *= t2d::Scale(size.x, size.y);
    RenderMesh2D(meshes[mesh], model, color);
}


void Tema1::DrawRect(const Rect &rect, glm::vec3 color)
{
    DrawMesh("square", rect.center, rect.size, color);
}


void Tema1::DrawOutline(const Rect &rect, glm::vec3 color, float thickness)
{
    glm::vec2 c = rect.center;
    glm::vec2 h = rect.size * 0.5f;
    DrawMesh("square", c + glm::vec2(0,  h.y), glm::vec2(rect.size.x + thickness, thickness), color);
    DrawMesh("square", c + glm::vec2(0, -h.y), glm::vec2(rect.size.x + thickness, thickness), color);
    DrawMesh("square", c + glm::vec2(-h.x, 0), glm::vec2(thickness, rect.size.y + thickness), color);
    DrawMesh("square", c + glm::vec2( h.x, 0), glm::vec2(thickness, rect.size.y + thickness), color);
}


void Tema1::DrawBoard()
{
    // Hand zones
    DrawRect(enemyHandZone,  glm::vec3(0.09f, 0.11f, 0.17f));
    DrawRect(playerHandZone, glm::vec3(0.09f, 0.11f, 0.17f));

    // Board halves
    DrawRect(enemyBoard,  glm::vec3(0.25f, 0.10f, 0.14f));
    DrawRect(playerBoard, glm::vec3(0.10f, 0.17f, 0.30f));
    DrawOutline(enemyBoard,  glm::vec3(0.55f, 0.25f, 0.30f), 2);
    DrawOutline(playerBoard, glm::vec3(0.25f, 0.40f, 0.65f), 2);
}


void Tema1::DrawCardFront(const Card &card, glm::vec2 center, float scale)
{
    glm::vec2 size = glm::vec2(kCardWidth, kCardHeight) * scale;
    glm::vec3 color = CardColor(card.type);

    // Border + body
    DrawRect({ center, size + glm::vec2(4 * scale) }, glm::vec3(0.9f));
    DrawRect({ center, size }, color);

    // Type symbol
    float symbolSize = 0.5f * size.x;
    glm::vec2 symbolPos = center + glm::vec2(0, 0.08f * size.y);
    float angle = 0;
    if (card.type == CardType::WarHorn) angle = glm::pi<float>();
    if (card.type == CardType::Scorch)  angle = glm::quarter_pi<float>();
    DrawMesh(CardSymbol(card.type), symbolPos, glm::vec2(symbolSize), SymbolColor(card.type), angle);

    // Temporary cards: small corner diamond marks the category
    if (card.IsTemporary()) {
        DrawMesh("diamond", center + glm::vec2(-0.32f * size.x, 0.38f * size.y), glm::vec2(0.18f * size.x), SymbolColor(card.type));
    }

    // Value V: one small square per unit, along the bottom edge
    float pip = 0.11f * size.x;
    float gap = 0.04f * size.x;
    float total = card.value * pip + (card.value - 1) * gap;
    float x0 = center.x - total / 2 + pip / 2;
    float y = center.y - 0.38f * size.y;
    for (int i = 0; i < card.value; i++) {
        glm::vec2 p = glm::vec2(x0 + i * (pip + gap), y);
        DrawMesh("square", p, glm::vec2(pip + 2 * scale), glm::vec3(0.1f));
        DrawMesh("square", p, glm::vec2(pip), glm::vec3(0.9f, 0.2f, 0.2f));
    }
}


void Tema1::DrawCardBack(glm::vec2 center, float scale)
{
    glm::vec2 size = glm::vec2(kCardWidth, kCardHeight) * scale;
    glm::vec3 border = glm::vec3(0.35f, 0.5f, 0.8f);

    DrawRect({ center, size + glm::vec2(4 * scale) }, border);
    DrawRect({ center, size }, glm::vec3(0.12f, 0.2f, 0.38f));

    // Simple pattern: an X across the card
    float diag = glm::length(size) * 0.85f;
    float a = glm::atan(size.y, size.x);
    DrawMesh("square", center, glm::vec2(diag, 2 * scale), border,  a);
    DrawMesh("square", center, glm::vec2(diag, 2 * scale), border, -a);
}


void Tema1::DrawDeck(glm::vec2 center, int cardsLeft)
{
    if (cardsLeft <= 0) {
        DrawOutline({ center, glm::vec2(kCardWidth, kCardHeight) }, glm::vec3(0.3f), 2);
        return;
    }

    // A few stacked backs, the top one at `center`
    int layers = glm::min(cardsLeft, 3);
    for (int i = layers - 1; i >= 0; i--) {
        DrawCardBack(center + glm::vec2(4.0f * i, -4.0f * i));
    }
}


void Tema1::DrawLives(glm::vec2 start, int hp, int shield)
{
    const float cell = 10;
    const float step = 13;

    // Life: one square per unit, missing life shown dark
    for (int i = 0; i < maxHP; i++) {
        glm::vec3 color = (i < hp) ? glm::vec3(0.35f, 0.8f, 0.4f) : glm::vec3(0.18f, 0.2f, 0.22f);
        DrawMesh("square", start + glm::vec2(cell / 2 + i * step, 0), glm::vec2(cell), color);
    }

    // Shield: separate row of blue squares just below
    for (int i = 0; i < shield; i++) {
        DrawMesh("square", start + glm::vec2(cell / 2 + i * step, -step), glm::vec2(cell), glm::vec3(0.3f, 0.55f, 1.0f));
    }
}


glm::vec3 Tema1::CardColor(CardType type)
{
    switch (type) {
    case CardType::Damage:       return glm::vec3(0.80f, 0.22f, 0.22f);
    case CardType::Heal:         return glm::vec3(0.25f, 0.65f, 0.35f);
    case CardType::Shield:       return glm::vec3(0.22f, 0.42f, 0.80f);
    case CardType::Spy:          return glm::vec3(0.08f, 0.08f, 0.08f);
    case CardType::FrostWeather: return glm::vec3(0.92f, 0.94f, 0.97f);
    case CardType::ClearWeather: return glm::vec3(0.90f, 0.75f, 0.20f);
    case CardType::WarHorn:      return glm::vec3(0.95f, 0.50f, 0.15f);
    case CardType::Scorch:       return glm::vec3(0.50f, 0.25f, 0.70f);
    default:                     return glm::vec3(1, 0, 1);
    }
}


glm::vec3 Tema1::SymbolColor(CardType type)
{
    switch (type) {
    case CardType::FrostWeather: return glm::vec3(0.35f, 0.65f, 0.95f);
    case CardType::ClearWeather: return glm::vec3(0.45f, 0.30f, 0.05f);
    case CardType::Spy:          return glm::vec3(0.75f, 0.75f, 0.75f);
    default:                     return glm::vec3(1.0f, 0.95f, 0.9f);
    }
}


std::string Tema1::CardSymbol(CardType type)
{
    switch (type) {
    case CardType::Damage:       return "triangle";
    case CardType::Heal:         return "cross";
    case CardType::Shield:       return "circle";
    case CardType::Spy:          return "diamond";
    case CardType::FrostWeather: return "star";
    case CardType::ClearWeather: return "ring";
    case CardType::WarHorn:      return "triangle";   // drawn upside down
    case CardType::Scorch:       return "cross";      // drawn rotated (X)
    default:                     return "square";
    }
}


/*
 *  Input callbacks (see `input_controller.h`)
 */


void Tema1::OnInputUpdate(float deltaTime, int mods)
{
}


void Tema1::OnKeyPress(int key, int mods)
{
    // Debug: re-deal random hands
    if (key == GLFW_KEY_R) {
        DealDemoHands();
    }
}


void Tema1::OnKeyRelease(int key, int mods)
{
}


void Tema1::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    mouseWorld = ScreenToWorld(mouseX, mouseY);
}


void Tema1::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    mouseWorld = ScreenToWorld(mouseX, mouseY);
    // TODO: drag & drop - select card under cursor
}


void Tema1::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    mouseWorld = ScreenToWorld(mouseX, mouseY);
    // TODO: drag & drop - drop on playerBoard or return to hand
}


void Tema1::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}


void Tema1::OnWindowResize(int width, int height)
{
    // Camera keeps the logical 1280x720 space; ScreenToWorld handles the scaling
}
