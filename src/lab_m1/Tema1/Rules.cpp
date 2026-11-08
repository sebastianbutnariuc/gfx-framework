#include "lab_m1/Tema1/Rules.h"

#include <cstdlib>
#include <vector>

using namespace m1;


void Rules::ResolveTurn(Player &player, Player &enemy) const
{
    const CardType order[] =
    {
        CardType::Scorch, CardType::Spy, CardType::Medic, CardType::ClearWeather,
        CardType::FrostWeather, CardType::Fog, CardType::Rain,      // weather, together
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


void Rules::UseLeaderAbility(Player &owner, Player &opponent) const
{
    if (owner.leader.used) {
        return;
    }
    owner.leader.used = true;

    switch (owner.leader.ability) {
    case LeaderAbility::ClearWeather:
        // Foltest: all weather cards are destroyed
        for (Player *p : { &owner, &opponent }) {
            for (Card &c : p->row) {
                if (c.IsWeather()) {
                    c.value = 0;
                }
            }
        }
        break;

    case LeaderAbility::RevealHand:
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

    case LeaderAbility::DoubleMelee:
    case LeaderAbility::HornSiege:
    {
        // Eredin: your melee cards x2; Radovid: your siege cards x2
        Lane lane = (owner.leader.ability == LeaderAbility::DoubleMelee) ? Lane::Melee : Lane::Siege;
        for (Card &c : owner.row) {
            if (c.placedLane == lane) {
                c.value *= 2;
            }
        }
        break;
    }

    case LeaderAbility::DestroyStrongestMelee:
    {
        // Francesca: the opponent's strongest melee card is destroyed
        Card *strongest = nullptr;
        for (Card &c : opponent.row) {
            if (c.placedLane == Lane::Melee && (!strongest || c.value > strongest->value)) {
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

    case CardType::Medic:
        owner.ReviveRandom();
        break;

    case CardType::ClearWeather:
        // Removes all active weather (Frost, Fog, Rain) from both sides
        for (Player *p : { &owner, &opponent }) {
            for (Card &c : p->row) {
                if (c.IsWeather()) {
                    c.value = 0;
                }
            }
        }
        break;

    case CardType::FrostWeather:
    case CardType::Fog:
    case CardType::Rain:
        // The opponent's cards on the weather's lane (Frost: melee, Fog: ranged, Rain: siege) lose 1
        for (Card &c : opponent.row) {
            if (c.placedLane == card.placedLane) {
                c.value--;
            }
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
