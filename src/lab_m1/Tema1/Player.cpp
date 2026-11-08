#include "lab_m1/Tema1/Player.h"

#include <cstdlib>

using namespace m1;


Player::Player()
    : hp(0), shield(0), hiddenHand(false), playedIndex(-1), playedCancelled(false), deckPos(0), livesPos(0)
{
}


void Player::DrawFromDeck()
{
    if (deck.empty()) {
        return;
    }
    Card card = deck.back();
    deck.pop_back();
    card.position = deckPos;
    hand.push_back(card);
}


void Player::PlayCard(int handIndex, Lane lane)
{
    Card card = hand[handIndex];
    card.faceUp = true;     // enemy cards flip once they reach the board
    card.placedLane = lane;

    hand.erase(hand.begin() + handIndex);
    row.push_back(card);
    playedIndex = (int)row.size() - 1;
}


void Player::PlayDecoy(int handIndex, int rowIndex)
{
    Card decoy = hand[handIndex];
    decoy.faceUp = true;
    decoy.placedLane = row[rowIndex].placedLane;

    // The replaced card slides from the board back to the hand
    Card replaced = row[rowIndex];
    replaced.faceUp = !hiddenHand;
    replaced.flip = replaced.faceUp ? 1.0f : 0.0f;

    hand.erase(hand.begin() + handIndex);
    hand.push_back(replaced);
    row[rowIndex] = decoy;
    playedIndex = rowIndex;
}


void Player::ReviveRandom()
{
    if (graveyard.empty()) {
        return;
    }
    int index = rand() % (int)graveyard.size();
    Card card = graveyard[index];
    card.value = card.baseValue;
    card.faceUp = !hiddenHand;
    card.flip = card.faceUp ? 1.0f : 0.0f;

    graveyard.erase(graveyard.begin() + index);
    hand.push_back(card);
}


bool Player::CanPlay(const Card &card) const
{
    return card.type != CardType::Decoy || !row.empty();
}


bool Player::CanPlayOn(const Card &card, Lane lane) const
{
    if (card.type != CardType::Decoy) {
        return card.CanGoOn(lane);
    }
    for (const Card &c : row) {
        if (c.placedLane == lane) {
            return true;
        }
    }
    return false;
}


bool Player::HasPlayableCard() const
{
    for (const Card &c : hand) {
        if (CanPlay(c)) {
            return true;
        }
    }
    return false;
}


int Player::RowCardAt(glm::vec2 p, glm::vec2 cardSize) const
{
    for (size_t i = 0; i < row.size(); i++) {
        if (Rect(row[i].position, cardSize).Contains(p)) {
            return (int)i;
        }
    }
    return -1;
}


int Player::LaneAt(glm::vec2 p) const
{
    for (int i = 0; i < kLaneCount; i++) {
        if (lanes[i].Contains(p)) {
            return i;
        }
    }
    return -1;
}


void Player::TakeDamage(int amount)
{
    int absorbed = glm::min(shield, amount);
    shield -= absorbed;
    hp = glm::max(0, hp - (amount - absorbed));
}


void Player::Heal(int amount, int maxHP)
{
    hp = glm::min(maxHP, hp + amount);
}


void Player::EndTurn()
{
    for (size_t i = 0; i < row.size(); ) {
        if (row[i].value <= 0) {
            graveyard.push_back(row[i]);
            row.erase(row.begin() + i);
        } else {
            i++;
        }
    }
    playedIndex = -1;
    playedCancelled = false;
}


void Player::Animate(float deltaTime)
{
    for (Card &c : hand) c.Animate(deltaTime);
    for (Card &c : row)  c.Animate(deltaTime);
}
