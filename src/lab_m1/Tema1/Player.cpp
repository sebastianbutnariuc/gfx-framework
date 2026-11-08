#include "lab_m1/Tema1/Player.h"

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


void Player::PlayCard(int handIndex)
{
    Card card = hand[handIndex];
    card.faceUp = true;     // enemy cards flip once they reach the board

    hand.erase(hand.begin() + handIndex);
    row.push_back(card);
    playedIndex = (int)row.size() - 1;
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
