#pragma once

#include <stdint.h>
#include <string>

// Types de reptiles disponibles avec données scientifiques précises
enum class ReptileSpecies : uint8_t {
    POGONA_VITTICEPS = 0,      // Dragon barbu
    LEOPARD_GECKO,             // Gecko léopard  
    CORN_SNAKE,                // Serpent des blés
    BALL_PYTHON,               // Python royal
    BLUE_TONGUE_SKINK,         // Scinque à langue bleue
    BEARDED_DRAGON_GERMAN,     // Dragon barbu allemand
    CRESTED_GECKO,             // Gecko à crête
    RED_EARED_SLIDER,          // Tortue de Floride
    HERMANN_TORTOISE,          // Tortue d'Hermann
    IGUANA_IGUANA              // Iguane vert
};

// États physiologiques critiques
enum class LifeStage : uint8_t {
    EGG = 0,
    HATCHLING,
    JUVENILE, 
    SUB_ADULT,
    ADULT,
    SENIOR
};

// Paramètres environnementaux critiques
struct EnvironmentalParams {
    float temperature_day;      // °C - Zone chaude diurne
    float temperature_night;    // °C - Zone fraîche nocturne  
    float humidity;            // % - Hygrométrie
    uint16_t uvb_index;        // Index UVB (0-14)
    uint16_t photoperiod;      // Heures d'éclairage/jour
    bool has_water_dish;       // Présence gamelle d'eau
    bool has_hide_hot;         // Cache côté chaud
    bool has_hide_cool;        // Cache côté froid
};

// Données nutritionnelles précises
enum class FoodType : uint8_t {
    CRICKETS = 0,
    MEALWORMS,
    DUBIA_ROACHES, 
    WAXWORMS,
    FROZEN_MICE_PINKIE,
    FROZEN_MICE_FUZZY,
    FROZEN_MICE_ADULT,
    LEAFY_GREENS,
    VEGETABLES,
    FRUITS,
    CALCIUM_SUPPLEMENT,
    D3_SUPPLEMENT,
    MULTIVITAMIN
};

// État de santé complet
struct HealthStats {
    uint8_t overall_health;     // 0-100
    uint8_t hunger_level;       // 0-100 (100 = affamé)
    uint8_t hydration;         // 0-100
    uint8_t stress_level;      // 0-100
    uint8_t reproductive_condition; // 0-100
    bool is_shedding;          // En mue
    bool has_parasites;        // Parasites présents
    bool respiratory_infection; // Infection respiratoire
    uint32_t last_feeding;     // Timestamp dernière alimentation
    uint32_t last_defecation;  // Timestamp dernière défécation
};

// Données comportementales réalistes
enum class Behavior : uint8_t {
    BASKING = 0,              // Thermorégulation
    HIDING,                   // Cache
    EXPLORING,                // Exploration
    FEEDING,                  // Alimentation
    SLEEPING,                 // Repos
    SHEDDING,                 // Mue
    COURTING,                 // Parade nuptiale
    AGGRESSIVE,               // Agressivité
    STRESSED,                 // Stress
    BRUMATION                 // Hibernation
};

// Structure principale du reptile
struct Reptile {
    ReptileSpecies species;
    char name[32];
    LifeStage life_stage;
    uint16_t age_days;
    uint16_t weight_grams;
    uint16_t length_mm;
    HealthStats health;
    Behavior current_behavior;
    EnvironmentalParams habitat;
    uint32_t birth_timestamp;
    uint32_t last_update;
    bool is_gravid;            // Femelle gravide
    uint8_t genetics_quality;   // Qualité génétique (0-100)
    uint32_t experience_points;
};