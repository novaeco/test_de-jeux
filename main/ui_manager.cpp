#include <inttypes.h>
#include "include/ui_manager.h"
#include "include/species_database.h"
#include "esp_log.h"

static const char* TAG = "UIManager";

UIManager::UIManager(GameEngine* engine) 
    : game_engine(engine), current_screen_id(SCREEN_MAIN), notification_visible(false) {
    last_ui_update = 0;
}

UIManager::~UIManager() {
    // Nettoyage des ressources LVGL
}

bool UIManager::initialize() {
    ESP_LOGI(TAG, "Initialisation de l'interface utilisateur");
    
    // Configuration des styles personnalisés
    lv_style_init(&style_bg);
    lv_style_set_bg_color(&style_bg, lv_color_hex(0x2E3440));
    lv_style_set_bg_opa(&style_bg, LV_OPA_COVER);
    
    lv_style_init(&style_card);
    lv_style_set_bg_color(&style_card, lv_color_hex(0x3B4252));
    lv_style_set_bg_opa(&style_card, LV_OPA_COVER);
    lv_style_set_radius(&style_card, 12);
    lv_style_set_pad_all(&style_card, 16);
    lv_style_set_shadow_width(&style_card, 8);
    lv_style_set_shadow_color(&style_card, lv_color_hex(0x000000));
    lv_style_set_shadow_opa(&style_card, LV_OPA_30);
    
    lv_style_init(&style_btn_primary);
    lv_style_set_bg_color(&style_btn_primary, lv_color_hex(0x5E81AC));
    lv_style_set_bg_opa(&style_btn_primary, LV_OPA_COVER);
    lv_style_set_radius(&style_btn_primary, 8);
    lv_style_set_text_color(&style_btn_primary, lv_color_hex(0xECEFF4));
    
    lv_style_init(&style_health_good);
    lv_style_set_bg_color(&style_health_good, lv_color_hex(0xA3BE8C));
    
    lv_style_init(&style_health_warning);
    lv_style_set_bg_color(&style_health_warning, lv_color_hex(0xEBCB8B));
    
    lv_style_init(&style_health_critical);
    lv_style_set_bg_color(&style_health_critical, lv_color_hex(0xBF616A));
    
    // Création de l'écran principal
    create_main_screen();
    
    ESP_LOGI(TAG, "Interface utilisateur initialisée avec succès");
    return true;
}

void UIManager::create_main_screen() {
    main_screen = lv_obj_create(NULL);
    lv_obj_add_style(main_screen, &style_bg, 0);
    
    // Panneau principal - informations sur le reptile sélectionné
    reptile_info_panel = lv_obj_create(main_screen);
    lv_obj_add_style(reptile_info_panel, &style_card, 0);
    lv_obj_set_size(reptile_info_panel, 480, 200);
    lv_obj_set_pos(reptile_info_panel, 20, 20);
    
    // Nom et espèce du reptile
    lv_obj_t* name_label = lv_label_create(reptile_info_panel);
    lv_label_set_text(name_label, "Sélectionnez un reptile");
    lv_obj_set_style_text_font(name_label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(name_label, lv_color_hex(0xECEFF4), 0);
    lv_obj_set_pos(name_label, 0, 0);
    
    // Avatar/illustration du reptile (zone réservée pour graphics)
    lv_obj_t* reptile_avatar = lv_obj_create(reptile_info_panel);
    lv_obj_set_size(reptile_avatar, 120, 120);
    lv_obj_set_pos(reptile_avatar, 320, 40);
    lv_obj_set_style_bg_color(reptile_avatar, lv_color_hex(0x4C566A), 0);
    lv_obj_set_style_radius(reptile_avatar, 60, 0);
    
    // Informations vitales compactes
    lv_obj_t* vital_info = lv_label_create(reptile_info_panel);
    lv_label_set_text(vital_info, "Âge: -- jours\nPoids: -- g\nTaille: -- mm");
    lv_obj_set_style_text_color(vital_info, lv_color_hex(0xD8DEE9), 0);
    lv_obj_set_pos(vital_info, 0, 50);
    
    // Barres de santé
    create_health_monitoring();
    
    // Panneau de contrôles environnementaux
    create_environment_controls();
    
    // Interface d'alimentation
    create_feeding_interface();
    
    // Boutons de soin rapide
    create_care_buttons();
    
    // Menu de navigation
    create_navigation_menu();
    
    // Zone de notifications
    notification_area = lv_obj_create(main_screen);
    lv_obj_set_size(notification_area, 600, 60);
    lv_obj_set_pos(notification_area, 200, 10);
    lv_obj_add_flag(notification_area, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(notification_area, lv_color_hex(0xEBCB8B), 0);
    lv_obj_set_style_radius(notification_area, 8, 0);
    
    lv_scr_load(main_screen);
}

void UIManager::create_health_monitoring() {
    health_bars = lv_obj_create(main_screen);
    lv_obj_add_style(health_bars, &style_card, 0);
    lv_obj_set_size(health_bars, 280, 180);
    lv_obj_set_pos(health_bars, 520, 20);
    
    // Titre
    lv_obj_t* title = lv_label_create(health_bars);
    lv_label_set_text(title, "État de santé");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xECEFF4), 0);
    
    // Barres de santé avec icônes
    const char* health_labels[] = {"Santé", "Faim", "Hydratation", "Stress"};
    
    for (int i = 0; i < 4; i++) {
        // Label
        lv_obj_t* label = lv_label_create(health_bars);
        lv_label_set_text(label, health_labels[i]);
        lv_obj_set_pos(label, 0, 30 + i * 35);
        lv_obj_set_style_text_color(label, lv_color_hex(0xD8DEE9), 0);
        
        // Barre de progression
        lv_obj_t* bar = lv_bar_create(health_bars);
        lv_obj_set_size(bar, 150, 20);
        lv_obj_set_pos(bar, 100, 30 + i * 35);
        lv_bar_set_value(bar, 75, LV_ANIM_ON); // Valeur par défaut
        
        // Couleur dynamique selon la valeur
        if (i == 3) { // Stress - inversé (rouge = mauvais)
            lv_obj_set_style_bg_color(bar, lv_color_hex(0xBF616A), LV_PART_INDICATOR);
        } else {
            lv_obj_set_style_bg_color(bar, lv_color_hex(0xA3BE8C), LV_PART_INDICATOR);
        }
    }
}

void UIManager::create_environment_controls() {
    environment_controls = lv_obj_create(main_screen);
    lv_obj_add_style(environment_controls, &style_card, 0);
    lv_obj_set_size(environment_controls, 380, 120);
    lv_obj_set_pos(environment_controls, 20, 240);
    
    lv_obj_t* title = lv_label_create(environment_controls);
    lv_label_set_text(title, "Contrôles environnementaux");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xECEFF4), 0);
    
    // Contrôle température
    lv_obj_t* temp_container = lv_obj_create(environment_controls);
    lv_obj_set_size(temp_container, 180, 60);
    lv_obj_set_pos(temp_container, 0, 30);
    lv_obj_clear_flag(temp_container, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_obj_t* temp_label = lv_label_create(temp_container);
    lv_label_set_text(temp_label, "🌡️ Température: 30°C");
    lv_obj_set_style_text_color(temp_label, lv_color_hex(0xD8DEE9), 0);
    
    lv_obj_t* temp_slider = lv_slider_create(temp_container);
    lv_obj_set_size(temp_slider, 150, 20);
    lv_obj_set_pos(temp_slider, 0, 25);
    lv_slider_set_range(temp_slider, 18, 45);
    lv_slider_set_value(temp_slider, 30, LV_ANIM_OFF);
    lv_obj_add_event_cb(temp_slider, on_temperature_adjust, LV_EVENT_VALUE_CHANGED, this);
    
    // Contrôle humidité
    lv_obj_t* hum_container = lv_obj_create(environment_controls);
    lv_obj_set_size(hum_container, 180, 60);
    lv_obj_set_pos(hum_container, 190, 30);
    lv_obj_clear_flag(hum_container, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_obj_t* hum_label = lv_label_create(hum_container);
    lv_label_set_text(hum_label, "💧 Humidité: 60%");
    lv_obj_set_style_text_color(hum_label, lv_color_hex(0xD8DEE9), 0);
    
    lv_obj_t* hum_slider = lv_slider_create(hum_container);
    lv_obj_set_size(hum_slider, 150, 20);
    lv_obj_set_pos(hum_slider, 0, 25);
    lv_slider_set_range(hum_slider, 20, 90);
    lv_slider_set_value(hum_slider, 60, LV_ANIM_OFF);
    lv_obj_add_event_cb(hum_slider, on_humidity_adjust, LV_EVENT_VALUE_CHANGED, this);
}

void UIManager::create_feeding_interface() {
    feeding_panel = lv_obj_create(main_screen);
    lv_obj_add_style(feeding_panel, &style_card, 0);
    lv_obj_set_size(feeding_panel, 420, 120);
    lv_obj_set_pos(feeding_panel, 420, 240);
    
    lv_obj_t* title = lv_label_create(feeding_panel);
    lv_label_set_text(title, "🍽️ Alimentation");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xECEFF4), 0);
    
    // Boutons de nourriture spécifique
    const char* food_names[] = {"🦗 Grillons", "🪱 Vers", "🥬 Légumes", "🧬 Compléments"};
    FoodType food_types[] = {FoodType::CRICKETS, FoodType::MEALWORMS, FoodType::LEAFY_GREENS, FoodType::CALCIUM_SUPPLEMENT};
    
    for (int i = 0; i < 4; i++) {
        lv_obj_t* food_btn = lv_btn_create(feeding_panel);
        lv_obj_add_style(food_btn, &style_btn_primary, 0);
        lv_obj_set_size(food_btn, 90, 40);
        lv_obj_set_pos(food_btn, (i % 2) * 100, 30 + (i / 2) * 50);
        
        lv_obj_t* btn_label = lv_label_create(food_btn);
        lv_label_set_text(btn_label, food_names[i]);
        lv_obj_center(btn_label);
        
        // Stocker le type de nourriture comme user_data
        lv_obj_set_user_data(food_btn, (void*)(intptr_t)food_types[i]);
        lv_obj_add_event_cb(food_btn, on_feed_button, LV_EVENT_CLICKED, this);
    }
    
    // Indicateur de dernière alimentation
    lv_obj_t* last_feeding = lv_label_create(feeding_panel);
    lv_label_set_text(last_feeding, "Dernière alimentation: il y a 2h");
    lv_obj_set_pos(last_feeding, 200, 30);
    lv_obj_set_style_text_color(last_feeding, lv_color_hex(0xD8DEE9), 0);
    lv_obj_set_style_text_font(last_feeding, &lv_font_montserrat_12, 0);
}

void UIManager::create_care_buttons() {
    // Boutons de soins rapides en bas de l'écran
    const char* care_labels[] = {"🧹 Nettoyer", "💡 Éclairage", "🏥 Santé", "👥 Reproduction"};
    lv_event_cb_t callbacks[] = {on_clean_terrarium, on_lighting_toggle, on_health_check, nullptr};
    
    for (int i = 0; i < 4; i++) {
        lv_obj_t* care_btn = lv_btn_create(main_screen);
        lv_obj_add_style(care_btn, &style_btn_secondary, 0);
        lv_obj_set_size(care_btn, 120, 50);
        lv_obj_set_pos(care_btn, 20 + i * 130, 380);
        
        lv_obj_t* btn_label = lv_label_create(care_btn);
        lv_label_set_text(btn_label, care_labels[i]);
        lv_obj_center(btn_label);
        
        if (callbacks[i]) {
            lv_obj_add_event_cb(care_btn, callbacks[i], LV_EVENT_CLICKED, this);
        }
    }
}

void UIManager::create_navigation_menu() {
    // Menu de navigation en bas
    lv_obj_t* nav_menu = lv_obj_create(main_screen);
    lv_obj_set_size(nav_menu, 1024, 60);
    lv_obj_set_pos(nav_menu, 0, 540);
    lv_obj_set_style_bg_color(nav_menu, lv_color_hex(0x2E3440), 0);
    lv_obj_set_style_border_width(nav_menu, 0, 0);
    
    const char* nav_labels[] = {"🏠 Accueil", "🦎 Reptiles", "⚙️ Habitat", "📊 Stats", "⚙️ Paramètres"};
    
    for (int i = 0; i < 5; i++) {
        lv_obj_t* nav_btn = lv_btn_create(nav_menu);
        lv_obj_set_size(nav_btn, 180, 50);
        lv_obj_set_pos(nav_btn, i * 205, 5);
        lv_obj_set_style_bg_color(nav_btn, lv_color_hex(0x4C566A), 0);
        lv_obj_set_style_bg_opa(nav_btn, LV_OPA_80, 0);
        
        lv_obj_t* nav_label = lv_label_create(nav_btn);
        lv_label_set_text(nav_label, nav_labels[i]);
        lv_obj_center(nav_label);
        lv_obj_set_style_text_color(nav_label, lv_color_hex(0xECEFF4), 0);
        
        lv_obj_set_user_data(nav_btn, (void*)(intptr_t)i);
        lv_obj_add_event_cb(nav_btn, on_navigation_click, LV_EVENT_CLICKED, this);
    }
}

// Callbacks d'événements
void UIManager::on_feed_button(lv_event_t* e) {
    UIManager* ui = static_cast<UIManager*>(lv_event_get_user_data(e));
    lv_obj_t* btn = static_cast<lv_obj_t*>(lv_event_get_target(e));
    FoodType food = static_cast<FoodType>((intptr_t)lv_obj_get_user_data(btn));
    
    uint8_t selected = ui->game_engine->get_selected_reptile();
    if (ui->game_engine->feed_reptile(selected, food)) {
        ui->show_notification("✅ Alimentation réussie!", false);
        ui->animate_feeding(btn);
    } else {
        ui->show_notification("❌ Nourriture inappropriée", true);
    }
}

void UIManager::on_temperature_adjust(lv_event_t* e) {
    UIManager* ui = static_cast<UIManager*>(lv_event_get_user_data(e));
    lv_obj_t* slider = static_cast<lv_obj_t*>(lv_event_get_target(e));
    int32_t value = lv_slider_get_value(slider);
    
    uint8_t selected = ui->game_engine->get_selected_reptile();
    if (ui->game_engine->adjust_temperature(selected, (float)value)) {
        char msg[64];
        snprintf(msg, sizeof(msg), "🌡️ Température: %" PRId32 "°C", value);
        ui->show_notification(msg, false);
    }
}

void UIManager::update() {
    if (game_engine->get_reptile_count() == 0) return;
    
    uint8_t selected = game_engine->get_selected_reptile();
    Reptile* reptile = game_engine->get_reptile(selected);
    if (!reptile) return;
    
    // Mise à jour des informations vitales
    update_health_display(*reptile);
    update_environment_display(*reptile);
    update_behavior_animation(*reptile);
    
    // Vérifications d'alertes
    if (reptile->health.hunger_level > 80) {
        show_feeding_reminder(reptile->name);
    }
    
    if (reptile->health.overall_health < 30) {
        show_health_alert(reptile->name, "Santé critique!");
    }
}

void UIManager::update_health_display(const Reptile& reptile) {
    // Mise à jour des barres de santé avec animations fluides
    // Implémentation détaillée des animations et couleurs dynamiques
}

void UIManager::show_notification(const char* message, bool is_warning) {
    if (!notification_area) return;
    
    lv_obj_t* notif_label = lv_label_create(notification_area);
    lv_label_set_text(notif_label, message);
    lv_obj_center(notif_label);
    lv_obj_set_style_text_color(notif_label, lv_color_hex(0x2E3440), 0);
    
    lv_obj_clear_flag(notification_area, LV_OBJ_FLAG_HIDDEN);
    
    // Animation d'apparition
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, notification_area);
    lv_anim_set_values(&anim, -100, 10);
    lv_anim_set_time(&anim, 300);
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_start(&anim);
    
    notification_visible = true;
    
    // Auto-masquage après 3 secondes
    lv_timer_create([](lv_timer_t* timer) {
        UIManager* ui = static_cast<UIManager*>(lv_timer_get_user_data(timer));
        ui->hide_notification();
        lv_timer_del(timer);
    }, 3000, this);
}

void UIManager::animate_feeding(lv_obj_t* target) {
    // Animation de "pulse" pour le feedback tactile
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, target);
    lv_anim_set_values(&anim, 100, 120);
    lv_anim_set_time(&anim, 150);
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_obj_set_width);
    lv_anim_set_playback_time(&anim, 150);
    lv_anim_start(&anim);
}

void UIManager::hide_notification() {
    if (!notification_visible) return;
    
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, notification_area);
    lv_anim_set_values(&anim, 10, -100);
    lv_anim_set_time(&anim, 200);
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_ready_cb(&anim, [](lv_anim_t* a) {
        lv_obj_add_flag(static_cast<lv_obj_t*>(a->var), LV_OBJ_FLAG_HIDDEN);
    });
    lv_anim_start(&anim);
    
    notification_visible = false;
}

void UIManager::on_humidity_adjust(lv_event_t* e) {
    UIManager* ui = static_cast<UIManager*>(lv_event_get_user_data(e));
    lv_obj_t* slider = static_cast<lv_obj_t*>(lv_event_get_target(e));
    int32_t value = lv_slider_get_value(slider);

    uint8_t selected = ui->game_engine->get_selected_reptile();
    if (ui->game_engine->adjust_humidity(selected, static_cast<float>(value))) {
        char msg[64];
        snprintf(msg, sizeof(msg), "\xF0\x9F\x92\xA7 Humidit\xC3\xA9: %" PRId32 "%%", value);
        ui->show_notification(msg, false);
    }
}

void UIManager::on_navigation_click(lv_event_t* e) {
    UIManager* ui = static_cast<UIManager*>(lv_event_get_user_data(e));
    lv_obj_t* btn = static_cast<lv_obj_t*>(lv_event_get_target(e));
    uint8_t index = static_cast<uint8_t>((intptr_t)lv_obj_get_user_data(btn));
    ui->switch_to_screen(index);
}

void UIManager::on_clean_terrarium(lv_event_t* e) {
    UIManager* ui = static_cast<UIManager*>(lv_event_get_user_data(e));
    uint8_t selected = ui->game_engine->get_selected_reptile();
    if (ui->game_engine->clean_terrarium(selected)) {
        ui->show_notification("\xF0\x9F\xA7\xB9 Terrarium nettoy\xC3\xA9", false);
    }
}

void UIManager::on_lighting_toggle(lv_event_t* e) {
    UIManager* ui = static_cast<UIManager*>(lv_event_get_user_data(e));
    uint8_t selected = ui->game_engine->get_selected_reptile();
    if (ui->game_engine->toggle_lighting(selected)) {
        ui->show_notification("\xF0\x9F\x92\xA1 \xC3\x89clairage bascul\xC3\xA9", false);
    }
}

void UIManager::on_health_check(lv_event_t* e) {
    UIManager* ui = static_cast<UIManager*>(lv_event_get_user_data(e));
    uint8_t selected = ui->game_engine->get_selected_reptile();
    if (ui->game_engine->diagnose_health_issue(selected)) {
        ui->show_notification("\xF0\x9F\x8F\xA5 Bilan de sant\xC3\xA9 effectu\xC3\xA9", false);
    }
}

void UIManager::update_environment_display(const Reptile& reptile) {
    if (!environment_controls) return;

    lv_obj_t* temp_container = lv_obj_get_child(environment_controls, 1);
    lv_obj_t* temp_label = lv_obj_get_child(temp_container, 0);
    lv_obj_t* temp_slider = lv_obj_get_child(temp_container, 1);

    char buf[64];
    snprintf(buf, sizeof(buf), "\xF0\x9F\x8C\xA1 Temp\xC3\xA9rature: %.0f\xC2\xB0C", reptile.habitat.temperature_day);
    lv_label_set_text(temp_label, buf);
    lv_slider_set_value(temp_slider, static_cast<int32_t>(reptile.habitat.temperature_day), LV_ANIM_OFF);

    lv_obj_t* hum_container = lv_obj_get_child(environment_controls, 2);
    lv_obj_t* hum_label = lv_obj_get_child(hum_container, 0);
    lv_obj_t* hum_slider = lv_obj_get_child(hum_container, 1);

    snprintf(buf, sizeof(buf), "\xF0\x9F\x92\xA7 Humidit\xC3\xA9: %.0f%%", reptile.habitat.humidity);
    lv_label_set_text(hum_label, buf);
    lv_slider_set_value(hum_slider, static_cast<int32_t>(reptile.habitat.humidity), LV_ANIM_OFF);
}

void UIManager::update_behavior_animation(const Reptile& reptile) {
    if (!behavior_display) return;

    const char* icon;
    switch (reptile.current_behavior) {
        case Behavior::BASKING:      icon = "\xE2\x98\x80\xEF\xB8\x8F Basking"; break;
        case Behavior::HIDING:       icon = "\xF0\x9F\x95\xB5\xEF\xB8\x8F C\xE3\xA9ch\xC3\xA9"; break;
        case Behavior::EXPLORING:    icon = "\xF0\x9F\x90\x8E Explore"; break;
        case Behavior::FEEDING:      icon = "\xF0\x9F\x8D\xBD Mange"; break;
        case Behavior::SLEEPING:     icon = "\xF0\x9F\x98\xB4 Dort"; break;
        case Behavior::SHEDDING:     icon = "\xF0\x9F\xAA\xA0 Mue"; break;
        case Behavior::COURTING:     icon = "\xF0\x9F\x92\x8F Parade"; break;
        case Behavior::AGGRESSIVE:   icon = "\xF0\x9F\x98\xA1 Agressif"; break;
        case Behavior::STRESSED:     icon = "\xF0\x9F\x98\xB1 Stress"; break;
        case Behavior::BRUMATION:    icon = "\xE2\x9D\x84\xEF\xB8\x8F Brumation"; break;
        default:                     icon = "\xE2\x9D\x93"; break;
    }
    lv_label_set_text(behavior_display, icon);
}

void UIManager::show_feeding_reminder(const char* reptile_name) {
    char msg[128];
    snprintf(msg, sizeof(msg), "\xF0\x9F\x8D\xBD Rappel: nourrir %s", reptile_name);
    show_notification(msg, false);
}

void UIManager::show_health_alert(const char* reptile_name, const char* issue) {
    char msg[128];
    snprintf(msg, sizeof(msg), "\xE2\x9A\xA0\xEF\xB8\x8F %s: %s", reptile_name, issue);
    show_notification(msg, true);
}