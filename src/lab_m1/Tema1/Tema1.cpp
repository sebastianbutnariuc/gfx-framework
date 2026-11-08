#include "lab_m1/Tema1/Tema1.h"

#include <iostream>

#include "lab_m1/Tema1/transform2D.h"
#include "lab_m1/Tema1/object2D.h"
#include "lab_m1/Tema1/Rules.h"

using namespace std;
using namespace m1;

namespace t2d = tema1::transform2D;


/*
 *  Scene coordinates are logical: (0, 0) bottom-left, (1280, 720) top-right.
 *  Depth test is off, so draw order = layering (what is drawn last is on top).
 */


void Tema1::Init()
{
    // Camera: orthographic over the logical scene
    auto camera = GetSceneCamera();
    camera->SetOrthographic(0, kSceneWidth, 0, kSceneHeight, 0.01f, 400);
    camera->SetPosition(glm::vec3(0, 0, 50));
    camera->SetRotation(glm::vec3(0, 0, 0));
    camera->Update();
    GetCameraInput()->SetActive(false);

    // Meshes
    AddMeshToList(tema1::object2D::CreateSquare("square"));
    AddMeshToList(tema1::object2D::CreateCircle("circle"));
    AddMeshToList(tema1::object2D::CreateRing("ring", 0.6f));
    AddMeshToList(tema1::object2D::CreateTriangle("triangle"));
    AddMeshToList(tema1::object2D::CreateDiamond("diamond"));
    AddMeshToList(tema1::object2D::CreateStar("star"));
    AddMeshToList(tema1::object2D::CreateCross("cross"));

    // Layout: enemy on top, player on the bottom
    enemy.handZone  = Rect(glm::vec2(640, 645), glm::vec2(900, 120));
    enemy.boardZone = Rect(glm::vec2(600, 450), glm::vec2(800, 170));
    enemy.deckPos   = glm::vec2(1120, 470);
    enemy.livesPos  = glm::vec2(20, 565);

    player.handZone  = Rect(glm::vec2(640, 75),  glm::vec2(900, 120));
    player.boardZone = Rect(glm::vec2(600, 280), glm::vec2(800, 170));
    player.deckPos   = glm::vec2(1120, 280);
    player.livesPos  = glm::vec2(20, 170);

    enemy.hiddenHand = true;

    mouseWorld = glm::vec2(0);

    NewGame();
}


void Tema1::NewGame()
{
    // Random deck for each player; hands are dealt one card at a time in Update
    for (Player *p : { &player, &enemy }) {
        p->deck.clear();
        p->hand.clear();
        p->row.clear();
        p->EndTurn();
        for (int i = 0; i < kDeckSize; i++) {
            Card card = Card::Random();
            if (p->hiddenHand) {
                card.faceUp = false;    // enemy cards stay hidden until played
                card.flip = 0;
            }
            p->deck.push_back(card);
        }
        p->hp = kMaxHP;
        p->shield = 0;
    }

    draggedIndex = -1;
    phase = Phase::Dealing;
    timer = 0;
    cardsToDeal = 2 * kHandSize;
}


void Tema1::DealCard(Player &p)
{
    // The new card starts on the deck, the layout gives it its slot in the hand -> it slides there
    p.DrawFromDeck();
    LayoutAll();
}


void Tema1::EnemyPlayCard()
{
    if (enemy.hand.empty()) {
        return;
    }

    // Random card from the hand -> enemy board, face down; it flips once it arrives (Card::Animate)
    enemy.PlayCard(rand() % (int)enemy.hand.size());
    LayoutAll();
}


bool Tema1::IsGameOver() const
{
    // Someone has no life left, or nobody has cards left to play
    return player.hp == 0 || enemy.hp == 0 || (player.hand.empty() && enemy.hand.empty());
}


void Tema1::FrameStart()
{
    glClearColor(0.05f, 0.07f, 0.12f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    // Framebuffer size in pixels (2x the window size on Retina).
    // window->GetResolution() applies the scale twice on HiDPI screens, so ask GLFW directly.
    int width, height;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);
    glViewport(0, 0, width, height);
}


void Tema1::Update(float deltaTimeSeconds)
{
    timer -= deltaTimeSeconds;

    switch (phase) {
    case Phase::Dealing:
        // One card every kDealInterval seconds, alternating player / enemy
        if (timer <= 0) {
            DealCard(cardsToDeal % 2 == 0 ? player : enemy);
            cardsToDeal--;
            timer = kDealInterval;
        }
        if (cardsToDeal == 0) {
            phase = Phase::PlayerTurn;
        }
        break;

    case Phase::EnemyTurn:
        if (timer <= 0) {
            EnemyPlayCard();
            phase = Phase::Reveal;
        }
        break;

    case Phase::Reveal:
        // Wait until the enemy card is fully turned face up, then apply the effects
        if (enemy.playedIndex == -1 || enemy.row[enemy.playedIndex].flip >= 1) {
            Rules(kMaxHP).ResolveTurn(player, enemy);
            LayoutAll();    // cards stolen by Spy / destroyed cards

            if (IsGameOver()) {
                phase = Phase::GameOver;
                cout << "Game over: " << (player.hp > enemy.hp ? "you win" : player.hp < enemy.hp ? "you lose" : "draw")
                     << " (" << player.hp << " - " << enemy.hp << ")\n";
            } else {
                phase = Phase::PlayerTurn;
            }
        }
        break;

    case Phase::PlayerTurn:
        // No cards left (but the enemy still has some): skip to the enemy's move
        if (player.hand.empty()) {
            phase = Phase::EnemyTurn;
            timer = kEnemyDelay;
        }
        break;

    case Phase::GameOver:
        break;
    }

    player.Animate(deltaTimeSeconds);
    enemy.Animate(deltaTimeSeconds);

    DrawPlayer(enemy, glm::vec3(0.25f, 0.10f, 0.14f));
    DrawPlayer(player, glm::vec3(0.10f, 0.17f, 0.30f));

    // The dragged card is drawn last, on top of everything
    if (draggedIndex != -1) {
        DrawCard(player.hand[draggedIndex]);
    }

    if (phase == Phase::GameOver) {
        DrawGameOver();
    }
}


glm::vec2 Tema1::ScreenToWorld(int mouseX, int mouseY) const
{
    // Mouse coordinates are in window points, not framebuffer pixels
    int width, height;
    glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
    return glm::vec2(mouseX * kSceneWidth / width, (height - mouseY) * kSceneHeight / height);
}


void Tema1::LayoutRow(std::vector<Card> &cards, glm::vec2 center)
{
    float step = kCardWidth + kCardSpacing;
    float firstX = center.x - step * (cards.size() - 1) / 2;

    for (size_t i = 0; i < cards.size(); i++) {
        cards[i].target = glm::vec2(firstX + i * step, center.y);
    }
}


void Tema1::LayoutAll()
{
    for (Player *p : { &player, &enemy }) {
        LayoutRow(p->hand, p->handZone.center);
        LayoutRow(p->row, p->boardZone.center);
    }
}


/*
 *  Drawing
 */


void Tema1::DrawMesh(const std::string &mesh, glm::vec2 center, glm::vec2 size, glm::vec3 color,
                     float angle, const glm::mat3 &parent)
{
    glm::mat3 model = parent * t2d::Translate(center.x, center.y) * t2d::Rotate(angle) * t2d::Scale(size.x, size.y);
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
    glm::vec2 horizontal = glm::vec2(rect.size.x + thickness, thickness);
    glm::vec2 vertical = glm::vec2(thickness, rect.size.y + thickness);

    DrawMesh("square", c + glm::vec2(0,  h.y), horizontal, color);
    DrawMesh("square", c + glm::vec2(0, -h.y), horizontal, color);
    DrawMesh("square", c + glm::vec2(-h.x, 0), vertical, color);
    DrawMesh("square", c + glm::vec2( h.x, 0), vertical, color);
}


void Tema1::DrawPlayer(const Player &p, glm::vec3 boardColor)
{
    DrawRect(p.handZone, glm::vec3(0.09f, 0.11f, 0.17f));
    DrawRect(p.boardZone, boardColor);
    DrawOutline(p.boardZone, boardColor * 2.0f, 2);

    DrawDeck(p.deckPos, (int)p.deck.size());
    DrawLives(p.livesPos, p.hp, p.shield);

    for (const Card &c : p.row) {
        DrawCard(c);
    }
    for (size_t i = 0; i < p.hand.size(); i++) {
        if (&p == &player && (int)i == draggedIndex) {
            continue;   // drawn separately, on top
        }
        DrawCard(p.hand[i]);
    }
}


void Tema1::DrawCard(const Card &card)
{
    // Flip = rotation around the vertical axis, seen from the front: the card gets narrower on X
    // (|cos| goes 1 -> 0 -> 1); the back is visible in the first half, the front in the second
    float angle = card.flip * glm::pi<float>();
    glm::mat3 model = t2d::Translate(card.position.x, card.position.y) * t2d::Scale(glm::abs(glm::cos(angle)), 1);

    if (card.flip > 0.5f) {
        DrawCardFront(card, model);
    } else {
        DrawCardBack(model);
    }
}


// The card details are drawn relative to the card's center (0, 0), then `model` places the whole card
void Tema1::DrawCardFront(const Card &card, const glm::mat3 &model)
{
    glm::vec2 size = glm::vec2(kCardWidth, kCardHeight);

    // Border + body
    DrawMesh("square", glm::vec2(0), size + glm::vec2(4), glm::vec3(0.9f), 0, model);
    DrawMesh("square", glm::vec2(0), size, card.GetColor(), 0, model);

    // Type symbol
    DrawMesh(card.GetSymbolMesh(), glm::vec2(0, 8), glm::vec2(35), card.GetSymbolColor(), card.GetSymbolAngle(), model);

    // Temporary cards: small diamond in the corner
    if (card.IsTemporary()) {
        DrawMesh("diamond", glm::vec2(-22, 38), glm::vec2(13), card.GetSymbolColor(), 0, model);
    }

    // Value V: one small square per unit, centered along the bottom edge
    // (squares get smaller for large values, e.g. after War Horn, to stay inside the card)
    float step = glm::min(11.0f, 60.0f / card.value);
    float pip = 0.75f * step;
    float firstX = -step * (card.value - 1) / 2;
    for (int i = 0; i < card.value; i++) {
        glm::vec2 p = glm::vec2(firstX + i * step, -38);
        DrawMesh("square", p, glm::vec2(pip + 2), glm::vec3(0.1f), 0, model);
        DrawMesh("square", p, glm::vec2(pip), glm::vec3(0.9f, 0.2f, 0.2f), 0, model);
    }
}


void Tema1::DrawCardBack(const glm::mat3 &model)
{
    glm::vec2 size = glm::vec2(kCardWidth, kCardHeight);
    glm::vec3 border = glm::vec3(0.35f, 0.5f, 0.8f);

    DrawMesh("square", glm::vec2(0), size + glm::vec2(4), border, 0, model);
    DrawMesh("square", glm::vec2(0), size, glm::vec3(0.12f, 0.2f, 0.38f), 0, model);

    // Pattern: an X across the card
    float diagonal = glm::length(size) * 0.85f;
    float angle = glm::atan(size.y, size.x);
    DrawMesh("square", glm::vec2(0), glm::vec2(diagonal, 2), border,  angle, model);
    DrawMesh("square", glm::vec2(0), glm::vec2(diagonal, 2), border, -angle, model);
}


void Tema1::DrawDeck(glm::vec2 center, int cardsLeft)
{
    if (cardsLeft <= 0) {
        DrawOutline(Rect(center, glm::vec2(kCardWidth, kCardHeight)), glm::vec3(0.3f), 2);
        return;
    }

    // Up to 3 stacked backs, the top one at `center`
    for (int i = glm::min(cardsLeft, 3) - 1; i >= 0; i--) {
        DrawCardBack(t2d::Translate(center.x + 4.0f * i, center.y - 4.0f * i));
    }
}


void Tema1::DrawLives(glm::vec2 start, int hp, int shield)
{
    const float cell = 10, step = 13;

    // One square per life unit, lost ones in dark gray
    for (int i = 0; i < kMaxHP; i++) {
        glm::vec3 color = (i < hp) ? glm::vec3(0.35f, 0.8f, 0.4f) : glm::vec3(0.18f, 0.2f, 0.22f);
        DrawMesh("square", start + glm::vec2(i * step, 0), glm::vec2(cell), color);
    }

    // Shield on a separate row, in blue
    for (int i = 0; i < shield; i++) {
        DrawMesh("square", start + glm::vec2(i * step, -step), glm::vec2(cell), glm::vec3(0.3f, 0.55f, 1.0f));
    }
}


void Tema1::DrawGameOver()
{
    // Banner over the board: green + star = win, red + X = loss, gray + ring = draw
    glm::vec3 color = glm::vec3(0.4f);
    std::string symbol = "ring";
    float angle = 0;

    if (player.hp > enemy.hp) {
        color = glm::vec3(0.25f, 0.65f, 0.35f);
        symbol = "star";
    } else if (player.hp < enemy.hp) {
        color = glm::vec3(0.80f, 0.22f, 0.22f);
        symbol = "cross";
        angle = glm::quarter_pi<float>();
    }

    Rect banner(glm::vec2(600, 365), glm::vec2(500, 140));
    DrawRect(banner, color);
    DrawOutline(banner, glm::vec3(0.95f), 4);
    DrawMesh(symbol, banner.center, glm::vec2(90), glm::vec3(0.95f), angle);
}


/*
 *  Input
 */


void Tema1::OnKeyPress(int key, int mods)
{
    // Debug: restart with new random decks
    if (key == GLFW_KEY_R) {
        NewGame();
    }
}


void Tema1::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    mouseWorld = ScreenToWorld(mouseX, mouseY);

    // The dragged card follows the cursor (target too, so the animation doesn't pull it back)
    if (draggedIndex != -1) {
        Card &card = player.hand[draggedIndex];
        card.position = card.target = mouseWorld;
    }
}


void Tema1::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Cards can be picked up only during the player's turn
    if (!IS_BIT_SET(button, GLFW_MOUSE_BUTTON_LEFT) || phase != Phase::PlayerTurn) {
        return;
    }
    mouseWorld = ScreenToWorld(mouseX, mouseY);

    // Select the card under the cursor
    for (size_t i = 0; i < player.hand.size(); i++) {
        if (Rect(player.hand[i].position, glm::vec2(kCardWidth, kCardHeight)).Contains(mouseWorld)) {
            draggedIndex = (int)i;
            return;
        }
    }
}


void Tema1::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    if (!IS_BIT_SET(button, GLFW_MOUSE_BUTTON_LEFT) || draggedIndex == -1) {
        return;
    }
    mouseWorld = ScreenToWorld(mouseX, mouseY);

    // Valid drop: move the card from the hand to the board row; the enemy answers after a short pause
    if (player.boardZone.Contains(mouseWorld)) {
        player.PlayCard(draggedIndex);
        phase = Phase::EnemyTurn;
        timer = kEnemyDelay;
    }

    // Recompute slots: the remaining cards (or the rejected one) slide into place
    draggedIndex = -1;
    LayoutAll();
}
