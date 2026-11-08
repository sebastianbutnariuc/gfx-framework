#include "lab_m1/Tema1/Card.h"

#include <cstdlib>

using namespace m1;


Card::Card(CardType type, int value)
    : type(type), value(value), position(0), target(0)
{
}


Card Card::Random()
{
    CardType type = (CardType)(rand() % (int)CardType::Count);
    int value = 1 + rand() % 4;
    return Card(type, value);
}


bool Card::IsTemporary() const
{
    return type != CardType::Damage && type != CardType::Heal && type != CardType::Shield;
}


void Card::MoveTowardsTarget(float deltaTime)
{
    // Covers a fixed fraction of the remaining distance each frame: fast at first, slows down near the target
    const float speed = 10;
    position += (target - position) * glm::min(1.0f, speed * deltaTime);
}


glm::vec3 Card::GetColor() const
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


glm::vec3 Card::GetSymbolColor() const
{
    switch (type) {
    case CardType::FrostWeather: return glm::vec3(0.35f, 0.65f, 0.95f);
    case CardType::ClearWeather: return glm::vec3(0.45f, 0.30f, 0.05f);
    case CardType::Spy:          return glm::vec3(0.75f, 0.75f, 0.75f);
    default:                     return glm::vec3(1.0f, 0.95f, 0.9f);
    }
}


std::string Card::GetSymbolMesh() const
{
    switch (type) {
    case CardType::Damage:       return "triangle";
    case CardType::Heal:         return "cross";
    case CardType::Shield:       return "circle";
    case CardType::Spy:          return "diamond";
    case CardType::FrostWeather: return "star";
    case CardType::ClearWeather: return "ring";
    case CardType::WarHorn:      return "triangle";
    case CardType::Scorch:       return "cross";
    default:                     return "square";
    }
}


float Card::GetSymbolAngle() const
{
    // Same mesh as a permanent card, but rotated so it stays distinct
    switch (type) {
    case CardType::WarHorn: return glm::pi<float>();           // upside-down triangle
    case CardType::Scorch:  return glm::quarter_pi<float>();   // X
    default:                return 0;
    }
}
