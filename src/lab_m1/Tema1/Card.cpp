#include "lab_m1/Tema1/Card.h"

using namespace m1;


Card::Card(int type, int value, int lane)
{
    this->type = type;
    this->value = value;
    baseValue = value;
    this->lane = lane;
    placedLane = lane;

    position = glm::vec2(0);
    target = glm::vec2(0);
    faceUp = true;
    flip = 1;
}


bool Card::CanGoOn(int l) const
{
    return lane == LANE_ANY || lane == l;
}


bool Card::IsWeather() const
{
    return type == CARD_FROST_WEATHER || type == CARD_FOG || type == CARD_RAIN;
}


bool Card::IsTemporary() const
{
    return type != CARD_DAMAGE && type != CARD_HEAL && type != CARD_SHIELD;
}


void Card::Animate(float deltaTime)
{
    // Covers a fixed fraction of the remaining distance each frame: fast at first, slows down near the target
    const float moveSpeed = 10;
    position += (target - position) * glm::min(1.0f, moveSpeed * deltaTime);

    // Flip only after arriving, so a placed card is first seen face down, then turned
    const float flipSpeed = 2.5f;
    if (faceUp && flip < 1 && glm::distance(position, target) < 1) {
        flip = glm::min(1.0f, flip + flipSpeed * deltaTime);
    }
}


std::string Card::GetBodyMesh() const
{
    switch (type) {
    case CARD_DAMAGE:           return "card_damage";
    case CARD_HEAL:             return "card_heal";
    case CARD_SHIELD:           return "card_shield";
    case CARD_SPY:              return "card_spy";
    case CARD_FROST_WEATHER:    return "card_frost_weather";
    case CARD_CLEAR_WEATHER:    return "card_clear_weather";
    case CARD_WAR_HORN:         return "card_war_horn";
    case CARD_SCORCH:           return "card_scorch";
    case CARD_FOG:              return "card_fog";
    case CARD_RAIN:             return "card_rain";
    case CARD_MEDIC:            return "card_medic";
    case CARD_DECOY:            return "card_decoy";
    default:                    return "card_border";   // unknown type
    }
}


std::string Card::GetDetailColor() const
{
    // Light cards (Frost, Clear Weather) and the black Spy card need other colors to stay readable
    switch (type) {
    case CARD_FROST_WEATHER:    return "blue";
    case CARD_CLEAR_WEATHER:    return "brown";
    case CARD_SPY:              return "gray";
    default:                    return "cream";
    }
}


std::string Card::GetSymbolShape() const
{
    switch (type) {
    case CARD_DAMAGE:           return "triangle";
    case CARD_HEAL:             return "cross";
    case CARD_SHIELD:           return "circle";
    case CARD_SPY:              return "diamond";
    case CARD_FROST_WEATHER:    return "star";     // all weather cards: star, different colors
    case CARD_FOG:              return "star";
    case CARD_RAIN:             return "star";
    case CARD_MEDIC:            return "cross";
    case CARD_DECOY:            return "square";
    case CARD_CLEAR_WEATHER:    return "ring";
    case CARD_WAR_HORN:         return "triangle";
    case CARD_SCORCH:           return "cross";
    default:                    return "square";
    }
}


float Card::GetSymbolAngle() const
{
    // Same mesh as a permanent card, but rotated so it stays distinct
    switch (type) {
    case CARD_WAR_HORN:     return glm::pi<float>();           // upside-down triangle
    case CARD_SCORCH:       return glm::quarter_pi<float>();   // X
    default:                return 0;
    }
}
