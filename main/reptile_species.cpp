#include "include/species_database.h"
#include <cstring>

// Base de données scientifique ultra-précise basée sur les dernières recherches
const SpeciesData SPECIES_DATABASE[10] = {
    // Pogona vitticeps - Dragon barbu central
    {
        .scientific_name = "Pogona vitticeps",
        .common_name_fr = "Dragon barbu central", 
        .common_name_en = "Central Bearded Dragon",
        .environment = {
            .temp_day_min = 35.0f, .temp_day_max = 42.0f,
            .temp_night_min = 20.0f, .temp_night_max = 25.0f,
            .humidity_min = 30.0f, .humidity_max = 40.0f,
            .uvb_min = 10, .uvb_max = 12,
            .photoperiod_summer = 14, .photoperiod_winter = 10,
            .terrarium_min_size_L = 120, .terrarium_min_size_l = 60, .terrarium_min_size_h = 60
        },
        .biology = {
            .adult_weight_min_g = 300, .adult_weight_max_g = 600,
            .adult_length_min_mm = 400, .adult_length_max_mm = 600,
            .lifespan_years = 12, .sexual_maturity_months = 12,
            .clutch_size_min = 15, .clutch_size_max = 30,
            .incubation_days = 65, .incubation_temp_optimal = 29.0f
        },
        .diet = {
            .is_carnivore = true, .is_herbivore = true, .is_insectivore = true,
            .feeding_frequency_juvenile = 1, .feeding_frequency_adult = 2,
            .preferred_foods = {CRICKETS, DUBIA_ROACHES, LEAFY_GREENS, VEGETABLES, FRUITS},
            .needs_calcium_supplement = true, .needs_d3_supplement = true
        },
        .difficulty_level = 2,
        .requires_cdc = false, .requires_authorization = false, .cites_appendix = "Non listée"
    },
    
    // Eublepharis macularius - Gecko léopard  
    {
        .scientific_name = "Eublepharis macularius",
        .common_name_fr = "Gecko léopard",
        .common_name_en = "Leopard Gecko", 
        .environment = {
            .temp_day_min = 28.0f, .temp_day_max = 32.0f,
            .temp_night_min = 20.0f, .temp_night_max = 24.0f,
            .humidity_min = 30.0f, .humidity_max = 40.0f,
            .uvb_min = 2, .uvb_max = 5,
            .photoperiod_summer = 12, .photoperiod_winter = 10,
            .terrarium_min_size_L = 80, .terrarium_min_size_l = 40, .terrarium_min_size_h = 40
        },
        .biology = {
            .adult_weight_min_g = 60, .adult_weight_max_g = 110,
            .adult_length_min_mm = 180, .adult_length_max_mm = 250,
            .lifespan_years = 20, .sexual_maturity_months = 10,
            .clutch_size_min = 2, .clutch_size_max = 2,
            .incubation_days = 52, .incubation_temp_optimal = 27.5f
        },
        .diet = {
            .is_carnivore = false, .is_herbivore = false, .is_insectivore = true,
            .feeding_frequency_juvenile = 1, .feeding_frequency_adult = 3,
            .preferred_foods = {CRICKETS, MEALWORMS, DUBIA_ROACHES, WAXWORMS},
            .needs_calcium_supplement = true, .needs_d3_supplement = false
        },
        .difficulty_level = 1,
        .requires_cdc = false, .requires_authorization = false, .cites_appendix = "Non listée"
    },
    
    // Python regius - Python royal
    {
        .scientific_name = "Python regius",
        .common_name_fr = "Python royal",
        .common_name_en = "Ball Python",
        .environment = {
            .temp_day_min = 28.0f, .temp_day_max = 32.0f,
            .temp_night_min = 24.0f, .temp_night_max = 27.0f,
            .humidity_min = 50.0f, .humidity_max = 65.0f,
            .uvb_min = 0, .uvb_max = 2,
            .photoperiod_summer = 12, .photoperiod_winter = 8,
            .terrarium_min_size_L = 120, .terrarium_min_size_l = 60, .terrarium_min_size_h = 60
        },
        .biology = {
            .adult_weight_min_g = 1000, .adult_weight_max_g = 2500,
            .adult_length_min_mm = 900, .adult_length_max_mm = 1500,
            .lifespan_years = 30, .sexual_maturity_months = 36,
            .clutch_size_min = 4, .clutch_size_max = 10,
            .incubation_days = 55, .incubation_temp_optimal = 31.5f
        },
        .diet = {
            .is_carnivore = true, .is_herbivore = false, .is_insectivore = false,
            .feeding_frequency_juvenile = 7, .feeding_frequency_adult = 21,
            .preferred_foods = {FROZEN_MICE_PINKIE, FROZEN_MICE_FUZZY, FROZEN_MICE_ADULT},
            .needs_calcium_supplement = false, .needs_d3_supplement = false
        },
        .difficulty_level = 2,
        .requires_cdc = true, .requires_authorization = true, .cites_appendix = "Annexe II"
    }
    // Ajout des autres espèces...
};

const SpeciesData& get_species_data(ReptileSpecies species) {
    return SPECIES_DATABASE[static_cast<uint8_t>(species)];
}

bool is_temperature_optimal(const Reptile& reptile, float current_temp) {
    const SpeciesData& data = get_species_data(reptile.species);
    return (current_temp >= data.environment.temp_day_min && 
            current_temp <= data.environment.temp_day_max);
}

bool is_humidity_optimal(const Reptile& reptile, float current_humidity) {
    const SpeciesData& data = get_species_data(reptile.species);
    return (current_humidity >= data.environment.humidity_min && 
            current_humidity <= data.environment.humidity_max);
}

uint8_t calculate_health_impact(const Reptile& reptile, const EnvironmentalParams& conditions) {
    const SpeciesData& data = get_species_data(reptile.species);
    uint8_t impact = 100;
    
    // Impact température
    if (conditions.temperature_day < data.environment.temp_day_min || 
        conditions.temperature_day > data.environment.temp_day_max) {
        impact -= 20;
    }
    
    // Impact humidité  
    if (conditions.humidity < data.environment.humidity_min ||
        conditions.humidity > data.environment.humidity_max) {
        impact -= 15;
    }
    
    // Impact UVB
    if (conditions.uvb_index < data.environment.uvb_min ||
        conditions.uvb_index > data.environment.uvb_max) {
        impact -= 10;
    }
    
    return impact;
}