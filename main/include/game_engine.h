#pragma once

#include "reptile_types.h"
#include "lvgl.h"
#include <vector>

class GameEngine {
private:
    std::vector<Reptile> reptiles;
    uint32_t current_timestamp;
    uint8_t selected_reptile_index;
    
    // Systèmes de simulation avancés
    void update_physiology(Reptile& reptile, uint32_t delta_time);
    void update_behavior(Reptile& reptile);
    void update_growth(Reptile& reptile);
    void update_health_systems(Reptile& reptile);
    void simulate_circadian_rhythms(Reptile& reptile);
    void simulate_seasonal_changes(Reptile& reptile);
    void check_breeding_conditions(Reptile& reptile);
    void process_aging(Reptile& reptile);
    
public:
    GameEngine();
    ~GameEngine();
    
    // Gestion des reptiles
    bool add_reptile(ReptileSpecies species, const char* name);
    bool remove_reptile(uint8_t index);
    Reptile* get_reptile(uint8_t index);
    size_t get_reptile_count() const;
    const std::vector<Reptile>& get_reptiles() const;
    void set_reptiles(const std::vector<Reptile>& reptiles);
    
    // Interactions de gameplay
    bool feed_reptile(uint8_t index, FoodType food);
    bool adjust_temperature(uint8_t index, float new_temp);
    bool adjust_humidity(uint8_t index, float new_humidity);
    bool toggle_lighting(uint8_t index);
    bool clean_terrarium(uint8_t index);
    bool handle_reptile(uint8_t index);
    
    // Système de reproduction
    bool can_breed(uint8_t female_index, uint8_t male_index);
    bool initiate_breeding(uint8_t female_index, uint8_t male_index);
    
    // Système de santé vétérinaire
    bool diagnose_health_issue(uint8_t index);
    bool treat_health_issue(uint8_t index, const char* treatment);
    
    // Mise à jour du moteur de jeu
    void update(uint32_t delta_time_ms);
    
    // Événements aléatoires
    void trigger_random_events();
    
    // Sélection active
    void select_reptile(uint8_t index);
    uint8_t get_selected_reptile() const;
    
    // Statistiques
    uint32_t get_total_experience() const;
    uint8_t get_keeper_level() const;
    
    // Sauvegarde/chargement
    bool save_game_state();
    bool load_game_state();
};