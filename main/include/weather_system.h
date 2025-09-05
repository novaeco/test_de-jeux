#pragma once

#include "reptile_types.h"
#include "lvgl.h"

// Système météorologique et saisonnier réaliste
class WeatherSystem {
public:
    enum Season : uint8_t {
        SPRING = 0,
        SUMMER,
        AUTUMN,
        WINTER
    };
    
    enum WeatherPattern : uint8_t {
        CLEAR = 0,
        PARTLY_CLOUDY,
        OVERCAST,
        LIGHT_RAIN,
        HEAVY_RAIN,
        THUNDERSTORM,
        FOG,
        DROUGHT,
        HEATWAVE,
        COLD_SNAP
    };
    
private:
    // État météorologique actuel
    struct CurrentWeather {
        Season current_season;
        WeatherPattern weather_pattern;
        float ambient_temperature;      // Température extérieure
        float humidity_level;           // Humidité extérieure
        uint16_t daylight_hours;        // Heures de jour
        uint8_t cloud_cover;           // Couverture nuageuse (%)
        uint16_t barometric_pressure;   // Pression atmosphérique
        float wind_speed;              // Vitesse du vent
        bool is_stormy_weather;        // Conditions orageuses
    };
    
    CurrentWeather weather_state;
    
    // Cycle saisonnier
    struct SeasonalData {
        uint16_t day_of_year;          // 1-365
        float seasonal_temperature_base;
        float seasonal_humidity_base;
        uint16_t seasonal_daylight;
        bool breeding_season_active;
        bool brumation_period;         // Hibernation
    };
    
    SeasonalData seasonal_info;
    
    // Événements météorologiques
    struct WeatherEvent {
        WeatherPattern type;
        uint32_t start_time;
        uint32_t duration_hours;
        float temperature_modifier;
        float humidity_modifier;
        bool affects_behavior;
        const char* description;
    };
    
    static const size_t MAX_WEATHER_EVENTS = 10;
    WeatherEvent active_weather_events[MAX_WEATHER_EVENTS];
    size_t active_events_count;
    
    // Historique météorologique
    struct WeatherHistory {
        uint32_t timestamp;
        Season season;
        WeatherPattern pattern;
        float temperature;
        float humidity;
    };
    
    static const size_t WEATHER_HISTORY_SIZE = 365; // 1 an
    WeatherHistory weather_history[WEATHER_HISTORY_SIZE];
    size_t history_index;
    
public:
    WeatherSystem();
    
    // Mise à jour du système
    void update_weather_system(uint32_t delta_time);
    void advance_seasonal_cycle();
    void generate_daily_weather();
    
    // Contrôle saisonnier
    void set_season(Season new_season);
    Season get_current_season() const;
    uint16_t get_days_in_season() const;
    float get_seasonal_progression() const; // 0.0-1.0
    
    // Conditions météorologiques
    WeatherPattern get_current_weather() const;
    float get_ambient_temperature() const;
    float get_ambient_humidity() const;
    uint16_t get_daylight_hours() const;
    bool is_breeding_season() const;
    bool is_brumation_period() const;
    
    // Impact environnemental
    float calculate_temperature_influence() const;
    float calculate_humidity_influence() const;
    float calculate_lighting_influence() const;
    bool should_trigger_brumation(const Reptile& reptile) const;
    
    // Événements météorologiques
    void trigger_weather_event(WeatherPattern pattern, uint32_t duration_hours);
    void trigger_seasonal_storm();
    void trigger_heatwave();
    void trigger_cold_snap();
    void trigger_drought_period();
    
    // Impact comportemental
    bool affects_activity_level(const Reptile& reptile) const;
    bool affects_feeding_behavior(const Reptile& reptile) const;
    bool affects_breeding_readiness(const Reptile& reptile) const;
    Behavior get_weather_influenced_behavior(const Reptile& reptile) const;
    
    // Changements saisonniers automatiques
    void apply_spring_changes();
    void apply_summer_changes();
    void apply_autumn_changes();
    void apply_winter_changes();
    
    // Prédictions météorologiques
    WeatherPattern predict_weather_tomorrow() const;
    std::vector<WeatherPattern> get_weekly_forecast() const;
    Season predict_next_season_start() const;
    
    // Impact sur la santé
    bool increases_disease_risk(const Reptile& reptile) const;
    bool affects_respiratory_health(const Reptile& reptile) const;
    bool increases_stress_levels(const Reptile& reptile) const;
    
    // Gestion de l'habitat en fonction de la météo
    struct WeatherAdaptations {
        bool increase_heating;
        bool increase_humidity;
        bool extend_lighting;
        bool improve_ventilation;
        bool provide_extra_hiding_spots;
    };
    
    WeatherAdaptations get_habitat_recommendations(const Reptile& reptile) const;
    
    // Événements spéciaux
    void simulate_natural_disaster();
    void simulate_climate_anomaly();
    void trigger_migration_instinct(Reptile& reptile);
    
    // Interface utilisateur
    const char* get_season_name() const;
    const char* get_weather_description() const;
    lv_color_t get_weather_color() const;
    const char* get_weather_icon() const; // Unicode/emoji
    
    // Graphiques et visualisation
    void draw_temperature_graph(lv_obj_t* chart);
    void draw_humidity_graph(lv_obj_t* chart);
    void draw_seasonal_cycle(lv_obj_t* canvas);
    
    // Alertes météorologiques
    std::vector<const char*> get_weather_alerts() const;
    bool has_severe_weather_warning() const;
    
    // Statistiques climatiques
    struct WeatherStats {
        float average_temperature_year;
        float highest_temperature;
        float lowest_temperature;
        uint32_t total_rainy_days;
        uint32_t total_sunny_days;
        uint32_t storm_events;
    };
    
    WeatherStats get_yearly_weather_stats() const;
    
    // Configuration géographique
    void set_geographic_location(float latitude, float longitude);
    void apply_climate_zone_settings(const char* climate_zone);
    
    // Sauvegarde/chargement
    bool save_weather_data();
    bool load_weather_data();
};