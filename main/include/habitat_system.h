#pragma once

#include "reptile_types.h"
#include "lvgl.h"

// Système de gestion avancée de l'habitat
class HabitatSystem {
private:
    // Capteurs virtuels de l'habitat
    struct HabitatSensors {
        float temperature_basking;    // Zone chaude
        float temperature_cool;       // Zone fraîche
        float humidity_ambient;       // Humidité ambiante
        float humidity_hide;          // Humidité dans les caches
        uint16_t uvb_level;          // Niveau UVB actuel
        uint16_t uva_level;          // Niveau UVA actuel
        bool day_cycle;              // Cycle jour/nuit
        uint8_t substrate_moisture;   // Humidité du substrat
        float air_circulation;       // Circulation d'air
    };
    
    // Équipements de l'habitat
    struct HabitatEquipment {
        bool heating_pad;            // Tapis chauffant
        bool ceramic_heater;         // Chauffage céramique
        bool uvb_lamp;              // Lampe UVB
        bool day_light;             // Éclairage jour
        bool misting_system;        // System de brumisation
        bool thermostat;            // Thermostat
        uint8_t ventilation_level;  // Niveau ventilation
        bool water_heater;          // Chauffage eau
    };
    
    HabitatSensors current_conditions;
    HabitatEquipment equipment_state;
    
    // Gestion automatisée
    bool auto_temperature_control;
    bool auto_humidity_control;
    bool auto_lighting_cycle;
    
    // Historique environnemental
    struct EnvironmentHistory {
        uint32_t timestamp;
        float temperature;
        float humidity;
        uint16_t light_level;
    };
    
    static const size_t HISTORY_SIZE = 288; // 24h à 5min d'intervalle
    EnvironmentHistory environment_history[HISTORY_SIZE];
    size_t history_index;
    
public:
    HabitatSystem();
    
    // Contrôle manuel des équipements
    bool set_heating(bool enabled, uint8_t intensity = 100);
    bool set_lighting(bool uvb_on, bool day_light_on);
    bool set_humidity_system(bool enabled, uint8_t level = 50);
    bool adjust_ventilation(uint8_t level);
    
    // Contrôle automatisé
    void enable_auto_temperature(bool enabled);
    void enable_auto_humidity(bool enabled);
    void enable_auto_lighting(bool enabled);
    
    // Mise à jour du système
    void update_habitat_conditions(uint32_t delta_time);
    void apply_species_requirements(const Reptile& reptile);
    
    // Lecture des conditions
    const HabitatSensors& get_current_conditions() const;
    const HabitatEquipment& get_equipment_state() const;
    
    // Analyse environnementale
    float calculate_temperature_gradient() const;
    bool is_day_night_cycle_proper() const;
    uint8_t get_habitat_quality_score() const;
    
    // Alertes
    bool has_temperature_alert() const;
    bool has_humidity_alert() const;
    bool has_equipment_failure() const;
    
    // Historique et statistiques
    void record_conditions();
    float get_average_temperature_24h() const;
    float get_average_humidity_24h() const;
    
    // Simulation d'événements
    void simulate_equipment_aging();
    void simulate_seasonal_changes();
    void trigger_weather_event(const char* weather_type);
};