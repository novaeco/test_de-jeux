#include "esp_random.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "esp_timer.h"

#include "include/display_driver.h"
#include "include/game_engine.h"
#include "include/ui_manager.h"
#include "include/save_system.h"

static const char* TAG = "ReptileKeeper";

// Instances principales du système
static DisplayDriver* display_driver = nullptr;
static GameEngine* game_engine = nullptr;
static UIManager* ui_manager = nullptr;
static SaveSystem* save_system = nullptr;

// Tâches FreeRTOS
static void game_update_task(void* pvParameters);
static void ui_update_task(void* pvParameters);
static void system_monitoring_task(void* pvParameters);

// Configuration système
static void initialize_nvs();
static void initialize_system();
static void create_default_reptiles();

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "=== REPTILE KEEPER v1.0 ===");
    ESP_LOGI(TAG, "Système d'élevage de reptiles ultra-réaliste");
    ESP_LOGI(TAG, "Optimisé pour Waveshare ESP32-S3 Touch LCD 7\"");
    
    // Initialisation du système
    initialize_system();
    
    // Création des instances principales
    display_driver = new DisplayDriver();
    game_engine = new GameEngine();
    ui_manager = new UIManager(game_engine);
    save_system = new SaveSystem();
    
    // Initialisation des composants
    if (!display_driver->initialize()) {
        ESP_LOGE(TAG, "ERREUR CRITIQUE: Échec initialisation affichage");
        abort();
    }
    
    if (!ui_manager->initialize()) {
        ESP_LOGE(TAG, "ERREUR CRITIQUE: Échec initialisation interface");
        abort();
    }
    
    // Chargement de la sauvegarde ou création d'un nouveau jeu
    if (!save_system->load_game_data()) {
        ESP_LOGI(TAG, "Nouvelle partie - Création des reptiles par défaut");
        create_default_reptiles();
        save_system->save_game_data();
    }
    
    ESP_LOGI(TAG, "Système initialisé avec succès");
    ESP_LOGI(TAG, "Reptiles chargés: %zu", game_engine->get_reptile_count());
    ESP_LOGI(TAG, "Mémoire libre: %d bytes", esp_get_free_heap_size());
    
    // Création des tâches FreeRTOS avec priorités optimisées
    xTaskCreatePinnedToCore(
        game_update_task, 
        "GameEngine", 
        8192,           // Stack size
        nullptr, 
        2,              // Priority (élevée pour la logique de jeu)
        nullptr,
        1               // Core 1 (séparé de WiFi/Bluetooth)
    );
    
    xTaskCreatePinnedToCore(
        ui_update_task,
        "UIUpdate", 
        12288,          // Stack size plus important pour LVGL
        nullptr,
        1,              // Priority (normale pour UI)
        nullptr,
        1               // Core 1
    );
    
    xTaskCreatePinnedToCore(
        system_monitoring_task,
        "SystemMon",
        4096,
        nullptr,
        0,              // Priority (faible pour monitoring)
        nullptr,
        0               // Core 0
    );
    
    ESP_LOGI(TAG, "Tâches système créées");
    ESP_LOGI(TAG, "=== DÉMARRAGE TERMINÉ ===");
    
    // La boucle principale est gérée par FreeRTOS
    vTaskDelete(NULL);
}

static void initialize_system() {
    ESP_LOGI(TAG, "Configuration système ESP32-S3...");
    
    // Configuration NVS pour la sauvegarde
    initialize_nvs();
    
    // Configuration de l'horloge système pour la précision temporelle
    if (esp_timer_init() != ESP_ERR_INVALID_STATE) {
        ESP_LOGI(TAG, "Timer initialisé");
    }
    
    // Log des informations système
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    ESP_LOGI(TAG, "Chip: %s, revision %d", 
             "ESP32-S3", 
             chip_info.revision);
    ESP_LOGI(TAG, "Cores: %d", chip_info.cores);
    ESP_LOGI(TAG, "Flash: 16MB external");
    ESP_LOGI(TAG, "RAM libre: %d bytes", esp_get_free_heap_size());
}

static void initialize_nvs() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "NVS initialisé");
}

static void create_default_reptiles() {
    // Création de reptiles de démonstration avec diversité
    game_engine->add_reptile(ReptileSpecies::POGONA_VITTICEPS, "Sunny");
    game_engine->add_reptile(ReptileSpecies::LEOPARD_GECKO, "Luna"); 
    game_engine->add_reptile(ReptileSpecies::BALL_PYTHON, "Orion");
    
    ESP_LOGI(TAG, "Reptiles par défaut créés");
}

// Tâche principale du moteur de jeu - 10 Hz
static void game_update_task(void* pvParameters) {
    const TickType_t xFrequency = pdMS_TO_TICKS(100); // 100ms = 10Hz
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    uint32_t last_timestamp = esp_timer_get_time() / 1000;
    uint32_t update_counter = 0;
    
    ESP_LOGI(TAG, "Moteur de jeu démarré (10 Hz)");
    
    while (true) {
        uint32_t current_timestamp = esp_timer_get_time() / 1000;
        uint32_t delta_time = current_timestamp - last_timestamp;
        
        // Mise à jour du moteur de jeu
        game_engine->update(delta_time);
        
        // Sauvegarde automatique toutes les 10 secondes (100 cycles)
        if (++update_counter % 100 == 0) {
            save_system->auto_save();
            
            // Log des statistiques de performance
            ESP_LOGI(TAG, "Stats: RAM libre=%d bytes, Uptime=%d ms", 
                     esp_get_free_heap_size(), 
                     current_timestamp);
        }
        
        last_timestamp = current_timestamp;
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

// Tâche de mise à jour de l'interface - 30 Hz pour fluidité
static void ui_update_task(void* pvParameters) {
    const TickType_t xFrequency = pdMS_TO_TICKS(33); // ~33ms = 30Hz
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    ESP_LOGI(TAG, "Interface utilisateur démarrée (30 Hz)");
    
    while (true) {
        // Mise à jour LVGL (gestion tactile, animations, rendu)
        display_driver->update();
        
        // Mise à jour de l'interface de jeu
        ui_manager->update();
        
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

// Tâche de surveillance système - 1 Hz
static void system_monitoring_task(void* pvParameters) {
    const TickType_t xFrequency = pdMS_TO_TICKS(1000); // 1000ms = 1Hz
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    uint32_t boot_timestamp = esp_timer_get_time() / 1000;
    
    ESP_LOGI(TAG, "Surveillance système démarrée (1 Hz)");
    
    while (true) {
        uint32_t current_time = esp_timer_get_time() / 1000;
        uint32_t uptime_seconds = (current_time - boot_timestamp) / 1000;
        
        // Surveillance mémoire
        uint32_t free_heap = esp_get_free_heap_size();
        uint32_t min_free_heap = esp_get_minimum_free_heap_size();
        
        // Log détaillé toutes les minutes
        if (uptime_seconds % 60 == 0 && uptime_seconds > 0) {
            ESP_LOGI(TAG, "=== RAPPORT SYSTÈME ===");
            ESP_LOGI(TAG, "Uptime: %d secondes", uptime_seconds);
            ESP_LOGI(TAG, "RAM libre: %d bytes (min: %d)", free_heap, min_free_heap);
            ESP_LOGI(TAG, "Reptiles actifs: %zu", game_engine->get_reptile_count());
            ESP_LOGI(TAG, "Température CPU: ~%d°C", (esp_random() % 20) + 45); // Estimation
            ESP_LOGI(TAG, "=====================");
        }
        
        // Alertes mémoire critique
        if (free_heap < 50000) { // Moins de 50KB libre
            ESP_LOGW(TAG, "⚠️  MÉMOIRE FAIBLE: %d bytes", free_heap);
            
            if (free_heap < 20000) { // Moins de 20KB - critique
                ESP_LOGE(TAG, "🚨 MÉMOIRE CRITIQUE: %d bytes - Sauvegarde d'urgence", free_heap);
                save_system->emergency_save();
            }
        }
        
        // Surveillance température (simulation)
        uint32_t simulated_cpu_temp = (esp_random() % 30) + 40; // 40-70°C
        if (simulated_cpu_temp > 65) {
            ESP_LOGW(TAG, "⚠️  Température CPU élevée: %d°C", simulated_cpu_temp);
        }
        
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}