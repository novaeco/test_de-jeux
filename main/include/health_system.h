#pragma once

#include "reptile_types.h"
#include <vector>

// Système de santé vétérinaire ultra-détaillé
class HealthSystem {
public:
    // Maladies et conditions médicales
    enum HealthCondition : uint8_t {
        HEALTHY = 0,
        RESPIRATORY_INFECTION,      // Infection respiratoire
        METABOLIC_BONE_DISEASE,     // Maladie métabolique osseuse
        PARASITES_INTERNAL,         // Parasites internes
        PARASITES_EXTERNAL,         // Parasites externes (acariens)
        STUCK_SHED,                 // Mue bloquée
        BURNS,                      // Brûlures
        DEHYDRATION,                // Déshydratation
        MALNUTRITION,               // Malnutrition
        EGG_BINDING,                // Rétention d'œufs
        MOUTH_ROT,                  // Stomatite
        SCALE_ROT,                  // Pourriture des écailles
        PROLAPSE,                   // Prolapsus
        STRESS_RELATED_ILLNESS,     // Maladie liée au stress
        GENETIC_DEFECT              // Défaut génétique
    };
    
    // Symptômes observables
    enum Symptom : uint16_t {
        NONE = 0,
        LETHARGY = 1,               // Léthargie
        LOSS_OF_APPETITE = 2,       // Perte d'appétit
        DIFFICULTY_BREATHING = 4,    // Difficultés respiratoires
        MOUTH_BREATHING = 8,         // Respiration par la bouche
        WHEEZING = 16,              // Sifflements respiratoires
        WEIGHT_LOSS = 32,           // Perte de poids
        ABNORMAL_POSTURE = 64,      // Posture anormale
        TREMORS = 128,              // Tremblements
        DISCHARGE_EYES = 256,       // Écoulements oculaires
        DISCHARGE_NOSE = 512,       // Écoulements nasaux
        SWOLLEN_JOINTS = 1024,      // Articulations gonflées
        SOFT_SHELL = 2048,          // Carapace molle (tortues)
        RETAINED_SHED = 4096,       // Mue retenue
        MITES_VISIBLE = 8192,       // Acariens visibles
        DIARRHEA = 16384,           // Diarrhée
        CONSTIPATION = 32768        // Constipation
    };
    
private:
    // Dossier médical complet
    struct MedicalRecord {
        uint32_t reptile_id;
        std::vector<HealthCondition> current_conditions;
        uint16_t current_symptoms;   // Bitmask des symptômes
        float body_condition_score;  // 1-5 (condition corporelle)
        uint16_t weight_history[30]; // 30 derniers pesages
        uint32_t last_vet_visit;
        uint32_t last_health_check;
        
        // Traitements en cours
        struct Treatment {
            HealthCondition condition;
            const char* medication;
            uint8_t dosage;
            uint32_t start_date;
            uint32_t duration_days;
            bool administered_today;
        };
        
        std::vector<Treatment> active_treatments;
    };
    
    std::vector<MedicalRecord> medical_records;
    
    // Base de données des maladies
    struct DiseaseInfo {
        HealthCondition condition;
        const char* name_fr;
        const char* name_scientific;
        uint16_t common_symptoms;    // Bitmask
        uint8_t severity;           // 1-5
        uint8_t contagiousness;     // 0-5
        uint16_t treatment_duration_days;
        float mortality_rate;       // 0.0-1.0
        const char* prevention_tips[3];
        const char* treatment_protocol;
        uint32_t treatment_cost;    // Coût virtuel
    };
    
    static const DiseaseInfo DISEASE_DATABASE[];
    
    // Facteurs de risque environnementaux
    struct RiskFactors {
        float temperature_stress;
        float humidity_stress;
        float nutrition_deficiency;
        float overcrowding_stress;
        float handling_stress;
        float substrate_contamination;
        float seasonal_vulnerability;
    };
    
public:
    HealthSystem();
    
    // Gestion dossiers médicaux
    void create_medical_record(uint32_t reptile_id);
    MedicalRecord* get_medical_record(uint32_t reptile_id);
    
    // Examens de santé
    void perform_health_check(Reptile& reptile);
    void perform_weight_check(Reptile& reptile, uint16_t weight_grams);
    float calculate_body_condition_score(const Reptile& reptile) const;
    
    // Détection des maladies
    std::vector<HealthCondition> diagnose_conditions(const Reptile& reptile) const;
    bool has_symptoms(const Reptile& reptile, Symptom symptom_mask) const;
    void add_symptoms(Reptile& reptile, Symptom new_symptoms);
    void remove_symptoms(Reptile& reptile, Symptom removed_symptoms);
    
    // Progression des maladies
    void update_health_conditions(Reptile& reptile, uint32_t delta_time);
    void progress_disease(Reptile& reptile, HealthCondition condition);
    bool can_recover_naturally(HealthCondition condition) const;
    
    // Traitements vétérinaires
    bool start_treatment(Reptile& reptile, HealthCondition condition);
    bool administer_medication(Reptile& reptile, const char* medication);
    void update_treatments(Reptile& reptile, uint32_t current_time);
    bool is_treatment_complete(const Reptile& reptile, HealthCondition condition) const;
    
    // Prévention
    RiskFactors calculate_risk_factors(const Reptile& reptile) const;
    std::vector<const char*> get_prevention_recommendations(const Reptile& reptile) const;
    void apply_preventive_care(Reptile& reptile);
    
    // Quarantaine
    bool needs_quarantine(const Reptile& reptile) const;
    void place_in_quarantine(Reptile& reptile);
    void release_from_quarantine(Reptile& reptile);
    
    // Urgences médicales
    bool is_emergency_condition(HealthCondition condition) const;
    bool requires_immediate_attention(const Reptile& reptile) const;
    void handle_medical_emergency(Reptile& reptile);
    
    // Analyses et statistiques
    float get_overall_health_score(const Reptile& reptile) const;
    uint32_t get_days_since_last_illness(const Reptile& reptile) const;
    std::vector<HealthCondition> get_health_history(const Reptile& reptile) const;
    
    // Contagion (multi-reptiles)
    void check_disease_transmission(const std::vector<Reptile>& all_reptiles);
    float calculate_transmission_risk(HealthCondition condition) const;
    
    // Coûts vétérinaires
    uint32_t calculate_treatment_cost(HealthCondition condition) const;
    uint32_t get_total_medical_expenses() const;
    
    // Événements de santé aléatoires
    void trigger_health_event(Reptile& reptile);
    void simulate_age_related_decline(Reptile& reptile);
    void simulate_seasonal_health_risks(Reptile& reptile);
    
    // Interface utilisateur santé
    const char* get_condition_description(HealthCondition condition) const;
    const char* get_treatment_instructions(HealthCondition condition) const;
    lv_color_t get_health_status_color(const Reptile& reptile) const;
    
    // Alertes système
    bool has_critical_health_alerts() const;
    std::vector<const char*> get_health_alerts() const;
    
    // Sauvegarde/chargement
    bool save_medical_records();
    bool load_medical_records();
};