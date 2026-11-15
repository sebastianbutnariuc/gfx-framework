#include "lab_m1/Tema1/Tema1.h"

#include <iostream>

#include "lab_m1/Tema1/transform2D.h"
#include "lab_m1/Tema1/object2D.h"

using namespace std;
using namespace m1;

namespace t2d = tema1::transform2D;


/*
 *  Scene coordinates are logical: (0, 0) bottom-left, (1280, 720) top-right.
 *  Depth test is off, so draw order = layering (what is drawn last is on top).
 */


void Tema1::Init()
{
    // Logical scene size (first: the camera below uses it)
    sceneWidth = 1280;
    sceneHeight = 720;

    // Card sizes
    cardWidth = 70;
    cardHeight = 100;
    cardSpacing = 15;
    boardScale = 0.65f;

    // Board
    boardCenterX = 630;
    boardWidth = 760;
    middleY = 360;
    laneHeight = 76;

    // Game settings
    maxHP = 15;
    maxCrystals = 2;
    roundPause = 2.0f;
    deckSize = 20;
    handSize = 10;
    dealInterval = 0.15f;
    enemyDelay = 0.5f;

    // Camera: orthographic over the logical scene
    auto camera = GetSceneCamera();
    camera->SetOrthographic(0, sceneWidth, 0, sceneHeight, 0.01f, 400);
    camera->SetPosition(glm::vec3(0, 0, 50));
    camera->SetRotation(glm::vec3(0, 0, 0));
    camera->Update();
    GetCameraInput()->SetActive(false);

    CreateMeshes();

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
    for (int i = 0; i < LANE_COUNT; i++) {
        float offset = laneHeight * (i + 0.5f);
        glm::vec2 size = glm::vec2(boardWidth, laneHeight);
        player.lanes[i] = Rect(glm::vec2(boardCenterX, middleY - offset), size);
        enemy.lanes[i]  = Rect(glm::vec2(boardCenterX, middleY + offset), size);
    }

    enemy.hiddenHand = true;

    mouseWorld = glm::vec2(0);

    // Leaders in the menu: shape, ability (their colors are in CreateMeshes)
    leaders.push_back(Leader("circle",   ABILITY_CLEAR_WEATHER));            // Foltest
    leaders.push_back(Leader("diamond",  ABILITY_REVEAL_HAND));              // Emhyr
    leaders.push_back(Leader("triangle", ABILITY_DOUBLE_MELEE));             // Eredin
    leaders.push_back(Leader("star",     ABILITY_DESTROY_STRONGEST_MELEE));  // Francesca
    leaders.push_back(Leader("hexagon",  ABILITY_HORN_SIEGE));               // Radovid

    ShowLeaderMenu();
}


void Tema1::CreateMeshes()
{
    // Every mesh has size 1 and is centered in the origin (object2D); its color is stored in its vertices,
    // so every shape + color combination drawn in the scene is a separate mesh

    // Card: border, one body for each card type, value squares
    AddMeshToList(tema1::object2D::CreateSquare("card_border",        glm::vec3(0.9f)));
    AddMeshToList(tema1::object2D::CreateSquare("card_damage",        glm::vec3(0.80f, 0.22f, 0.22f)));
    AddMeshToList(tema1::object2D::CreateSquare("card_heal",          glm::vec3(0.25f, 0.65f, 0.35f)));
    AddMeshToList(tema1::object2D::CreateSquare("card_shield",        glm::vec3(0.22f, 0.42f, 0.80f)));
    AddMeshToList(tema1::object2D::CreateSquare("card_spy",           glm::vec3(0.08f, 0.08f, 0.08f)));
    AddMeshToList(tema1::object2D::CreateSquare("card_frost_weather", glm::vec3(0.92f, 0.94f, 0.97f)));
    AddMeshToList(tema1::object2D::CreateSquare("card_clear_weather", glm::vec3(0.90f, 0.75f, 0.20f)));
    AddMeshToList(tema1::object2D::CreateSquare("card_war_horn",      glm::vec3(0.95f, 0.50f, 0.15f)));
    AddMeshToList(tema1::object2D::CreateSquare("card_scorch",        glm::vec3(0.50f, 0.25f, 0.70f)));
    AddMeshToList(tema1::object2D::CreateSquare("card_fog",           glm::vec3(0.55f, 0.58f, 0.62f)));
    AddMeshToList(tema1::object2D::CreateSquare("card_rain",          glm::vec3(0.20f, 0.55f, 0.58f)));
    AddMeshToList(tema1::object2D::CreateSquare("card_medic",         glm::vec3(0.90f, 0.50f, 0.65f)));
    AddMeshToList(tema1::object2D::CreateSquare("card_decoy",         glm::vec3(0.50f, 0.36f, 0.22f)));
    AddMeshToList(tema1::object2D::CreateSquare("pip",                glm::vec3(0.9f, 0.2f, 0.2f)));
    AddMeshToList(tema1::object2D::CreateSquare("pip_border",         glm::vec3(0.1f)));

    // Symbols: every shape in every symbol color, named "<shape>_<color>" (e.g. "star_blue").
    // Used for the card details (Card::GetDetailColor) and the result banner (white)
    std::string colorNames[] = { "cream", "gray", "blue", "brown", "white" };
    glm::vec3 colors[] =
    {
        glm::vec3(1.0f, 0.95f, 0.9f),       // cream
        glm::vec3(0.75f),                   // gray
        glm::vec3(0.35f, 0.65f, 0.95f),     // blue
        glm::vec3(0.45f, 0.30f, 0.05f),     // brown
        glm::vec3(0.95f)                    // white
    };
    for (int i = 0; i < 5; i++) {
        AddMeshToList(tema1::object2D::CreateSquare("square_" + colorNames[i], colors[i]));
        AddMeshToList(tema1::object2D::CreateTriangle("triangle_" + colorNames[i], colors[i]));
        AddMeshToList(tema1::object2D::CreateDiamond("diamond_" + colorNames[i], colors[i]));
        AddMeshToList(tema1::object2D::CreatePolygon("circle_" + colorNames[i], 40, colors[i]));
        AddMeshToList(tema1::object2D::CreateRing("ring_" + colorNames[i], 0.6f, colors[i]));
        AddMeshToList(tema1::object2D::CreateStar("star_" + colorNames[i], colors[i]));
        AddMeshToList(tema1::object2D::CreateCross("cross_" + colorNames[i], colors[i]));
    }

    // Card back
    AddMeshToList(tema1::object2D::CreateSquare("back_border", glm::vec3(0.35f, 0.5f, 0.8f)));
    AddMeshToList(tema1::object2D::CreateSquare("back_body",   glm::vec3(0.12f, 0.2f, 0.38f)));

    // Board: hand zones, empty deck outline
    AddMeshToList(tema1::object2D::CreateSquare("hand_zone",  glm::vec3(0.09f, 0.11f, 0.17f)));
    AddMeshToList(tema1::object2D::CreateSquare("deck_empty", glm::vec3(0.3f), false));

    // Lanes of each side: normal, highlighted while dragging, outline, lane dots (brighter versions of the same color)
    glm::vec3 playerLane = glm::vec3(0.10f, 0.17f, 0.30f);
    glm::vec3 enemyLane = glm::vec3(0.25f, 0.10f, 0.14f);
    AddMeshToList(tema1::object2D::CreateSquare("lane_player",           playerLane));
    AddMeshToList(tema1::object2D::CreateSquare("lane_player_highlight", playerLane * 2.0f));
    AddMeshToList(tema1::object2D::CreateSquare("lane_player_outline",   playerLane * 2.5f, false));
    AddMeshToList(tema1::object2D::CreatePolygon("lane_player_dot", 40,  playerLane * 3.0f));
    AddMeshToList(tema1::object2D::CreateSquare("lane_enemy",            enemyLane));
    AddMeshToList(tema1::object2D::CreateSquare("lane_enemy_highlight",  enemyLane * 2.0f));
    AddMeshToList(tema1::object2D::CreateSquare("lane_enemy_outline",    enemyLane * 2.5f, false));
    AddMeshToList(tema1::object2D::CreatePolygon("lane_enemy_dot", 40,   enemyLane * 3.0f));

    // Lives, shield, crystals (lost ones in dark gray)
    AddMeshToList(tema1::object2D::CreateSquare("life",          glm::vec3(0.35f, 0.8f, 0.4f)));
    AddMeshToList(tema1::object2D::CreateSquare("life_lost",     glm::vec3(0.18f, 0.2f, 0.22f)));
    AddMeshToList(tema1::object2D::CreateSquare("shield",        glm::vec3(0.3f, 0.55f, 1.0f)));
    AddMeshToList(tema1::object2D::CreateDiamond("crystal",      glm::vec3(0.4f, 0.85f, 0.95f)));
    AddMeshToList(tema1::object2D::CreateDiamond("crystal_lost", glm::vec3(0.18f, 0.2f, 0.22f)));

    // Leaders: own color, gray once the ability was used, light blue highlight in the menu
    glm::vec3 used = glm::vec3(0.35f);
    glm::vec3 highlight = glm::vec3(0.5f, 0.8f, 1.0f);

    AddMeshToList(tema1::object2D::CreatePolygon("leader_circle", 40, glm::vec3(0.30f, 0.50f, 0.90f)));    // Foltest
    AddMeshToList(tema1::object2D::CreatePolygon("leader_circle_used", 40, used));
    AddMeshToList(tema1::object2D::CreatePolygon("leader_circle_highlight", 40, highlight));

    AddMeshToList(tema1::object2D::CreateDiamond("leader_diamond", glm::vec3(0.90f, 0.75f, 0.25f)));       // Emhyr
    AddMeshToList(tema1::object2D::CreateDiamond("leader_diamond_used", used));
    AddMeshToList(tema1::object2D::CreateDiamond("leader_diamond_highlight", highlight));

    AddMeshToList(tema1::object2D::CreateTriangle("leader_triangle", glm::vec3(0.80f, 0.25f, 0.25f)));     // Eredin
    AddMeshToList(tema1::object2D::CreateTriangle("leader_triangle_used", used));
    AddMeshToList(tema1::object2D::CreateTriangle("leader_triangle_highlight", highlight));

    AddMeshToList(tema1::object2D::CreateStar("leader_star", glm::vec3(0.30f, 0.70f, 0.40f)));             // Francesca
    AddMeshToList(tema1::object2D::CreateStar("leader_star_used", used));
    AddMeshToList(tema1::object2D::CreateStar("leader_star_highlight", highlight));

    AddMeshToList(tema1::object2D::CreatePolygon("leader_hexagon", 6, glm::vec3(0.60f, 0.35f, 0.80f)));    // Radovid
    AddMeshToList(tema1::object2D::CreatePolygon("leader_hexagon_used", 6, used));
    AddMeshToList(tema1::object2D::CreatePolygon("leader_hexagon_highlight", 6, highlight));

    // Result banner: green = win, red = loss, gray = draw; white outline (the symbol is white too, see above)
    AddMeshToList(tema1::object2D::CreateSquare("banner_win",     glm::vec3(0.25f, 0.65f, 0.35f)));
    AddMeshToList(tema1::object2D::CreateSquare("banner_lose",    glm::vec3(0.80f, 0.22f, 0.22f)));
    AddMeshToList(tema1::object2D::CreateSquare("banner_draw",    glm::vec3(0.4f)));
    AddMeshToList(tema1::object2D::CreateSquare("banner_outline", glm::vec3(0.95f), false));
}


void Tema1::ShowLeaderMenu()
{
    phase = PHASE_LEADER_SELECT;
    selectedLeader = -1;
    draggedIndex = -1;
}


Rect Tema1::LeaderMenuSlot(int index) const
{
    // Leaders side by side, centered on the screen
    float offset = index - (leaders.size() - 1) / 2.0f;
    return Rect(glm::vec2(sceneWidth / 2 + offset * 200, sceneHeight / 2), glm::vec2(120));
}


void Tema1::NewGame()
{
    // Chosen leader for the player, random one for the enemy
    player.leader = leaders[selectedLeader];
    enemy.leader = leaders[rand() % leaders.size()];

    // Random deck for each player; hands are dealt one card at a time in Update
    for (int side = 0; side < 2; side++) {
        Player &p = (side == 0) ? player : enemy;

        p.deck.clear();
        p.hand.clear();
        p.row.clear();
        p.graveyard.clear();
        p.NewRound(maxHP);
        p.crystals = maxCrystals;
        for (int i = 0; i < deckSize; i++) {
            Card card = RandomCard();
            if (p.hiddenHand) {
                card.faceUp = false;    // enemy cards stay hidden until played
                card.flip = 0;
            }
            p.deck.push_back(card);
        }
    }

    draggedIndex = -1;
    phase = PHASE_DEALING;
    timer = 0;
    cardsToDeal = 2 * handSize;
    roundResult = 0;
}


Card Tema1::RandomCard()
{
    int type = rand() % CARD_TYPE_COUNT;
    int value = 1 + rand() % 4;

    // Permanent cards belong to one random lane, weather cards to their own lane,
    // the other temporary cards can go anywhere
    Card card(type, value, LANE_ANY);
    switch (type) {
    case CARD_FROST_WEATHER:    card.lane = LANE_MELEE;  break;
    case CARD_FOG:              card.lane = LANE_RANGED; break;
    case CARD_RAIN:             card.lane = LANE_SIEGE;  break;
    default:
        if (!card.IsTemporary()) {
            card.lane = rand() % LANE_COUNT;
        }
        break;
    }
    return card;
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
    if (enemy.hand[index].type == CARD_DECOY) {
        enemy.PlayDecoy(index, rand() % (int)enemy.row.size());
    } else {
        int lane = enemy.hand[index].lane;
        if (lane == LANE_ANY) {
            lane = rand() % LANE_COUNT;
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
    if (player.hp > enemy.hp) {
        roundResult = 1;
    } else if (player.hp < enemy.hp) {
        roundResult = -1;
    } else {
        roundResult = 0;
    }
    if (roundResult <= 0) player.crystals--;
    if (roundResult >= 0) enemy.crystals--;

    cout << "Round over: " << (roundResult > 0 ? "you win" : roundResult < 0 ? "you lose" : "draw")
         << " (" << player.hp << " - " << enemy.hp << ")\n";

    phase = PHASE_ROUND_OVER;
    timer = roundPause;
}


void Tema1::StartRound()
{
    // Board cleared, life reset; no new hands, just one more card for each player
    player.NewRound(maxHP);
    enemy.NewRound(maxHP);

    phase = PHASE_DEALING;
    timer = 0;
    cardsToDeal = 2;
}


/*
 *  Card effects
 */


void Tema1::ResolveTurn()
{
    const int order[] =
    {
        CARD_SCORCH, CARD_SPY, CARD_MEDIC, CARD_CLEAR_WEATHER,
        CARD_FROST_WEATHER, CARD_FOG, CARD_RAIN,      // weather, together
        CARD_WAR_HORN, CARD_SHIELD, CARD_DAMAGE, CARD_HEAL
    };

    for (int type : order) {
        // Same type: the player's cards first, then the enemy's
        for (int side = 0; side < 2; side++) {
            Player &owner = (side == 0) ? player : enemy;
            Player &opponent = (side == 0) ? enemy : player;

            for (size_t i = 0; i < owner.row.size(); i++) {
                Card &card = owner.row[i];

                bool cancelled = ((int)i == owner.playedIndex && owner.playedCancelled);
                if (card.type != type || card.value <= 0 || cancelled) {
                    continue;
                }

                // false if the card had nothing to act on this turn (then it keeps its value)
                bool applied = true;

                switch (card.type) {
                case CARD_SCORCH:
                    // Cancels the opponent's card from this turn, if its value is >= V
                    if (opponent.playedIndex != -1 && opponent.row[opponent.playedIndex].value >= card.value) {
                        opponent.playedCancelled = true;
                    }
                    break;

                case CARD_SPY:
                    // Steals a random card from the opponent's hand; it slides to the new hand
                    if (!opponent.hand.empty()) {
                        int stolenIndex = rand() % (int)opponent.hand.size();
                        Card stolen = opponent.hand[stolenIndex];
                        stolen.faceUp = !owner.hiddenHand;
                        stolen.flip = stolen.faceUp ? 1.0f : 0.0f;

                        opponent.hand.erase(opponent.hand.begin() + stolenIndex);
                        owner.hand.push_back(stolen);
                    }
                    break;

                case CARD_MEDIC:
                    owner.ReviveRandom();
                    break;

                case CARD_CLEAR_WEATHER:
                    // Removes all active weather (Frost, Fog, Rain) from both sides
                    owner.ClearWeather();
                    opponent.ClearWeather();
                    break;

                case CARD_FROST_WEATHER:
                case CARD_FOG:
                case CARD_RAIN:
                    // The opponent's cards on the weather's lane (Frost: melee, Fog: ranged, Rain: siege) lose 1
                    for (Card &c : opponent.row) {
                        if (c.placedLane == card.placedLane) {
                            c.value--;
                        }
                    }
                    break;

                case CARD_WAR_HORN:
                    // Doubles the next card played by the same player (not the War Horn itself).
                    // In the turn it is placed (or a turn with no card played) it waits for that card.
                    if (owner.playedIndex != -1 && owner.playedIndex != (int)i) {
                        owner.row[owner.playedIndex].value *= 2;
                    } else {
                        applied = false;
                    }
                    break;

                case CARD_SHIELD:
                    owner.shield = glm::min(maxHP, owner.shield + card.value);
                    break;

                case CARD_DAMAGE:
                    opponent.TakeDamage(card.value);
                    break;

                case CARD_HEAL:
                    owner.Heal(card.value, maxHP);
                    break;

                default:
                    break;
                }

                // Temporary cards lose 1 value after applying their effect
                if (card.IsTemporary() && applied) {
                    card.value--;
                }
            }
        }
    }

    player.EndTurn();
    enemy.EndTurn();
}


void Tema1::UseLeaderAbility(Player &owner, Player &opponent)
{
    if (owner.leader.used) {
        return;
    }
    owner.leader.used = true;

    switch (owner.leader.ability) {
    case ABILITY_CLEAR_WEATHER:
        // Foltest: all weather cards are destroyed
        owner.ClearWeather();
        opponent.ClearWeather();
        break;

    case ABILITY_REVEAL_HAND:
        // Emhyr: up to 3 random hidden cards of the opponent's hand are turned face up
        for (int i = 0; i < 3; i++) {
            std::vector<Card*> hidden;
            for (Card &c : opponent.hand) {
                if (!c.faceUp) {
                    hidden.push_back(&c);
                }
            }
            if (hidden.empty()) {
                break;
            }
            hidden[rand() % hidden.size()]->faceUp = true;
        }
        break;

    case ABILITY_DOUBLE_MELEE:
    case ABILITY_HORN_SIEGE:
    {
        // Eredin: your melee cards x2; Radovid: your siege cards x2
        int lane = (owner.leader.ability == ABILITY_DOUBLE_MELEE) ? LANE_MELEE : LANE_SIEGE;
        for (Card &c : owner.row) {
            if (c.placedLane == lane) {
                c.value *= 2;
            }
        }
        break;
    }

    case ABILITY_DESTROY_STRONGEST_MELEE:
    {
        // Francesca: the opponent's strongest melee card is destroyed
        Card *strongest = nullptr;
        for (Card &c : opponent.row) {
            if (c.placedLane == LANE_MELEE && (!strongest || c.value > strongest->value)) {
                strongest = &c;
            }
        }
        if (strongest) {
            strongest->value = 0;
        }
        break;
    }
    }

    // Destroyed cards go to the graveyard right away
    owner.EndTurn();
    opponent.EndTurn();
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
    if (phase == PHASE_LEADER_SELECT) {
        DrawLeaderMenu();
        return;
    }

    timer -= deltaTimeSeconds;

    switch (phase) {
    case PHASE_LEADER_SELECT:
        break;

    case PHASE_DEALING:
        // One card every dealInterval seconds, alternating player / enemy
        if (timer <= 0) {
            DealCard(cardsToDeal % 2 == 0 ? player : enemy);
            cardsToDeal--;
            timer = dealInterval;
        }
        if (cardsToDeal == 0) {
            phase = PHASE_PLAYER_TURN;
        }
        break;

    case PHASE_ENEMY_TURN:
        if (timer <= 0) {
            EnemyPlayCard();
            phase = PHASE_REVEAL;
        }
        break;

    case PHASE_REVEAL:
        // Wait until the enemy card is fully turned face up, then apply the effects
        if (enemy.playedIndex == -1 || enemy.row[enemy.playedIndex].flip >= 1) {
            ResolveTurn();
            LayoutAll();    // cards stolen by Spy / destroyed cards

            if (IsRoundOver()) {
                EndRound();
            } else {
                // The enemy uses its leader once, at a random moment between turns
                if (!enemy.leader.used && rand() % 4 == 0) {
                    UseLeaderAbility(enemy, player);
                    LayoutAll();
                }
                phase = PHASE_PLAYER_TURN;
            }
        }
        break;

    case PHASE_ROUND_OVER:
        // After the pause: next round, or the end of the game if someone has no crystals left
        if (timer <= 0) {
            if (player.crystals == 0 || enemy.crystals == 0) {
                phase = PHASE_GAME_OVER;
            } else {
                StartRound();
            }
        }
        break;

    case PHASE_PLAYER_TURN:
        // No playable cards left (but the enemy still has some): skip to the enemy's move
        if (!player.HasPlayableCard()) {
            phase = PHASE_ENEMY_TURN;
            timer = enemyDelay;
        }
        break;

    case PHASE_GAME_OVER:
        break;
    }

    player.Animate(deltaTimeSeconds);
    enemy.Animate(deltaTimeSeconds);

    DrawPlayer(enemy);
    DrawPlayer(player);

    // The dragged card is drawn last, on top of everything
    if (draggedIndex != -1) {
        DrawCard(player.hand[draggedIndex]);
    }

    if (phase == PHASE_ROUND_OVER) {
        DrawResultBanner(roundResult, glm::vec2(300, 90));
    }
    if (phase == PHASE_GAME_OVER) {
        int gameResult = 0;
        if (player.crystals > enemy.crystals) {
            gameResult = 1;
        } else if (player.crystals < enemy.crystals) {
            gameResult = -1;
        }
        DrawResultBanner(gameResult, glm::vec2(500, 140));
    }
}


glm::vec2 Tema1::ScreenToWorld(int mouseX, int mouseY) const
{
    // Mouse coordinates are in window points, not framebuffer pixels
    int width, height;
    glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
    return glm::vec2(mouseX * sceneWidth / width, (height - mouseY) * sceneHeight / height);
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
    for (int side = 0; side < 2; side++) {
        Player &p = (side == 0) ? player : enemy;

        // Hand: one row
        std::vector<Card*> hand;
        for (Card &c : p.hand) {
            hand.push_back(&c);
        }
        LayoutRow(hand, p.handZone.center, cardWidth + cardSpacing);

        // Board: one row per lane, with the smaller board cards
        for (int lane = 0; lane < LANE_COUNT; lane++) {
            std::vector<Card*> cards;
            for (Card &c : p.row) {
                if (c.placedLane == lane) {
                    cards.push_back(&c);
                }
            }
            LayoutRow(cards, p.lanes[lane].center, cardWidth * boardScale + 8);
        }
    }
}


/*
 *  Drawing
 */


void Tema1::DrawPlayer(const Player &p)
{
    // Lane meshes of this side: "lane_player..." or "lane_enemy..."
    bool isPlayer = (&p == &player);
    std::string lane = isPlayer ? "lane_player" : "lane_enemy";

    // Hand zone
    modelMatrix = glm::mat3(1);
    modelMatrix *= t2d::Translate(p.handZone.center.x, p.handZone.center.y);
    modelMatrix *= t2d::Scale(p.handZone.size.x, p.handZone.size.y);
    RenderMesh2D(meshes["hand_zone"], shaders["VertexColor"], modelMatrix);

    // Lanes; while dragging, the ones where the card can be dropped are highlighted
    for (int i = 0; i < LANE_COUNT; i++) {
        bool highlight = (isPlayer && draggedIndex != -1 && player.CanPlayOn(player.hand[draggedIndex], i));

        // Filled rectangle + outline: same size, so the same matrix
        modelMatrix = glm::mat3(1);
        modelMatrix *= t2d::Translate(p.lanes[i].center.x, p.lanes[i].center.y);
        modelMatrix *= t2d::Scale(p.lanes[i].size.x, p.lanes[i].size.y);
        RenderMesh2D(meshes[highlight ? lane + "_highlight" : lane], shaders["VertexColor"], modelMatrix);
        RenderMesh2D(meshes[lane + "_outline"], shaders["VertexColor"], modelMatrix);

        glm::vec2 markerPos = p.lanes[i].center - glm::vec2(boardWidth / 2 - 22, 0);
        DrawLaneMarker(i, markerPos, lane + "_dot", glm::mat3(1));
    }

    DrawDeck(p.deckPos, (int)p.deck.size());
    DrawLives(p.livesPos, p.hp, p.shield);
    DrawCrystals(p.crystalsPos, p.crystals);
    DrawLeader(p.leader, p.leaderPos, 60, false);

    for (const Card &c : p.row) {
        DrawCard(c, boardScale);
    }
    for (size_t i = 0; i < p.hand.size(); i++) {
        if (isPlayer && (int)i == draggedIndex) {
            continue;   // drawn separately, on top
        }
        DrawCard(p.hand[i]);
    }
}


void Tema1::DrawCard(const Card &card, float scale)
{
    // Card matrix: moves the card to its position and scales it (board cards are smaller).
    // Flip = rotation around the vertical axis, seen from the front: the card gets narrower on X
    // (|cos| goes 1 -> 0 -> 1); the back is visible in the first half, the front in the second
    float angle = card.flip * glm::pi<float>();
    glm::mat3 cardMatrix = glm::mat3(1);
    cardMatrix *= t2d::Translate(card.position.x, card.position.y);
    cardMatrix *= t2d::Scale(scale * glm::abs(glm::cos(angle)), scale);

    if (card.flip > 0.5f) {
        DrawCardFront(card, cardMatrix);
    } else {
        DrawCardBack(cardMatrix);
    }
}


// The card's parts are placed relative to the card's center (0, 0), then `cardMatrix` places the whole card
void Tema1::DrawCardFront(const Card &card, glm::mat3 cardMatrix)
{
    // Border (slightly bigger, behind) + body
    modelMatrix = cardMatrix;
    modelMatrix *= t2d::Scale(cardWidth + 4, cardHeight + 4);
    RenderMesh2D(meshes["card_border"], shaders["VertexColor"], modelMatrix);

    modelMatrix = cardMatrix;
    modelMatrix *= t2d::Scale(cardWidth, cardHeight);
    RenderMesh2D(meshes[card.GetBodyMesh()], shaders["VertexColor"], modelMatrix);

    // Symbol, temporary diamond and lane dots: meshes "<shape>_<detail color>"
    std::string detail = card.GetDetailColor();

    // Type symbol
    modelMatrix = cardMatrix;
    modelMatrix *= t2d::Translate(0, 8);
    modelMatrix *= t2d::Rotate(card.GetSymbolAngle());
    modelMatrix *= t2d::Scale(35, 35);
    RenderMesh2D(meshes[card.GetSymbolShape() + "_" + detail], shaders["VertexColor"], modelMatrix);

    // Temporary cards: small diamond in the top-left corner
    if (card.IsTemporary()) {
        modelMatrix = cardMatrix;
        modelMatrix *= t2d::Translate(-22, 38);
        modelMatrix *= t2d::Scale(13, 13);
        RenderMesh2D(meshes["diamond_" + detail], shaders["VertexColor"], modelMatrix);
    }

    // Lane the card belongs to: dots in the top-right corner
    if (card.lane != LANE_ANY) {
        DrawLaneMarker(card.lane, glm::vec2(18, 38), "circle_" + detail, cardMatrix);
    }

    // Value V: one small square per unit (with a dark border behind it), centered along the bottom edge
    // (squares get smaller for large values, e.g. after War Horn, to stay inside the card)
    float step = glm::min(11.0f, 60.0f / card.value);
    float pip = 0.75f * step;
    float firstX = -step * (card.value - 1) / 2;
    for (int i = 0; i < card.value; i++) {
        modelMatrix = cardMatrix;
        modelMatrix *= t2d::Translate(firstX + i * step, -38);
        modelMatrix *= t2d::Scale(pip + 2, pip + 2);
        RenderMesh2D(meshes["pip_border"], shaders["VertexColor"], modelMatrix);

        modelMatrix = cardMatrix;
        modelMatrix *= t2d::Translate(firstX + i * step, -38);
        modelMatrix *= t2d::Scale(pip, pip);
        RenderMesh2D(meshes["pip"], shaders["VertexColor"], modelMatrix);
    }
}


// `baseMatrix`: identity for the dots at the start of a lane, the card's matrix for the dots on a card
void Tema1::DrawLaneMarker(int lane, glm::vec2 center, const std::string &mesh, glm::mat3 baseMatrix)
{
    // Melee = 1 dot, Ranged = 2, Siege = 3, centered on `center`
    const float step = 8;
    int count = lane + 1;
    float firstX = center.x - step * (count - 1) / 2;
    for (int i = 0; i < count; i++) {
        modelMatrix = baseMatrix;
        modelMatrix *= t2d::Translate(firstX + i * step, center.y);
        modelMatrix *= t2d::Scale(6, 6);
        RenderMesh2D(meshes[mesh], shaders["VertexColor"], modelMatrix);
    }
}


void Tema1::DrawCardBack(glm::mat3 cardMatrix)
{
    // Border (slightly bigger, behind) + body
    modelMatrix = cardMatrix;
    modelMatrix *= t2d::Scale(cardWidth + 4, cardHeight + 4);
    RenderMesh2D(meshes["back_border"], shaders["VertexColor"], modelMatrix);

    modelMatrix = cardMatrix;
    modelMatrix *= t2d::Scale(cardWidth, cardHeight);
    RenderMesh2D(meshes["back_body"], shaders["VertexColor"], modelMatrix);

    // Pattern: an X across the card = two thin bars along the diagonals, in the border color
    float diagonal = glm::length(glm::vec2(cardWidth, cardHeight)) * 0.85f;
    float angle = glm::atan(cardHeight, cardWidth);

    modelMatrix = cardMatrix;
    modelMatrix *= t2d::Rotate(angle);
    modelMatrix *= t2d::Scale(diagonal, 2);
    RenderMesh2D(meshes["back_border"], shaders["VertexColor"], modelMatrix);

    modelMatrix = cardMatrix;
    modelMatrix *= t2d::Rotate(-angle);
    modelMatrix *= t2d::Scale(diagonal, 2);
    RenderMesh2D(meshes["back_border"], shaders["VertexColor"], modelMatrix);
}


void Tema1::DrawDeck(glm::vec2 center, int cardsLeft)
{
    // Empty deck: only the outline of a card
    if (cardsLeft <= 0) {
        modelMatrix = glm::mat3(1);
        modelMatrix *= t2d::Translate(center.x, center.y);
        modelMatrix *= t2d::Scale(cardWidth, cardHeight);
        RenderMesh2D(meshes["deck_empty"], shaders["VertexColor"], modelMatrix);
        return;
    }

    // Up to 3 stacked backs, the top one at `center`
    for (int i = glm::min(cardsLeft, 3) - 1; i >= 0; i--) {
        glm::mat3 cardMatrix = glm::mat3(1);
        cardMatrix *= t2d::Translate(center.x + 4.0f * i, center.y - 4.0f * i);
        DrawCardBack(cardMatrix);
    }
}


void Tema1::DrawLives(glm::vec2 start, int hp, int shield)
{
    const float cell = 10, step = 13;

    // One square per life unit, lost ones in dark gray
    for (int i = 0; i < maxHP; i++) {
        modelMatrix = glm::mat3(1);
        modelMatrix *= t2d::Translate(start.x + i * step, start.y);
        modelMatrix *= t2d::Scale(cell, cell);
        RenderMesh2D(meshes[(i < hp) ? "life" : "life_lost"], shaders["VertexColor"], modelMatrix);
    }

    // Shield on a separate row, in blue
    for (int i = 0; i < shield; i++) {
        modelMatrix = glm::mat3(1);
        modelMatrix *= t2d::Translate(start.x + i * step, start.y - step);
        modelMatrix *= t2d::Scale(cell, cell);
        RenderMesh2D(meshes["shield"], shaders["VertexColor"], modelMatrix);
    }
}


void Tema1::DrawCrystals(glm::vec2 start, int crystals)
{
    // One diamond per crystal: bright while it's still there, gray once lost
    for (int i = 0; i < maxCrystals; i++) {
        modelMatrix = glm::mat3(1);
        modelMatrix *= t2d::Translate(start.x + 8 + i * 22.0f, start.y);
        modelMatrix *= t2d::Scale(16, 16);
        RenderMesh2D(meshes[(i < crystals) ? "crystal" : "crystal_lost"], shaders["VertexColor"], modelMatrix);
    }
}


void Tema1::DrawLeader(const Leader &leader, glm::vec2 center, float size, bool highlight)
{
    std::string mesh = "leader_" + leader.shape;

    // Highlight: the same shape drawn again behind it, in light blue and slightly bigger
    if (highlight) {
        modelMatrix = glm::mat3(1);
        modelMatrix *= t2d::Translate(center.x, center.y);
        modelMatrix *= t2d::Scale(size * 1.15f, size * 1.15f);
        RenderMesh2D(meshes[mesh + "_highlight"], shaders["VertexColor"], modelMatrix);
    }

    // Gray once the ability was used
    modelMatrix = glm::mat3(1);
    modelMatrix *= t2d::Translate(center.x, center.y);
    modelMatrix *= t2d::Scale(size, size);
    RenderMesh2D(meshes[leader.used ? mesh + "_used" : mesh], shaders["VertexColor"], modelMatrix);
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
    std::string background = "banner_draw";
    std::string symbol = "ring_white";
    float angle = 0;

    if (result > 0) {
        background = "banner_win";
        symbol = "star_white";
    } else if (result < 0) {
        background = "banner_lose";
        symbol = "cross_white";
        angle = glm::quarter_pi<float>();
    }

    // Background + outline: same size, so the same matrix
    modelMatrix = glm::mat3(1);
    modelMatrix *= t2d::Translate(boardCenterX, middleY);
    modelMatrix *= t2d::Scale(size.x, size.y);
    RenderMesh2D(meshes[background], shaders["VertexColor"], modelMatrix);
    RenderMesh2D(meshes["banner_outline"], shaders["VertexColor"], modelMatrix);

    // Symbol in the middle
    modelMatrix = glm::mat3(1);
    modelMatrix *= t2d::Translate(boardCenterX, middleY);
    modelMatrix *= t2d::Rotate(angle);
    modelMatrix *= t2d::Scale(size.y * 0.65f, size.y * 0.65f);
    RenderMesh2D(meshes[symbol], shaders["VertexColor"], modelMatrix);
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
    if (phase == PHASE_LEADER_SELECT) {
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

    // Cards and the leader can be used only during the player's turn
    if (phase != PHASE_PLAYER_TURN) {
        return;
    }

    // Click on your leader: its ability is applied right away (once per game), the turn continues
    if (Rect(player.leaderPos, glm::vec2(60)).Contains(mouseWorld)) {
        UseLeaderAbility(player, enemy);
        LayoutAll();
        return;
    }

    // Select the card under the cursor
    for (size_t i = 0; i < player.hand.size(); i++) {
        if (Rect(player.hand[i].position, glm::vec2(cardWidth, cardHeight)).Contains(mouseWorld)) {
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
    if (player.hand[draggedIndex].type == CARD_DECOY) {
        int rowIndex = player.RowCardAt(mouseWorld, glm::vec2(cardWidth, cardHeight) * boardScale);
        if (rowIndex != -1) {
            player.PlayDecoy(draggedIndex, rowIndex);
            played = true;
        }
    } else {
        int lane = player.LaneAt(mouseWorld);
        if (lane != -1 && player.CanPlayOn(player.hand[draggedIndex], lane)) {
            player.PlayCard(draggedIndex, lane);
            played = true;
        }
    }

    // The enemy answers after a short pause
    if (played) {
        phase = PHASE_ENEMY_TURN;
        timer = enemyDelay;
    }

    // Recompute slots: the remaining cards (or the rejected one) slide into place
    draggedIndex = -1;
    LayoutAll();
}
