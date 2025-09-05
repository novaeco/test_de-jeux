#pragma once

#include "reptile_types.h"

// Base de données scientifique complète des espèces
struct SpeciesData {
    const char* scientific_name;
    const char* common_name_fr;
    const char* common_name_en;
    
    // Paramètres environnementaux optimaux
    struct {
        float temp_day_min, temp_day_max;    // Zone chaude diurne
        float temp_night_min, temp_night_max; // Zone fraîche nocturne
        float humidity_min, humidity_max;     // Hygrométrie
        uint8_t uvb_min, uvb_max;            // Indice UVB requis
        uint16_t photoperiod_summer;         // Heures éclairage été
        uint16_t photoperiod_winter;         // Heures éclairage hiver
        uint32_t terrarium_min_size_L;       // Taille minimale terrarium (L)
        uint32_t terrarium_min_size_l;       // Taille minimale terrarium (l)  
        uint32_t terrarium_min_size_h;       // Taille minimale terrarium (h)
    } environment;
    
    // Données biologiques
    struct {
        uint16_t adult_weight_min_g;         // Poids adulte min (g)
        uint16_t adult_weight_max_g;         // Poids adulte max (g)
        uint16_t adult_length_min_mm;        // Taille adulte min (mm)
        uint16_t adult_length_max_mm;        // Taille adulte max (mm)
        uint16_t lifespan_years;             // Espérance de vie
        uint16_t sexual_maturity_months;     // Maturité sexuelle
        uint8_t clutch_size_min;             // Ponte minimum
        uint8_t clutch_size_max;             // Ponte maximum
        uint16_t incubation_days;            // Incubation (jours)
        float incubation_temp_optimal;       // Température incubation optimale
    } biology;
    
    // Régime alimentaire
    struct {
        bool is_carnivore;
        bool is_herbivore;
        bool is_insectivore;
        uint8_t feeding_frequency_juvenile;   // Fréquence alimentation juvénile (jours)
        uint8_t feeding_frequency_adult;      // Fréquence alimentation adulte (jours)
        FoodType preferred_foods[8];         // Aliments préférés
        bool needs_calcium_supplement;       // Besoin supplémentation calcium
        bool needs_d3_supplement;           // Besoin supplémentation D3
    } diet;
    
    // Difficulté d'élevage (1=facile, 5=expert)
    uint8_t difficulty_level;
    
    // Statut légal en France (CITES, etc.)
    bool requires_cdc;                      // Certificat de capacité requis
    bool requires_authorization;            // Autorisation préfectorale
    const char* cites_appendix;            // Annexe CITES
};

// Base de données complète des espèces
extern const SpeciesData SPECIES_DATABASE[10];

// Fonctions utilitaires
const SpeciesData& get_species_data(ReptileSpecies species);
bool is_temperature_optimal(const Reptile& reptile, float current_temp);
bool is_humidity_optimal(const Reptile& reptile, float current_humidity);
uint8_t calculate_health_impact(const Reptile& reptile, const EnvironmentalParams& conditions);