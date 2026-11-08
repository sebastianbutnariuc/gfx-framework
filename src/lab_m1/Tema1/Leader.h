#pragma once

#include <string>
#include <vector>

#include "utils/glm_utils.h"


namespace m1
{
    // One-time ability per game
    enum class LeaderAbility
    {
        ClearWeather,           // Foltest: removes all weather
        RevealHand,             // Emhyr: reveals 3 random cards from the opponent's hand
        DoubleMelee,            // Eredin: doubles your melee cards
        DestroyStrongestMelee,  // Francesca: destroys the opponent's strongest melee card
        HornSiege               // Radovid: doubles your siege cards (War Horn on siege)
    };

    class Leader
    {
     public:
        Leader() : mesh("circle"), color(1), ability(LeaderAbility::ClearWeather), used(false) {}
        Leader(const std::string &mesh, glm::vec3 color, LeaderAbility ability)
            : mesh(mesh), color(color), ability(ability), used(false) {}

        // All the leaders that can be chosen
        static std::vector<Leader> All()
        {
            return
            {
                Leader("circle",   glm::vec3(0.30f, 0.50f, 0.90f), LeaderAbility::ClearWeather),          // Foltest
                Leader("diamond",  glm::vec3(0.90f, 0.75f, 0.25f), LeaderAbility::RevealHand),            // Emhyr
                Leader("triangle", glm::vec3(0.80f, 0.25f, 0.25f), LeaderAbility::DoubleMelee),           // Eredin
                Leader("star",     glm::vec3(0.30f, 0.70f, 0.40f), LeaderAbility::DestroyStrongestMelee), // Francesca
                Leader("hexagon",  glm::vec3(0.60f, 0.35f, 0.80f), LeaderAbility::HornSiege)              // Radovid
            };
        }

     public:
        std::string mesh;       // the leader's shape
        glm::vec3 color;
        LeaderAbility ability;
        bool used;              // ability already used (leader drawn gray)
    };
}   // namespace m1
