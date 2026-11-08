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
    AddMeshToList(tema1::object2D::CreatePolygon("circle", 40));
    AddMeshToList(tema1::object2D::CreatePolygon("hexagon", 6));
    AddMeshToList(tema1::object2D::CreateRing("ring", 0.6f));
    AddMeshToList(tema1::object2D::CreateTriangle("triangle"));
    AddMeshToList(tema1::object2D::CreateDiamond("diamond"));
    AddMeshToList(tema1::object2D::CreateStar("star"));
    AddMeshToList(tema1::object2D::CreateCross("cross"));

    // Layout: enemy on top, player on the bottom
    enemy.handZone = Rect(glm::vec2(640, 655), glm::vec2(900, 110));
    enemy.deckPos  = glm::vec2(1120, 480);
    enemy.livesPos = glm::vec2(20, 590);
    enemy.crystalsPos = glm::vec2(20, 615);
    enemy.leaderPos = glm::vec2(125, 460);

    player.handZone = Rect(glm::vec2(640, 65), glm::vec2(900, 110));
    player.deckPos  = glm::vec2(1120, 240);
    player.livesPos = glm::vec2(20, 140);
    player.crystalsPos = glm::vec2(20, 165);
    player.leaderPos = glm::vec2(125, 260);

    // Lanes: melee next to the middle line, then ranged, then siege (mirrored for the enemy)
    for (int i = 0; i < kLaneCount; i++) {
        float offset = kLaneHeight * (i + 0.5f);
        glm::vec2 size = glm::vec2(kBoardWidth, kLaneHeight);
        player.lanes[i] = Rect(glm::vec2(kBoardCenterX, kMiddleY - offset), size);
        enemy.lanes[i]  = Rect(glm::vec2(kBoardCenterX, kMiddleY + offset), size);
    }

    enemy.hiddenHand = true;

    mouseWorld = glm::vec2(0);

    leaders = Leader::All();
    ShowLeaderMenu();
}


void Tema1::ShowLeaderMenu()
{
    phase = Phase::LeaderSelect;
    selectedLeader = -1;
    draggedIndex = -1;
}


Rect Tema1::LeaderMenuSlot(int index) const
{
    // Leaders side by side, centered on the screen
    float offset = index - (leaders.size() - 1) / 2.0f;
    return Rect(glm::vec2(kSceneWidth / 2 + offset * 200, kSceneHeight / 2), glm::vec2(120));
}


void Tema1::NewGame()
{
    // Chosen leader for the player, random one for the enemy
    player.leader = leaders[selectedLeader];
    enemy.leader = leaders[rand() % leaders.size()];

    // Random deck for each player; hands are dealt one card at a time in Update
    for (Player *p : { &player, &enemy }) {
        p->deck.clear();
        p->hand.clear();
        p->row.clear();
        p->graveyard.clear();
        p->NewRound(kMaxHP);
        p->crystals = kCrystals;
        for (int i = 0; i < kDeckSize; i++) {
            Card card = Card::Random();
            if (p->hiddenHand) {
                card.faceUp = false;    // enemy cards stay hidden until played
                card.flip = 0;
            }
            p->deck.push_back(card);
        }
    }

    draggedIndex = -1;
    phase = Phase::Dealing;
    timer = 0;
    cardsToDeal = 2 * kHandSize;
    roundResult = 0;
}


void Tema1::DealCard(Player &p)
{
    // The new card starts on the deck, the layout gives it its slot in the hand -> it slides there
    p.DrawFromDeck();
    LayoutAll();
}


void Tema1::EnemyPlayCard()
{
    // Random playable card from the hand
    std::vector<int> playable;
    for (size_t i = 0; i < enemy.hand.size(); i++) {
        if (enemy.CanPlay(enemy.hand[i])) {
            playable.push_back((int)i);
        }
    }
    if (playable.empty()) {
        return;
    }
    int index = playable[rand() % playable.size()];

    // Decoy -> over a random own board card; otherwise -> its lane (random lane if it fits anywhere).
    // The card arrives face down and flips once it gets there (Card::Animate)
    if (enemy.hand[index].type == CardType::Decoy) {
        enemy.PlayDecoy(index, rand() % (int)enemy.row.size());
    } else {
        Lane lane = enemy.hand[index].lane;
        if (lane == Lane::Any) {
            lane = (Lane)(rand() % kLaneCount);
        }
        enemy.PlayCard(index, lane);
    }
    LayoutAll();
}


bool Tema1::IsRoundOver() const
{
    // Someone has no life left, or nobody has cards left to play
    return player.hp == 0 || enemy.hp == 0 || (!player.HasPlayableCard() && !enemy.HasPlayableCard());
}


void Tema1::EndRound()
{
    // Less life loses a crystal; equal life (draw) -> both lose one
    roundResult = (player.hp > enemy.hp) - (player.hp < enemy.hp);
    if (roundResult <= 0) player.crystals--;
    if (roundResult >= 0) enemy.crystals--;

    cout << "Round over: " << (roundResult > 0 ? "you win" : roundResult < 0 ? "you lose" : "draw")
         << " (" << player.hp << " - " << enemy.hp << ")\n";

    phase = Phase::RoundOver;
    timer = kRoundPause;
}


void Tema1::StartRound()
{
    // Board cleared, life reset; no new hands, just one more card for each player
    player.NewRound(kMaxHP);
    enemy.NewRound(kMaxHP);

    phase = Phase::Dealing;
    timer = 0;
    cardsToDeal = 2;
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
    // Before the game: only the leader menu
    if (phase == Phase::LeaderSelect) {
        DrawLeaderMenu();
        return;
    }

    timer -= deltaTimeSeconds;

    switch (phase) {
    case Phase::LeaderSelect:
        break;

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

            if (IsRoundOver()) {
                EndRound();
            } else {
                phase = Phase::PlayerTurn;
            }
        }
        break;

    case Phase::RoundOver:
        // After the pause: next round, or the end of the game if someone has no crystals left
        if (timer <= 0) {
            if (player.crystals == 0 || enemy.crystals == 0) {
                phase = Phase::GameOver;
            } else {
                StartRound();
            }
        }
        break;

    case Phase::PlayerTurn:
        // No playable cards left (but the enemy still has some): skip to the enemy's move
        if (!player.HasPlayableCard()) {
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

    if (phase == Phase::RoundOver) {
        DrawResultBanner(roundResult, glm::vec2(300, 90));
    }
    if (phase == Phase::GameOver) {
        int gameResult = (player.crystals > enemy.crystals) - (player.crystals < enemy.crystals);
        DrawResultBanner(gameResult, glm::vec2(500, 140));
    }
}


glm::vec2 Tema1::ScreenToWorld(int mouseX, int mouseY) const
{
    // Mouse coordinates are in window points, not framebuffer pixels
    int width, height;
    glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
    return glm::vec2(mouseX * kSceneWidth / width, (height - mouseY) * kSceneHeight / height);
}


void Tema1::LayoutRow(const std::vector<Card*> &cards, glm::vec2 center, float step)
{
    float firstX = center.x - step * (cards.size() - 1) / 2;

    for (size_t i = 0; i < cards.size(); i++) {
        cards[i]->target = glm::vec2(firstX + i * step, center.y);
    }
}


void Tema1::LayoutAll()
{
    for (Player *p : { &player, &enemy }) {
        // Hand: one row
        std::vector<Card*> hand;
        for (Card &c : p->hand) {
            hand.push_back(&c);
        }
        LayoutRow(hand, p->handZone.center, kCardWidth + kCardSpacing);

        // Board: one row per lane, with the smaller board cards
        for (int lane = 0; lane < kLaneCount; lane++) {
            std::vector<Card*> cards;
            for (Card &c : p->row) {
                if ((int)c.placedLane == lane) {
                    cards.push_back(&c);
                }
            }
            LayoutRow(cards, p->lanes[lane].center, kCardWidth * kBoardScale + 8);
        }
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

    // Lanes; while dragging, the ones where the card can be dropped are highlighted
    for (int i = 0; i < kLaneCount; i++) {
        bool highlight = (&p == &player && draggedIndex != -1 && player.CanPlayOn(player.hand[draggedIndex], (Lane)i));
        DrawRect(p.lanes[i], highlight ? boardColor * 2.0f : boardColor);
        DrawOutline(p.lanes[i], boardColor * 2.5f, 2);

        glm::vec2 markerPos = p.lanes[i].center - glm::vec2(kBoardWidth / 2 - 22, 0);
        DrawLaneMarker((Lane)i, markerPos, boardColor * 3.0f);
    }

    DrawDeck(p.deckPos, (int)p.deck.size());
    DrawLives(p.livesPos, p.hp, p.shield);
    DrawCrystals(p.crystalsPos, p.crystals);
    DrawLeader(p.leader, p.leaderPos, 60, false);

    for (const Card &c : p.row) {
        DrawCard(c, kBoardScale);
    }
    for (size_t i = 0; i < p.hand.size(); i++) {
        if (&p == &player && (int)i == draggedIndex) {
            continue;   // drawn separately, on top
        }
        DrawCard(p.hand[i]);
    }
}


void Tema1::DrawCard(const Card &card, float scale)
{
    // Flip = rotation around the vertical axis, seen from the front: the card gets narrower on X
    // (|cos| goes 1 -> 0 -> 1); the back is visible in the first half, the front in the second
    float angle = card.flip * glm::pi<float>();
    glm::mat3 model = t2d::Translate(card.position.x, card.position.y)
                    * t2d::Scale(scale * glm::abs(glm::cos(angle)), scale);

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

    // Temporary cards: small diamond in the top-left corner
    if (card.IsTemporary()) {
        DrawMesh("diamond", glm::vec2(-22, 38), glm::vec2(13), card.GetSymbolColor(), 0, model);
    }

    // Lane the card belongs to: dots in the top-right corner
    if (card.lane != Lane::Any) {
        DrawLaneMarker(card.lane, glm::vec2(18, 38), card.GetSymbolColor(), model);
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


void Tema1::DrawLaneMarker(Lane lane, glm::vec2 center, glm::vec3 color, const glm::mat3 &parent)
{
    // Melee = 1 dot, Ranged = 2, Siege = 3, centered on `center`
    const float step = 8;
    int count = (int)lane + 1;
    float firstX = center.x - step * (count - 1) / 2;
    for (int i = 0; i < count; i++) {
        DrawMesh("circle", glm::vec2(firstX + i * step, center.y), glm::vec2(6), color, 0, parent);
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


void Tema1::DrawCrystals(glm::vec2 start, int crystals)
{
    // One diamond per crystal: bright while it's still there, gray once lost
    for (int i = 0; i < kCrystals; i++) {
        glm::vec3 color = (i < crystals) ? glm::vec3(0.4f, 0.85f, 0.95f) : glm::vec3(0.18f, 0.2f, 0.22f);
        DrawMesh("diamond", start + glm::vec2(8 + i * 22.0f, 0), glm::vec2(16), color);
    }
}


void Tema1::DrawLeader(const Leader &leader, glm::vec2 center, float size, bool highlight)
{
    // Highlight: the same shape drawn again behind it, in light blue and slightly bigger
    if (highlight) {
        DrawMesh(leader.mesh, center, glm::vec2(size * 1.15f), glm::vec3(0.5f, 0.8f, 1.0f));
    }
    // Gray once the ability was used
    DrawMesh(leader.mesh, center, glm::vec2(size), leader.used ? glm::vec3(0.35f) : leader.color);
}


void Tema1::DrawLeaderMenu()
{
    for (size_t i = 0; i < leaders.size(); i++) {
        Rect slot = LeaderMenuSlot((int)i);
        DrawLeader(leaders[i], slot.center, slot.size.x, (int)i == selectedLeader);
    }
}


void Tema1::DrawResultBanner(int result, glm::vec2 size)
{
    // Banner over the board: green + star = win, red + X = loss, gray + ring = draw
    glm::vec3 color = glm::vec3(0.4f);
    std::string symbol = "ring";
    float angle = 0;

    if (result > 0) {
        color = glm::vec3(0.25f, 0.65f, 0.35f);
        symbol = "star";
    } else if (result < 0) {
        color = glm::vec3(0.80f, 0.22f, 0.22f);
        symbol = "cross";
        angle = glm::quarter_pi<float>();
    }

    Rect banner(glm::vec2(kBoardCenterX, kMiddleY), size);
    DrawRect(banner, color);
    DrawOutline(banner, glm::vec3(0.95f), 4);
    DrawMesh(symbol, banner.center, glm::vec2(size.y * 0.65f), glm::vec3(0.95f), angle);
}


/*
 *  Input
 */


void Tema1::OnKeyPress(int key, int mods)
{
    // New game: back to the leader menu
    if (key == GLFW_KEY_R) {
        ShowLeaderMenu();
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
    if (!IS_BIT_SET(button, GLFW_MOUSE_BUTTON_LEFT)) {
        return;
    }
    mouseWorld = ScreenToWorld(mouseX, mouseY);

    // Leader menu: first click selects, a second click on the selected leader starts the game
    if (phase == Phase::LeaderSelect) {
        for (size_t i = 0; i < leaders.size(); i++) {
            if (LeaderMenuSlot((int)i).Contains(mouseWorld)) {
                if ((int)i == selectedLeader) {
                    NewGame();
                } else {
                    selectedLeader = (int)i;
                }
                return;
            }
        }
        return;
    }

    // Cards can be picked up only during the player's turn
    if (phase != Phase::PlayerTurn) {
        return;
    }

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

    // Valid drop: a Decoy over one of your board cards, any other card on a lane where it is allowed
    bool played = false;
    if (player.hand[draggedIndex].type == CardType::Decoy) {
        int rowIndex = player.RowCardAt(mouseWorld, glm::vec2(kCardWidth, kCardHeight) * kBoardScale);
        if (rowIndex != -1) {
            player.PlayDecoy(draggedIndex, rowIndex);
            played = true;
        }
    } else {
        int lane = player.LaneAt(mouseWorld);
        if (lane != -1 && player.CanPlayOn(player.hand[draggedIndex], (Lane)lane)) {
            player.PlayCard(draggedIndex, (Lane)lane);
            played = true;
        }
    }

    // The enemy answers after a short pause
    if (played) {
        phase = Phase::EnemyTurn;
        timer = kEnemyDelay;
    }

    // Recompute slots: the remaining cards (or the rejected one) slide into place
    draggedIndex = -1;
    LayoutAll();
}
