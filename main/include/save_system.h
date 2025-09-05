#pragma once

#include "nvs_flash.h" 
#include "nvs.h"
#include "reptile_types.h"
#include <vector>

class SaveSystem {
private:
    nvs_handle_t nvs_handle;
    bool is_initialized;
    uint32_t save_version;
    
    // Clés de sauvegarde
    static const char* NVS_NAMESPACE;
    static const char* KEY_REPTILE_COUNT;
    static const char* KEY_REPTILE_DATA;
    static const char* KEY_GAME_SETTINGS;
    static const char* KEY_SAVE_VERSION;
    static const char* KEY_LAST_SAVE_TIME;
    
    // Données de sauvegarde compressées
    struct SaveHeader {
        uint32_t version;
        uint32_t timestamp;
        uint16_t reptile_count;
        uint32_t checksum;
    };
    
    // Compression/décompression simple
    size_t compress_reptile_data(const std::vector<Reptile>& reptiles, uint8_t* buffer, size_t buffer_size);
    bool decompress_reptile_data(const uint8_t* buffer, size_t buffer_size, std::vector<Reptile>& reptiles);
    
    // Vérification d'intégrité
    uint32_t calculate_checksum(const uint8_t* data, size_t size);
    bool verify_data_integrity(const uint8_t* data, size_t size, uint32_t expected_checksum);
    
public:
    SaveSystem();
    ~SaveSystem();
    
    bool initialize();
    
    // Sauvegarde/chargement principal
    bool save_game_data();
    bool load_game_data();
    
    // Sauvegardes spécialisées
    bool save_reptiles(const std::vector<Reptile>& reptiles);
    bool load_reptiles(std::vector<Reptile>& reptiles);
    
    // Gestion automatique
    void auto_save();
    void emergency_save();
    
    // Utilitaires
    bool has_save_data() const;
    uint32_t get_last_save_time() const;
    size_t get_save_size() const;
    
    // Maintenance
    bool backup_save();
    bool restore_backup();
    void clear_all_data();
    
    // Statistiques
    struct SaveStats {
        uint32_t total_saves;
        uint32_t successful_saves;
        uint32_t failed_saves;
        uint32_t last_save_duration_ms;
        size_t save_data_size;
    };
    
    SaveStats get_save_statistics() const;
};