#include "include/game_engine.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "include/species_database.h"
#include <algorithm>
#include <cmath>
#include <cstring>

static const char *TAG = "GameEngine";

GameEngine::GameEngine() : selected_reptile_index(0) {
  current_timestamp = esp_timer_get_time() / 1000; // Convertir en millisecondes
  ESP_LOGI(TAG, "Moteur de jeu initialisé");
}

GameEngine::~GameEngine() { save_game_state(); }

bool GameEngine::add_reptile(ReptileSpecies species, const char *name) {
  if (reptiles.size() >= 10) { // Limite de 10 reptiles
    ESP_LOGW(TAG, "Limite de reptiles atteinte");
    return false;
  }

  Reptile new_reptile = {};
  new_reptile.species = species;
  strncpy(new_reptile.name, name, sizeof(new_reptile.name) - 1);
  new_reptile.life_stage = LifeStage::HATCHLING;
  new_reptile.age_days = 0;
  new_reptile.birth_timestamp = current_timestamp;
  new_reptile.last_update = current_timestamp;

  // Initialiser les stats de santé
  new_reptile.health.overall_health = 100;
  new_reptile.health.hunger_level = 50;
  new_reptile.health.hydration = 80;
  new_reptile.health.stress_level = 20;
  new_reptile.health.reproductive_condition = 0;
  new_reptile.health.is_shedding = false;
  new_reptile.health.has_parasites = false;
  new_reptile.health.respiratory_infection = false;
  new_reptile.health.last_feeding = current_timestamp;
  new_reptile.health.last_defecation = current_timestamp;

  // Initialiser habitat avec paramètres par défaut
  const SpeciesData &data = get_species_data(species);
  new_reptile.habitat.temperature_day =
      (data.environment.temp_day_min + data.environment.temp_day_max) / 2.0f;
  new_reptile.habitat.temperature_night =
      (data.environment.temp_night_min + data.environment.temp_night_max) /
      2.0f;
  new_reptile.habitat.humidity =
      (data.environment.humidity_min + data.environment.humidity_max) / 2.0f;
  new_reptile.habitat.uvb_index =
      (data.environment.uvb_min + data.environment.uvb_max) / 2;
  new_reptile.habitat.photoperiod = data.environment.photoperiod_summer;
  new_reptile.habitat.has_water_dish = true;
  new_reptile.habitat.has_hide_hot = true;
  new_reptile.habitat.has_hide_cool = true;

  // Poids et taille initiaux basés sur l'espèce
  new_reptile.weight_grams =
      data.biology.adult_weight_min_g / 10; // 10% du poids adulte
  new_reptile.length_mm =
      data.biology.adult_length_min_mm / 3; // 1/3 de la taille adulte

  new_reptile.current_behavior = Behavior::EXPLORING;
  new_reptile.is_gravid = false;
  new_reptile.genetics_quality = 70 + (esp_random() % 30); // 70-99%
  new_reptile.experience_points = 0;

  reptiles.push_back(new_reptile);
  const char *sci = data.scientific_name ? data.scientific_name : "Inconnu";
  ESP_LOGI(TAG, "Nouveau reptile ajouté: %s (%s)",
           name ? name : "Sans nom", sci);
  return true;
}

void GameEngine::update_physiology(Reptile &reptile, uint32_t delta_time) {
  const SpeciesData &data = get_species_data(reptile.species);
  uint32_t time_since_feeding = current_timestamp - reptile.health.last_feeding;
  uint32_t feeding_interval = data.diet.feeding_frequency_adult * 24 * 60 * 60 *
                              1000; // En millisecondes

  // Mise à jour de la faim basée sur le métabolisme de l'espèce
  if (time_since_feeding > feeding_interval) {
    reptile.health.hunger_level =
        std::min<uint8_t>(100, reptile.health.hunger_level +
                                   static_cast<uint8_t>(delta_time / 3600000));
  }

  // Déshydratation graduelle
  reptile.health.hydration = std::max<uint8_t>(
      0, reptile.health.hydration - static_cast<uint8_t>(delta_time / 7200000));

  // Impact environnemental sur la santé
  uint8_t env_quality = calculate_health_impact(reptile, reptile.habitat);
  if (env_quality < 80) {
    reptile.health.stress_level =
        std::min(100, reptile.health.stress_level + 1);
  } else {
    reptile.health.stress_level = std::max(0, reptile.health.stress_level - 1);
  }

  // Calcul santé globale
  reptile.health.overall_health =
      (100 - reptile.health.hunger_level / 2 + reptile.health.hydration -
       reptile.health.stress_level) /
      2;
  reptile.health.overall_health =
      std::clamp<uint8_t>(reptile.health.overall_health, 0, 100);
}

void GameEngine::update_behavior(Reptile &reptile) {
  // Comportement basé sur l'heure circadienne et les besoins
  uint32_t hour_of_day = (current_timestamp / 3600000) % 24;

  if (reptile.health.hunger_level > 70) {
    reptile.current_behavior = Behavior::FEEDING;
  } else if (reptile.health.is_shedding) {
    reptile.current_behavior = Behavior::SHEDDING;
  } else if (hour_of_day >= 8 && hour_of_day <= 18) {
    // Jour - comportement diurne
    if (reptile.health.hydration < 30) {
      reptile.current_behavior = Behavior::HIDING; // Recherche d'humidité
    } else if (reptile.habitat.temperature_day <
               get_species_data(reptile.species).environment.temp_day_min + 2) {
      reptile.current_behavior = Behavior::BASKING;
    } else {
      reptile.current_behavior = Behavior::EXPLORING;
    }
  } else {
    // Nuit - repos
    reptile.current_behavior = Behavior::SLEEPING;
  }

  // Simulation de mue cyclique
  if (reptile.age_days % 45 == 0 &&
      reptile.age_days > 0) { // Mue tous les 45 jours
    reptile.health.is_shedding = true;
  }
  if (reptile.health.is_shedding &&
      esp_random() % 100 < 10) { // 10% chance de finir la mue
    reptile.health.is_shedding = false;
  }
}

void GameEngine::update_growth(Reptile &reptile) {
  const SpeciesData &data = get_species_data(reptile.species);

  // Croissance basée sur l'âge et la santé
  if (reptile.health.overall_health > 70) {
    float growth_rate = 1.0f - (float)reptile.age_days /
                                   (data.biology.sexual_maturity_months * 30);
    growth_rate = std::max(0.1f, growth_rate); // Croissance minimale

    reptile.weight_grams += (uint16_t)(growth_rate * 0.5f);
    reptile.length_mm += (uint16_t)(growth_rate * 0.2f);

    // Limiter à la taille adulte
    reptile.weight_grams =
        std::min(reptile.weight_grams, data.biology.adult_weight_max_g);
    reptile.length_mm =
        std::min(reptile.length_mm, data.biology.adult_length_max_mm);
  }

  // Mise à jour stade de vie
  if (reptile.age_days < 30) {
    reptile.life_stage = LifeStage::HATCHLING;
  } else if (reptile.age_days < 180) {
    reptile.life_stage = LifeStage::JUVENILE;
  } else if (reptile.age_days < data.biology.sexual_maturity_months * 30) {
    reptile.life_stage = LifeStage::SUB_ADULT;
  } else if (reptile.age_days < data.biology.lifespan_years * 365 * 0.8f) {
    reptile.life_stage = LifeStage::ADULT;
  } else {
    reptile.life_stage = LifeStage::SENIOR;
  }
}

bool GameEngine::feed_reptile(uint8_t index, FoodType food) {
  if (index >= reptiles.size())
    return false;

  Reptile &reptile = reptiles[index];
  const SpeciesData &data = get_species_data(reptile.species);

  // Vérifier si la nourriture est appropriée
  bool food_appropriate = false;
  for (int i = 0; i < 8; i++) {
    if (data.diet.preferred_foods[i] == food) {
      food_appropriate = true;
      break;
    }
  }

  if (food_appropriate) {
    reptile.health.hunger_level = std::max(0, reptile.health.hunger_level - 30);
    reptile.health.last_feeding = current_timestamp;

    // Bonus santé pour alimentation appropriée
    reptile.health.overall_health =
        std::min(100, reptile.health.overall_health + 5);
    ESP_LOGI(TAG, "%s nourri avec succès", reptile.name);
    return true;
  } else {
    ESP_LOGW(TAG, "Nourriture inappropriée pour %s", reptile.name);
    return false;
  }
}

bool GameEngine::adjust_temperature(uint8_t index, float new_temp) {
  if (index >= reptiles.size())
    return false;

  reptiles[index].habitat.temperature_day = new_temp;
  ESP_LOGI(TAG, "Température ajustée pour %s", reptiles[index].name);
  return true;
}

bool GameEngine::adjust_humidity(uint8_t index, float new_humidity) {
  if (index >= reptiles.size())
    return false;

  reptiles[index].habitat.humidity = new_humidity;
  ESP_LOGI(TAG, "Humidité ajustée pour %s", reptiles[index].name);
  return true;
}

bool GameEngine::toggle_lighting(uint8_t index) {
  if (index >= reptiles.size())
    return false;

  Reptile &reptile = reptiles[index];
  reptile.habitat.photoperiod =
      reptile.habitat.photoperiod
          ? 0
          : get_species_data(reptile.species).environment.photoperiod_summer;
  ESP_LOGI(TAG, "Éclairage %s pour %s",
           reptile.habitat.photoperiod ? "activé" : "désactivé", reptile.name);
  return true;
}

bool GameEngine::clean_terrarium(uint8_t index) {
  if (index >= reptiles.size())
    return false;

  Reptile &reptile = reptiles[index];
  reptile.health.stress_level =
      std::max<int>(0, static_cast<int>(reptile.health.stress_level) - 10);
  ESP_LOGI(TAG, "Terrarium nettoyé pour %s", reptile.name);
  return true;
}

bool GameEngine::diagnose_health_issue(uint8_t index) {
  if (index >= reptiles.size())
    return false;

  Reptile &reptile = reptiles[index];

  if (reptile.health.hunger_level > 80) {
    ESP_LOGW(TAG, "%s présente un niveau de faim élevé", reptile.name);
  }
  if (reptile.health.hydration < 30) {
    ESP_LOGW(TAG, "%s est potentiellement déshydraté", reptile.name);
  }
  if (reptile.health.stress_level > 70) {
    ESP_LOGW(TAG, "%s montre des signes de stress", reptile.name);
  }
  if (reptile.health.has_parasites) {
    ESP_LOGW(TAG, "%s pourrait avoir des parasites", reptile.name);
  }
  if (reptile.health.respiratory_infection) {
    ESP_LOGW(TAG, "%s pourrait avoir une infection respiratoire", reptile.name);
  }

  ESP_LOGI(TAG, "Diagnostic santé complété pour %s", reptile.name);
  return true;
}

void GameEngine::update(uint32_t delta_time_ms) {
  current_timestamp += delta_time_ms;

  for (auto &reptile : reptiles) {
    uint32_t time_diff = current_timestamp - reptile.last_update;

    // Mise à jour de l'âge
    reptile.age_days =
        (current_timestamp - reptile.birth_timestamp) / (24 * 60 * 60 * 1000);

    // Mises à jour physiologiques
    update_physiology(reptile, time_diff);
    update_behavior(reptile);
    update_growth(reptile);

    reptile.last_update = current_timestamp;
  }

  // Événements aléatoires occasionnels
  if (esp_random() % 10000 < 5) { // 0.05% de chance
    trigger_random_events();
  }
}

void GameEngine::trigger_random_events() {
  if (reptiles.empty())
    return;

  uint8_t random_reptile = esp_random() % reptiles.size();
  Reptile &reptile = reptiles[random_reptile];

  uint32_t event_type = esp_random() % 100;

  if (event_type < 10) { // 10% - Stress environnemental
    reptile.health.stress_level =
        std::min(100, reptile.health.stress_level + 20);
    ESP_LOGI(TAG, "Événement: %s est stressé", reptile.name);
  } else if (event_type < 15) { // 5% - Amélioration génétique
    reptile.genetics_quality = std::min(100, reptile.genetics_quality + 5);
    ESP_LOGI(TAG, "Événement: %s développe une meilleure constitution",
             reptile.name);
  } else if (event_type < 18) { // 3% - Parasites
    reptile.health.has_parasites = true;
    ESP_LOGW(TAG, "Événement: %s a des parasites", reptile.name);
  }
}

size_t GameEngine::get_reptile_count() const { return reptiles.size(); }

const std::vector<Reptile>& GameEngine::get_reptiles() const { return reptiles; }

void GameEngine::set_reptiles(const std::vector<Reptile>& new_reptiles) {
  reptiles = new_reptiles;
}

Reptile *GameEngine::get_reptile(uint8_t index) {
  if (index >= reptiles.size())
    return nullptr;
  return &reptiles[index];
}

void GameEngine::select_reptile(uint8_t index) {
  if (index < reptiles.size()) {
    selected_reptile_index = index;
  }
}

uint8_t GameEngine::get_selected_reptile() const {
  return selected_reptile_index;
}

bool GameEngine::remove_reptile(uint8_t index) {
  if (index >= reptiles.size()) {
    return false;
  }

  reptiles.erase(reptiles.begin() + index);
  if (selected_reptile_index >= reptiles.size()) {
    selected_reptile_index = reptiles.empty() ? 0 : reptiles.size() - 1;
  }
  return true;
}

bool GameEngine::handle_reptile(uint8_t index) {
  if (index >= reptiles.size()) {
    return false;
  }

  Reptile &reptile = reptiles[index];
  if (reptile.health.stress_level > 0) {
    reptile.health.stress_level -= 1;
  }
  reptile.experience_points += 1;
  return true;
}

bool GameEngine::can_breed(uint8_t female_index, uint8_t male_index) {
  if (female_index >= reptiles.size() || male_index >= reptiles.size()) {
    return false;
  }
  return false; // Système de reproduction non implémenté
}

bool GameEngine::initiate_breeding(uint8_t female_index, uint8_t male_index) {
  (void)female_index;
  (void)male_index;
  return false; // Fonctionnalité non disponible
}

bool GameEngine::treat_health_issue(uint8_t index, const char *treatment) {
  if (index >= reptiles.size() || treatment == nullptr) {
    return false;
  }

  Reptile &reptile = reptiles[index];
  reptile.health.overall_health =
      std::min<uint8_t>(100, reptile.health.overall_health + 5);
  reptile.experience_points += 2;
  return true;
}

uint32_t GameEngine::get_total_experience() const {
  uint32_t total = 0;
  for (const auto &r : reptiles) {
    total += r.experience_points;
  }
  return total;
}

uint8_t GameEngine::get_keeper_level() const {
  return static_cast<uint8_t>(get_total_experience() / 100);
}

bool GameEngine::save_game_state() {
  return true;
}

bool GameEngine::load_game_state() {
  return true;
}
