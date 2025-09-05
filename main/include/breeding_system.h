#pragma once

#include "reptile_types.h"
#include <vector>

// Système de reproduction ultra-réaliste
class BreedingSystem {
public:
    // Phases de reproduction
    enum BreedingPhase : uint8_t {
        NOT_BREEDING = 0,
        PRE_BREEDING_CONDITIONING,  // Conditionnement pré-reproduction
        COURTSHIP,                  // Parade nuptiale
        MATING,                     // Accouplement
        GESTATION,                  // Gestation/incubation
        EGG_LAYING,                 // Ponte
        INCUBATION,                 // Incubation des œufs
        HATCHING,                   // Éclosion
        POST_BREEDING_RECOVERY      // Récupération post-reproduction
    };
    
    // Compatibilité génétique
    enum GeneticCompatibility : uint8_t {
        INCOMPATIBLE = 0,           // Pas de reproduction possible
        POOR_MATCH,                 // Mauvaise compatibilité
        ACCEPTABLE_MATCH,           // Compatibilité acceptable
        GOOD_MATCH,                 // Bonne compatibilité
        EXCELLENT_MATCH             // Compatibilité excellente
    };
    
private:
    // Données de reproduction
    struct BreedingRecord {
        uint32_t female_id;
        uint32_t male_id;
        BreedingPhase current_phase;
        uint32_t breeding_start_date;
        uint32_t mating_date;
        uint32_t expected_laying_date;
        uint32_t laying_date;
        uint8_t eggs_laid;
        uint8_t fertile_eggs;
        uint8_t hatched_eggs;
        float incubation_temperature;
        uint8_t incubation_humidity;
        uint32_t incubation_duration;
        GeneticCompatibility compatibility;
        bool breeding_successful;
        std::vector<uint32_t> offspring_ids;
    };
    
    std::vector<BreedingRecord> breeding_history;
    std::vector<BreedingRecord> active_breeding_projects;
    
    // Conditions environnementales pour déclencher la reproduction
    struct BreedingTriggers {
        bool temperature_cycling;    // Cyclage thermique
        bool humidity_changes;       // Variations d'humidité
        bool photoperiod_changes;    // Changements photopériode
        bool nutritional_conditioning; // Conditionnement nutritionnel
        bool separation_period;      // Période de séparation
        uint8_t conditioning_weeks;  // Durée conditionnement
    };
    
    // Génétique et hérédité
    struct GeneticTraits {
        uint8_t size_gene;          // Gène de taille
        uint8_t color_gene_primary; // Gène couleur primaire
        uint8_t color_gene_secondary; // Gène couleur secondaire
        uint8_t pattern_gene;       // Gène de motifs
        uint8_t health_gene;        // Gène de résistance santé
        uint8_t temperament_gene;   // Gène de tempérament
        uint8_t fertility_gene;     // Gène de fertilité
        bool recessive_traits[8];   // Traits récessifs
    };
    
    // Incubateur virtuel
    struct IncubatorData {
        uint32_t eggs_in_incubation;
        float target_temperature;
        float actual_temperature;
        uint8_t target_humidity;
        uint8_t actual_humidity;
        bool temperature_stable;
        bool humidity_stable;
        uint32_t days_remaining;
        uint8_t estimated_hatch_rate;
    };
    
    IncubatorData incubator_status;
    
public:
    BreedingSystem();
    
    // Évaluation de la compatibilité
    GeneticCompatibility evaluate_breeding_compatibility(const Reptile& female, const Reptile& male) const;
    bool can_breed(const Reptile& female, const Reptile& male) const;
    float calculate_breeding_success_probability(const Reptile& female, const Reptile& male) const;
    
    // Initiation de la reproduction
    bool start_breeding_project(Reptile& female, Reptile& male);
    void apply_breeding_conditioning(Reptile& reptile, uint8_t weeks);
    bool trigger_breeding_behavior(Reptile& female, Reptile& male);
    
    // Gestion des phases de reproduction
    void update_breeding_phases(uint32_t delta_time);
    void progress_to_next_phase(BreedingRecord& record);
    bool is_ready_for_mating(const Reptile& reptile) const;
    
    // Accouplement et fécondation
    bool attempt_mating(Reptile& female, Reptile& male);
    void simulate_courtship_behavior(const Reptile& male, const Reptile& female);
    float calculate_fertilization_rate(const Reptile& female, const Reptile& male) const;
    
    // Gestation et ponte
    void update_gestation(Reptile& female, uint32_t delta_time);
    bool is_ready_for_laying(const Reptile& female) const;
    uint8_t simulate_egg_laying(Reptile& female);
    void create_laying_site(const Reptile& female);
    
    // Incubation des œufs
    void place_eggs_in_incubator(uint8_t egg_count, float temp, uint8_t humidity);
    void update_incubation_process(uint32_t delta_time);
    bool adjust_incubation_temperature(float new_temperature);
    bool adjust_incubation_humidity(uint8_t new_humidity);
    void monitor_egg_development();
    
    // Éclosion
    std::vector<uint32_t> simulate_hatching();
    Reptile create_offspring(const Reptile& mother, const Reptile& father);
    GeneticTraits combine_genetics(const Reptile& parent1, const Reptile& parent2);
    void apply_genetic_traits(Reptile& offspring, const GeneticTraits& traits);
    
    // Soin des nouveau-nés
    void setup_nursery_conditions(ReptileSpecies species);
    bool requires_special_care(const Reptile& hatchling) const;
    void monitor_hatchling_development(Reptile& hatchling);
    
    // Analyse génétique
    float calculate_genetic_diversity() const;
    bool detect_inbreeding_risk(const Reptile& female, const Reptile& male) const;
    std::vector<const char*> get_genetic_recommendations() const;
    
    // Sélection et amélioration génétique
    std::vector<const Reptile*> select_best_breeding_candidates(ReptileSpecies species) const;
    Reptile* find_ideal_breeding_partner(const Reptile& target) const;
    void track_lineage(const Reptile& reptile);
    
    // Problèmes de reproduction
    void handle_breeding_complications(BreedingRecord& record);
    bool treat_egg_binding(Reptile& female);
    void handle_infertile_eggs(BreedingRecord& record);
    void manage_difficult_hatching();
    
    // Conditionnement saisonnier
    void apply_seasonal_breeding_triggers(Reptile& reptile);
    void simulate_brumation_cycle(Reptile& reptile);
    void manage_breeding_season();
    
    // Données et statistiques
    const IncubatorData& get_incubator_status() const;
    BreedingRecord* get_active_breeding_project(uint32_t female_id);
    std::vector<BreedingRecord> get_breeding_history() const;
    
    // Calculs économiques
    uint32_t calculate_breeding_investment_cost(const Reptile& female, const Reptile& male) const;
    uint32_t estimate_offspring_value(const Reptile& parent1, const Reptile& parent2) const;
    
    // Événements de reproduction
    void trigger_breeding_event();
    void simulate_breeding_season_start();
    void handle_unexpected_pregnancy(Reptile& female);
    
    // Interface utilisateur
    const char* get_breeding_phase_description(BreedingPhase phase) const;
    lv_color_t get_breeding_status_color(BreedingPhase phase) const;
    std::vector<const char*> get_breeding_recommendations(const Reptile& reptile) const;
    
    // Alertes
    std::vector<const char*> get_breeding_alerts() const;
    bool requires_immediate_attention() const;
    
    // Sauvegarde/chargement
    bool save_breeding_data();
    bool load_breeding_data();
};