#pragma once

#include "lvgl.h"
#include "reptile_types.h"
#include "game_engine.h"

class UIManager {
private:
    GameEngine* game_engine;
    
    // Écrans principaux
    lv_obj_t* main_screen;
    lv_obj_t* reptile_selection_screen;
    lv_obj_t* care_screen;
    lv_obj_t* habitat_screen;
    lv_obj_t* breeding_screen;
    lv_obj_t* statistics_screen;
    lv_obj_t* settings_screen;
    
    // Widgets principaux
    lv_obj_t* reptile_info_panel;
    lv_obj_t* health_bars;
    lv_obj_t* environment_controls;
    lv_obj_t* feeding_panel;
    lv_obj_t* behavior_display;
    lv_obj_t* notification_area;
    
    // Styles personnalisés
    lv_style_t style_bg;
    lv_style_t style_card;
    lv_style_t style_btn_primary;
    lv_style_t style_btn_secondary;
    lv_style_t style_health_good;
    lv_style_t style_health_warning;
    lv_style_t style_health_critical;
    
    // État de l'interface
    static constexpr uint8_t screen_count = 7;
    lv_obj_t* screens[screen_count];
    uint8_t current_screen;
    bool notification_visible;
    uint32_t last_ui_update;
    
    // Méthodes de construction d'interface
    void create_main_screen();
    void create_reptile_info_display();
    void create_health_monitoring();
    void create_environment_controls();
    void create_feeding_interface();
    void create_care_buttons();
    void create_navigation_menu();
    
    // Callbacks d'événements
    static void on_reptile_select(lv_event_t* e);
    static void on_feed_button(lv_event_t* e);
    static void on_temperature_adjust(lv_event_t* e);
    static void on_humidity_adjust(lv_event_t* e);
    static void on_lighting_toggle(lv_event_t* e);
    static void on_clean_terrarium(lv_event_t* e);
    static void on_health_check(lv_event_t* e);
    static void on_navigation_click(lv_event_t* e);
    
    // Utilitaires UI
    void update_health_display(const Reptile& reptile);
    void update_environment_display(const Reptile& reptile);
    void update_behavior_animation(const Reptile& reptile);
    void show_notification(const char* message, bool is_warning = false);
    void hide_notification();
    
    // Animations
    void animate_feeding(lv_obj_t* target);
    void animate_health_change(lv_obj_t* health_bar, uint8_t new_value);
    void animate_behavior_transition(Behavior new_behavior);
    
public:
    UIManager(GameEngine* engine);
    ~UIManager();
    
    bool initialize();
    void update();
    void handle_touch_input(lv_indev_data_t* data);
    
    // Gestion des écrans
    void switch_to_screen(uint8_t screen_id);
    void refresh_current_screen();
    
    // Notifications système
    void show_feeding_reminder(const char* reptile_name);
    void show_health_alert(const char* reptile_name, const char* issue);
    void show_breeding_notification(const char* message);
    void show_achievement(const char* achievement);
    
    // Interface tactile optimisée
    void enable_touch_feedback();
    void set_screen_brightness(uint8_t brightness);
};

// IDs des écrans
enum ScreenID : uint8_t {
    SCREEN_MAIN = 0,
    SCREEN_REPTILE_SELECT,
    SCREEN_CARE,
    SCREEN_HABITAT,
    SCREEN_BREEDING,
    SCREEN_STATS,
    SCREEN_SETTINGS
};