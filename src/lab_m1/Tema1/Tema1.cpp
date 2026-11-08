#include "lab_m1/Tema1/Tema1.h"

#include "lab_m1/Tema1/transform2D.h"
#include "lab_m1/Tema1/object2D.h"

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

    mouseWorld = glm::vec2(0);

    DealDemoHands();
}


void Tema1::DealDemoHands()
{
    // Placeholder until the real deck logic: 10 random cards each
    for (Player *p : { &player, &enemy }) {
        p->hand.clear();
        p->row.clear();
        for (int i = 0; i < 10; i++) {
            p->hand.push_back(Card::Random());
        }
        p->deckCount = 10;
        p->hp = kMaxHP;
        p->shield = 0;
        LayoutRow(p->hand, p->handZone.center, true);
    }
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
    DrawPlayer(enemy, glm::vec3(0.25f, 0.10f, 0.14f), false);
    DrawPlayer(player, glm::vec3(0.10f, 0.17f, 0.30f), true);

    // TODO: dragged card is drawn last, on top of everything
}


glm::vec2 Tema1::ScreenToWorld(int mouseX, int mouseY) const
{
    // Mouse coordinates are in window points, not framebuffer pixels
    int width, height;
    glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
    return glm::vec2(mouseX * kSceneWidth / width, (height - mouseY) * kSceneHeight / height);
}


void Tema1::LayoutRow(std::vector<Card> &cards, glm::vec2 center, bool snap)
{
    float step = kCardWidth + kCardSpacing;
    float firstX = center.x - step * (cards.size() - 1) / 2;

    for (size_t i = 0; i < cards.size(); i++) {
        cards[i].target = glm::vec2(firstX + i * step, center.y);
        if (snap) {
            cards[i].position = cards[i].target;
        }
    }
}


/*
 *  Drawing
 */


void Tema1::DrawMesh(const std::string &mesh, glm::vec2 center, glm::vec2 size, glm::vec3 color, float angle)
{
    glm::mat3 model = t2d::Translate(center.x, center.y) * t2d::Rotate(angle) * t2d::Scale(size.x, size.y);
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


void Tema1::DrawPlayer(const Player &p, glm::vec3 boardColor, bool showHand)
{
    DrawRect(p.handZone, glm::vec3(0.09f, 0.11f, 0.17f));
    DrawRect(p.boardZone, boardColor);
    DrawOutline(p.boardZone, boardColor * 2.0f, 2);

    DrawDeck(p.deckPos, p.deckCount);
    DrawLives(p.livesPos, p.hp, p.shield);

    for (const Card &c : p.row) {
        DrawCardFront(c, c.position);
    }
    for (const Card &c : p.hand) {
        if (showHand) {
            DrawCardFront(c, c.position);
        } else {
            DrawCardBack(c.position);
        }
    }
}


void Tema1::DrawCardFront(const Card &card, glm::vec2 center)
{
    glm::vec2 size = glm::vec2(kCardWidth, kCardHeight);

    // Border + body
    DrawRect(Rect(center, size + glm::vec2(4)), glm::vec3(0.9f));
    DrawRect(Rect(center, size), card.GetColor());

    // Type symbol
    DrawMesh(card.GetSymbolMesh(), center + glm::vec2(0, 8), glm::vec2(35), card.GetSymbolColor(), card.GetSymbolAngle());

    // Temporary cards: small diamond in the corner
    if (card.IsTemporary()) {
        DrawMesh("diamond", center + glm::vec2(-22, 38), glm::vec2(13), card.GetSymbolColor());
    }

    // Value V: one small square per unit, centered along the bottom edge
    const float pip = 8, step = 11;
    float firstX = center.x - step * (card.value - 1) / 2;
    for (int i = 0; i < card.value; i++) {
        glm::vec2 p = glm::vec2(firstX + i * step, center.y - 38);
        DrawMesh("square", p, glm::vec2(pip + 2), glm::vec3(0.1f));
        DrawMesh("square", p, glm::vec2(pip), glm::vec3(0.9f, 0.2f, 0.2f));
    }
}


void Tema1::DrawCardBack(glm::vec2 center)
{
    glm::vec2 size = glm::vec2(kCardWidth, kCardHeight);
    glm::vec3 border = glm::vec3(0.35f, 0.5f, 0.8f);

    DrawRect(Rect(center, size + glm::vec2(4)), border);
    DrawRect(Rect(center, size), glm::vec3(0.12f, 0.2f, 0.38f));

    // Pattern: an X across the card
    float diagonal = glm::length(size) * 0.85f;
    float angle = glm::atan(size.y, size.x);
    DrawMesh("square", center, glm::vec2(diagonal, 2), border,  angle);
    DrawMesh("square", center, glm::vec2(diagonal, 2), border, -angle);
}


void Tema1::DrawDeck(glm::vec2 center, int cardsLeft)
{
    if (cardsLeft <= 0) {
        DrawOutline(Rect(center, glm::vec2(kCardWidth, kCardHeight)), glm::vec3(0.3f), 2);
        return;
    }

    // Up to 3 stacked backs, the top one at `center`
    for (int i = glm::min(cardsLeft, 3) - 1; i >= 0; i--) {
        DrawCardBack(center + glm::vec2(4.0f * i, -4.0f * i));
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


/*
 *  Input
 */


void Tema1::OnKeyPress(int key, int mods)
{
    // Debug: deal new random hands
    if (key == GLFW_KEY_R) {
        DealDemoHands();
    }
}


void Tema1::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    mouseWorld = ScreenToWorld(mouseX, mouseY);
    // TODO: drag & drop - move the selected card
}


void Tema1::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    mouseWorld = ScreenToWorld(mouseX, mouseY);
    // TODO: drag & drop - select the card under the cursor
}


void Tema1::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    mouseWorld = ScreenToWorld(mouseX, mouseY);
    // TODO: drag & drop - drop on the board or send back to hand
}
