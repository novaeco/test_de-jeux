#pragma once

#include "reptile_types.h"
#include <vector>

// Système d'alimentation ultra-réaliste
class FeedingSystem {
private:
    // Inventaire alimentaire
    struct FoodInventory {
        FoodType type;
        uint16_t quantity;
        uint32_t expiration_date;
        uint8_t quality;             // 0-100 (fraîcheur)
        float nutritional_value;     // Multiplicateur nutritionnel
        bool is_alive;              // Pour insectes vivants
        uint16_t cost_per_unit;     // Coût virtuel
    };
    
    std::vector<FoodInventory> food_storage;
    
    // Planning alimentaire
    struct FeedingSchedule {
        ReptileSpecies species;
        uint8_t frequency_days;      // Tous les X jours
        FoodType preferred_foods[5]; // Rotation alimentaire
        uint16_t portion_size;       // En grammes
        bool supplements_needed;     // Calcium/D3
        uint32_t next_feeding_time;  // Timestamp
    };
    
    std::vector<FeedingSchedule> feeding_schedules;
    
    // Historique alimentaire
    struct FeedingRecord {
        uint32_t timestamp;
        FoodType food_given;
        uint16_t quantity;
        bool supplements_added;
        uint8_t acceptance_rate;     // 0-100%
        bool regurgitated;           // Régurgitation
    };
    
    static const size_t MAX_FEEDING_RECORDS = 100;
    FeedingRecord feeding_history[MAX_FEEDING_RECORDS];
    size_t history_count;
    
public:
    FeedingSystem();
    
    // Gestion inventaire
    bool add_food_to_inventory(FoodType type, uint16_t quantity, uint32_t expiry_days = 30);
    bool remove_food_from_inventory(FoodType type, uint16_t quantity);
    uint16_t get_food_quantity(FoodType type) const;
    bool is_food_fresh(FoodType type) const;
    
    // Planification alimentaire
    void create_feeding_schedule(const Reptile& reptile);
    void update_feeding_schedules(uint32_t current_time);
    bool is_feeding_due(const Reptile& reptile, uint32_t current_time) const;
    uint32_t time_until_next_feeding(const Reptile& reptile, uint32_t current_time) const;
    
    // Alimentation
    enum FeedingResult {
        FEEDING_SUCCESS,
        FEEDING_REFUSED,
        FEEDING_PARTIAL,
        FEEDING_REGURGITATED,
        FEEDING_NO_FOOD,
        FEEDING_INAPPROPRIATE_FOOD
    };
    
    FeedingResult feed_reptile(Reptile& reptile, FoodType food, bool add_supplements = false);
    
    // Calculs nutritionnels avancés
    float calculate_nutritional_needs(const Reptile& reptile) const;
    bool is_diet_balanced(const Reptile& reptile) const;
    std::vector<FoodType> get_recommended_foods(const Reptile& reptile) const;
    
    // Gestion des insectes vivants
    void update_live_food_health();
    bool breed_feeder_insects(FoodType insect_type);
    uint8_t get_cricket_colony_health() const;
    
    // Suppléments
    bool is_calcium_supplement_needed(const Reptile& reptile) const;
    bool is_d3_supplement_needed(const Reptile& reptile) const;
    void apply_supplements(Reptile& reptile, bool calcium, bool d3);
    
    // Analyse comportement alimentaire
    uint8_t get_feeding_response_rate(const Reptile& reptile) const;
    bool shows_hunting_behavior(const Reptile& reptile) const;
    bool has_feeding_problems(const Reptile& reptile) const;
    
    // Coûts et budget
    uint32_t calculate_monthly_feeding_cost(const Reptile& reptile) const;
    uint32_t get_total_food_value() const;
    
    // Événements alimentaires
    void trigger_food_shortage_event();
    void trigger_feeding_frenzy_event();
    void simulate_prey_escape();
    
    // Statistiques
    struct FeedingStats {
        uint32_t total_feedings;
        uint32_t successful_feedings;
        uint32_t refused_feedings;
        float average_acceptance_rate;
        uint32_t total_food_consumed_grams;
        uint32_t supplements_given;
    };
    
    FeedingStats get_feeding_statistics(const Reptile& reptile) const;
    
    // Sauvegarde/chargement
    bool save_feeding_data();
    bool load_feeding_data();
};