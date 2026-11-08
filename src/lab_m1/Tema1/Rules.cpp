#include "lab_m1/Tema1/Rules.h"

#include <cstdlib>

using namespace m1;


void Rules::ResolveTurn(Player &player, Player &enemy) const
{
    const CardType order[] =
    {
        CardType::Scorch, CardType::Spy, CardType::ClearWeather, CardType::FrostWeather,
        CardType::WarHorn, CardType::Shield, CardType::Damage, CardType::Heal
    };

    for (CardType type : order) {
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

                ApplyEffect(card, (int)i, owner, opponent);
                if (card.IsTemporary()) {
                    card.value--;
                }
            }
        }
    }

    player.EndTurn();
    enemy.EndTurn();
}


void Rules::ApplyEffect(Card &card, int index, Player &owner, Player &opponent) const
{
    switch (card.type) {
    case CardType::Scorch:
        // Cancels the opponent's card from this turn, if its value is >= V
        if (opponent.playedIndex != -1 && opponent.row[opponent.playedIndex].value >= card.value) {
            opponent.playedCancelled = true;
        }
        break;

    case CardType::Spy:
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

    case CardType::ClearWeather:
        // Removes all active weather (Frost) from both sides
        for (Player *p : { &owner, &opponent }) {
            for (Card &c : p->row) {
                if (c.type == CardType::FrostWeather) {
                    c.value = 0;
                }
            }
        }
        break;

    case CardType::FrostWeather:
        // All the opponent's board cards lose 1
        for (Card &c : opponent.row) {
            c.value--;
        }
        break;

    case CardType::WarHorn:
        // Doubles the next card played by the same player (not the War Horn itself)
        if (owner.playedIndex != -1 && owner.playedIndex != index) {
            owner.row[owner.playedIndex].value *= 2;
        }
        break;

    case CardType::Shield:
        owner.shield = glm::min(maxHP, owner.shield + card.value);
        break;

    case CardType::Damage:
        opponent.TakeDamage(card.value);
        break;

    case CardType::Heal:
        owner.Heal(card.value, maxHP);
        break;

    default:
        break;
    }
}
