#include "game_engine.h"
#include "species_database.h"
#include <iostream>

int main() {
    GameEngine engine;
    engine.add_reptile(ReptileSpecies::POGONA_VITTICEPS, "Alpha");
    engine.add_reptile(ReptileSpecies::LEOPARD_GECKO, "Beta");

    Reptile* r0 = engine.get_reptile(0);
    Reptile* r1 = engine.get_reptile(1);
    if (!r0 || !r1) return 1;

    r0->experience_points = 150;
    r1->experience_points = 50;

    if (engine.get_total_experience() != 200) return 1;
    if (engine.get_keeper_level() != 2) return 1;

    if (!engine.treat_health_issue(0, "soin")) return 1;

    if (!engine.remove_reptile(0)) return 1;
    if (engine.get_reptile_count() != 1) return 1;
    std::cout << "OK" << std::endl;
    return 0;
}
