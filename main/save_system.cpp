#include "include/save_system.h"
#include "include/game_engine.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <cstring>
#include <algorithm>

static const char* TAG = "SaveSystem";

// Constantes de sauvegarde
const char* SaveSystem::NVS_NAMESPACE = "reptile_game";
const char* SaveSystem::KEY_REPTILE_COUNT = "reptile_cnt";
const char* SaveSystem::KEY_REPTILE_DATA = "reptile_data";
const char* SaveSystem::KEY_GAME_SETTINGS = "game_cfg";
const char* SaveSystem::KEY_SAVE_VERSION = "save_ver";
const char* SaveSystem::KEY_LAST_SAVE_TIME = "last_save";

#define CURRENT_SAVE_VERSION 1
#define MAX_REPTILES 20
#define SAVE_DATA_MAGIC 0xREPTILE

SaveSystem::SaveSystem() : is_initialized(false), save_version(CURRENT_SAVE_VERSION) {
}

SaveSystem::~SaveSystem() {
    if (is_initialized) {
        nvs_close(nvs_handle);
    }
}

bool SaveSystem::initialize() {
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Erreur ouverture NVS: %s", esp_err_to_name(err));
        return false;
    }
    
    is_initialized = true;
    ESP_LOGI(TAG, "Système de sauvegarde initialisé");
    
    // Vérifier la version de sauvegarde
    uint32_t stored_version = 0;
    size_t required_size = sizeof(stored_version);
    err = nvs_get_blob(nvs_handle, KEY_SAVE_VERSION, &stored_version, &required_size);
    
    if (err == ESP_OK) {
        if (stored_version != CURRENT_SAVE_VERSION) {
            ESP_LOGW(TAG, "Version de sauvegarde différente: %d (actuelle: %d)", 
                     stored_version, CURRENT_SAVE_VERSION);
            // Migration si nécessaire
        }
    } else {
        // Première utilisation
        ESP_LOGI(TAG, "Première initialisation du système de sauvegarde");
    }
    
    return true;
}

bool SaveSystem::save_game_data() {
    if (!is_initialized) {
        ESP_LOGE(TAG, "Système non initialisé");
        return false;
    }
    
    uint32_t start_time = esp_timer_get_time() / 1000;
    ESP_LOGI(TAG, "Début sauvegarde...");
    
    // Sauvegarder la version
    esp_err_t err = nvs_set_blob(nvs_handle, KEY_SAVE_VERSION, &save_version, sizeof(save_version));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Erreur sauvegarde version: %s", esp_err_to_name(err));
        return false;
    }
    
    // Sauvegarder le timestamp
    uint32_t current_time = esp_timer_get_time() / 1000;
    err = nvs_set_blob(nvs_handle, KEY_LAST_SAVE_TIME, &current_time, sizeof(current_time));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Erreur sauvegarde timestamp: %s", esp_err_to_name(err));
        return false;
    }
    
    // Valider les modifications
    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Erreur commit NVS: %s", esp_err_to_name(err));
        return false;
    }
    
    uint32_t end_time = esp_timer_get_time() / 1000;
    ESP_LOGI(TAG, "Sauvegarde terminée en %d ms", end_time - start_time);
    
    return true;
}

bool SaveSystem::load_game_data() {
    if (!is_initialized) {
        ESP_LOGE(TAG, "Système non initialisé");
        return false;
    }
    
    ESP_LOGI(TAG, "Chargement des données...");
    
    // Vérifier l'existence des données
    if (!has_save_data()) {
        ESP_LOGI(TAG, "Aucune sauvegarde trouvée");
        return false;
    }
    
    // Charger la version
    uint32_t stored_version;
    size_t required_size = sizeof(stored_version);
    esp_err_t err = nvs_get_blob(nvs_handle, KEY_SAVE_VERSION, &stored_version, &required_size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Erreur lecture version: %s", esp_err_to_name(err));
        return false;
    }
    
    if (stored_version > CURRENT_SAVE_VERSION) {
        ESP_LOGE(TAG, "Version de sauvegarde trop récente: %d", stored_version);
        return false;
    }
    
    ESP_LOGI(TAG, "Données chargées avec succès (version %d)", stored_version);
    return true;
}

bool SaveSystem::save_reptiles(const std::vector<Reptile>& reptiles) {
    if (!is_initialized) return false;
    
    if (reptiles.size() > MAX_REPTILES) {
        ESP_LOGE(TAG, "Trop de reptiles à sauvegarder: %zu", reptiles.size());
        return false;
    }
    
    ESP_LOGI(TAG, "Sauvegarde de %zu reptiles", reptiles.size());
    
    // Sauvegarder le nombre de reptiles
    uint16_t reptile_count = reptiles.size();
    esp_err_t err = nvs_set_blob(nvs_handle, KEY_REPTILE_COUNT, &reptile_count, sizeof(reptile_count));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Erreur sauvegarde nombre reptiles: %s", esp_err_to_name(err));
        return false;
    }
    
    // Sauvegarder les données des reptiles
    if (!reptiles.empty()) {
        // Créer un buffer compressé
        uint8_t compressed_data[4096]; // 4KB max
        size_t compressed_size = compress_reptile_data(reptiles, compressed_data, sizeof(compressed_data));
        
        if (compressed_size == 0) {
            ESP_LOGE(TAG, "Erreur compression données reptiles");
            return false;
        }
        
        // Calculer le checksum
        uint32_t checksum = calculate_checksum(compressed_data, compressed_size);
        
        // Créer l'en-tête de sauvegarde
        SaveHeader header = {
            .version = CURRENT_SAVE_VERSION,
            .timestamp = (uint32_t)(esp_timer_get_time() / 1000),
            .reptile_count = reptile_count,
            .checksum = checksum
        };
        
        // Buffer final avec en-tête + données
        size_t total_size = sizeof(SaveHeader) + compressed_size;
        uint8_t* save_buffer = new uint8_t[total_size];
        
        memcpy(save_buffer, &header, sizeof(SaveHeader));
        memcpy(save_buffer + sizeof(SaveHeader), compressed_data, compressed_size);
        
        // Sauvegarder dans NVS
        err = nvs_set_blob(nvs_handle, KEY_REPTILE_DATA, save_buffer, total_size);
        
        delete[] save_buffer;
        
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Erreur sauvegarde données: %s", esp_err_to_name(err));
            return false;
        }
        
        ESP_LOGI(TAG, "Données reptiles sauvegardées: %zu bytes (compressées: %zu)", 
                 total_size, compressed_size);
    }
    
    return true;
}

bool SaveSystem::load_reptiles(std::vector<Reptile>& reptiles) {
    if (!is_initialized) return false;
    
    // Charger le nombre de reptiles
    uint16_t reptile_count = 0;
    size_t required_size = sizeof(reptile_count);
    esp_err_t err = nvs_get_blob(nvs_handle, KEY_REPTILE_COUNT, &reptile_count, &required_size);
    
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGI(TAG, "Aucun reptile sauvegardé");
        return true; // Pas d'erreur, juste aucune donnée
    }
    
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Erreur lecture nombre reptiles: %s", esp_err_to_name(err));
        return false;
    }
    
    if (reptile_count == 0) {
        ESP_LOGI(TAG, "Aucun reptile à charger");
        return true;
    }
    
    ESP_LOGI(TAG, "Chargement de %d reptiles", reptile_count);
    
    // Déterminer la taille des données
    required_size = 0;
    err = nvs_get_blob(nvs_handle, KEY_REPTILE_DATA, nullptr, &required_size);
    if (err != ESP_OK || required_size == 0) {
        ESP_LOGE(TAG, "Erreur lecture taille données: %s", esp_err_to_name(err));
        return false;
    }
    
    // Allouer et charger les données
    uint8_t* save_buffer = new uint8_t[required_size];
    err = nvs_get_blob(nvs_handle, KEY_REPTILE_DATA, save_buffer, &required_size);
    
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Erreur lecture données: %s", esp_err_to_name(err));
        delete[] save_buffer;
        return false;
    }
    
    // Vérifier l'en-tête
    if (required_size < sizeof(SaveHeader)) {
        ESP_LOGE(TAG, "Données corrompues - taille insuffisante");
        delete[] save_buffer;
        return false;
    }
    
    SaveHeader* header = reinterpret_cast<SaveHeader*>(save_buffer);
    
    if (header->version > CURRENT_SAVE_VERSION) {
        ESP_LOGE(TAG, "Version sauvegarde non supportée: %d", header->version);
        delete[] save_buffer;
        return false;
    }
    
    if (header->reptile_count != reptile_count) {
        ESP_LOGW(TAG, "Incohérence nombre reptiles: %d vs %d", 
                 header->reptile_count, reptile_count);
    }
    
    // Vérifier l'intégrité des données
    uint8_t* compressed_data = save_buffer + sizeof(SaveHeader);
    size_t compressed_size = required_size - sizeof(SaveHeader);
    
    if (!verify_data_integrity(compressed_data, compressed_size, header->checksum)) {
        ESP_LOGE(TAG, "Données corrompues - checksum invalide");
        delete[] save_buffer;
        return false;
    }
    
    // Décompresser les données
    reptiles.clear();
    bool success = decompress_reptile_data(compressed_data, compressed_size, reptiles);
    
    delete[] save_buffer;
    
    if (!success) {
        ESP_LOGE(TAG, "Erreur décompression données reptiles");
        return false;
    }
    
    ESP_LOGI(TAG, "Reptiles chargés avec succès: %zu", reptiles.size());
    return true;
}

size_t SaveSystem::compress_reptile_data(const std::vector<Reptile>& reptiles, uint8_t* buffer, size_t buffer_size) {
    // Compression simple - copie directe avec optimisations
    size_t total_size = reptiles.size() * sizeof(Reptile);
    
    if (total_size > buffer_size) {
        ESP_LOGE(TAG, "Buffer trop petit pour compression: %zu > %zu", total_size, buffer_size);
        return 0;
    }
    
    // Copier les données (compression plus sophistiquée possible ici)
    memcpy(buffer, reptiles.data(), total_size);
    
    return total_size;
}

bool SaveSystem::decompress_reptile_data(const uint8_t* buffer, size_t buffer_size, std::vector<Reptile>& reptiles) {
    // Décompression simple - copie directe
    size_t reptile_count = buffer_size / sizeof(Reptile);
    
    if (reptile_count * sizeof(Reptile) != buffer_size) {
        ESP_LOGE(TAG, "Taille données incohérente pour décompression");
        return false;
    }
    
    reptiles.resize(reptile_count);
    memcpy(reptiles.data(), buffer, buffer_size);
    
    return true;
}

uint32_t SaveSystem::calculate_checksum(const uint8_t* data, size_t size) {
    uint32_t checksum = 0;
    for (size_t i = 0; i < size; i++) {
        checksum = (checksum << 1) ^ data[i];
    }
    return checksum;
}

bool SaveSystem::verify_data_integrity(const uint8_t* data, size_t size, uint32_t expected_checksum) {
    uint32_t calculated_checksum = calculate_checksum(data, size);
    return calculated_checksum == expected_checksum;
}

bool SaveSystem::has_save_data() const {
    if (!is_initialized) return false;
    
    size_t required_size = 0;
    esp_err_t err = nvs_get_blob(nvs_handle, KEY_SAVE_VERSION, nullptr, &required_size);
    return (err == ESP_OK && required_size > 0);
}

uint32_t SaveSystem::get_last_save_time() const {
    if (!is_initialized) return 0;
    
    uint32_t last_save_time = 0;
    size_t required_size = sizeof(last_save_time);
    esp_err_t err = nvs_get_blob(nvs_handle, KEY_LAST_SAVE_TIME, &last_save_time, &required_size);
    
    return (err == ESP_OK) ? last_save_time : 0;
}

void SaveSystem::auto_save() {
    save_game_data();
}

void SaveSystem::emergency_save() {
    ESP_LOGW(TAG, "🚨 SAUVEGARDE D'URGENCE");
    save_game_data();
    nvs_commit(nvs_handle); // Force la validation immédiate
}

void SaveSystem::clear_all_data() {
    if (!is_initialized) return;
    
    ESP_LOGW(TAG, "Suppression de toutes les données de sauvegarde");
    nvs_erase_all(nvs_handle);
    nvs_commit(nvs_handle);
}