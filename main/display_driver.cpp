#include "include/display_driver.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"

static const char* TAG = "DisplayDriver";

#define CH422_ADDR 0x40

static esp_err_t exio_set_level(uint8_t exio, int level) {
    uint8_t cmd[2] = { static_cast<uint8_t>(0x70 | ((exio & 0x06) << 1)),
                       static_cast<uint8_t>(level ? (1 << (exio & 0x07)) : 0) };
    return i2c_master_write_to_device(I2C_NUM_0, CH422_ADDR, cmd, sizeof(cmd), pdMS_TO_TICKS(100));
}

// Buffers LVGL statiques
lv_display_t* DisplayDriver::lvgl_display = nullptr;
lv_color_t* DisplayDriver::buf1 = nullptr;
lv_color_t* DisplayDriver::buf2 = nullptr;

DisplayDriver::DisplayDriver() : panel_handle(nullptr) {
}

DisplayDriver::~DisplayDriver() {
    if (panel_handle) {
        esp_lcd_panel_del(panel_handle);
    }
    if (buf1) {
        heap_caps_free(buf1);
    }
    if (buf2) {
        heap_caps_free(buf2);
    }
}

bool DisplayDriver::initialize() {
    ESP_LOGI(TAG, "Initialisation du pilote d'affichage Waveshare ESP32-S3 7\"");
    
    // Initialisation expander pour rétroéclairage et alimentation
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = TOUCH_SDA_PIN,
        .scl_io_num = TOUCH_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master = { .clk_speed = 100000 },
        .clk_flags = 0,
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &i2c_conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, i2c_conf.mode, 0, 0, 0));

    // Activer l'alimentation LCD via EXIO6
    exio_set_level(LCD_VDD_EN_EXIO, 1);
    
    // Configuration des interfaces
    if (!configure_lcd_interface()) {
        ESP_LOGE(TAG, "Échec configuration LCD");
        return false;
    }
    
    if (!configure_lvgl()) {
        ESP_LOGE(TAG, "Échec configuration LVGL");
        return false;
    }
    
    // Activer le rétroéclairage
    set_brightness(100);
    
    ESP_LOGI(TAG, "Pilote d'affichage initialisé avec succès");
    return true;
}

bool DisplayDriver::configure_lcd_interface() {
    esp_lcd_rgb_panel_config_t panel_config = {
        .data_width = 24,
        .psram_trans_align = 64,
        .num_fbs = 1,
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .timings = {
            .pclk_hz = 12000000,
            .h_res = LCD_WIDTH,
            .v_res = LCD_HEIGHT,
            .hsync_pulse_width = 10,
            .hsync_back_porch = 20,
            .hsync_front_porch = 10,
            .vsync_pulse_width = 2,
            .vsync_back_porch = 8,
            .vsync_front_porch = 4,
            .flags = {
                .hsync_idle_low = 0,
                .vsync_idle_low = 0,
                .de_idle_high = 0,
                .pclk_active_neg = 0,
            },
        },
        .hsync_gpio_num = LCD_PIN_HSYNC,
        .vsync_gpio_num = LCD_PIN_VSYNC,
        .de_gpio_num = LCD_PIN_DE,
        .pclk_gpio_num = LCD_PIN_PCLK,
        .disp_gpio_num = -1,
        .data_gpio_nums = {
            LCD_PIN_R0, LCD_PIN_R1, LCD_PIN_R2, LCD_PIN_R3, LCD_PIN_R4, LCD_PIN_R5, LCD_PIN_R6, LCD_PIN_R7,
            LCD_PIN_G0, LCD_PIN_G1, LCD_PIN_G2, LCD_PIN_G3, LCD_PIN_G4, LCD_PIN_G5, LCD_PIN_G6, LCD_PIN_G7,
            LCD_PIN_B0, LCD_PIN_B1, LCD_PIN_B2, LCD_PIN_B3, LCD_PIN_B4, LCD_PIN_B5, LCD_PIN_B6, LCD_PIN_B7,
        },
        .flags = { .fb_in_psram = 1 },
    };

    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
    return true;
}

bool DisplayDriver::configure_lvgl() {
    // Initialisation LVGL
    lv_init();
    
    // Allocation des buffers
    size_t buffer_size = LCD_WIDTH * 60; // 60 lignes de buffer
    buf1 = (lv_color_t*)heap_caps_malloc(
               buffer_size * sizeof(lv_color_t),
               MALLOC_CAP_SPIRAM | MALLOC_CAP_DMA | MALLOC_CAP_8BIT);
    buf2 = (lv_color_t*)heap_caps_malloc(
               buffer_size * sizeof(lv_color_t),
               MALLOC_CAP_SPIRAM | MALLOC_CAP_DMA | MALLOC_CAP_8BIT);

    if (!buf1 || !buf2) {
        // Fallback : réduire la hauteur du buffer à 30 lignes en RAM interne
        size_t small_size = LCD_WIDTH * 30;
        if (buf1) heap_caps_free(buf1);
        if (buf2) heap_caps_free(buf2);
        buf1 = (lv_color_t*)heap_caps_malloc(
                   small_size * sizeof(lv_color_t), MALLOC_CAP_DMA | MALLOC_CAP_8BIT);
        buf2 = (lv_color_t*)heap_caps_malloc(
                   small_size * sizeof(lv_color_t), MALLOC_CAP_DMA | MALLOC_CAP_8BIT);
        if (!buf1 || !buf2) {
            ESP_LOGE(TAG, "Échec allocation buffers LVGL");
            return false;
        }
        buffer_size = small_size;
    }
    
    // Création du display LVGL
    lvgl_display = lv_display_create(LCD_WIDTH, LCD_HEIGHT);
    lv_display_set_flush_cb(lvgl_display, lvgl_flush_cb);
    lv_display_set_buffers(lvgl_display, buf1, buf2, buffer_size * sizeof(lv_color_t), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_user_data(lvgl_display, this);
    
    // Configuration du périphérique d'entrée tactile (simulé pour l'instant)
    lv_indev_t* indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, lvgl_touch_cb);
    lv_indev_set_user_data(indev, this);
    
    return true;
}

void DisplayDriver::lvgl_flush_cb(lv_display_t* display, const lv_area_t* area, uint8_t* color_map) {
    DisplayDriver* driver = static_cast<DisplayDriver*>(lv_display_get_user_data(display));
    
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    
    esp_lcd_panel_draw_bitmap(driver->panel_handle, offsetx1, offsety1, 
                             offsetx2 + 1, offsety2 + 1, color_map);
    
    lv_display_flush_ready(display);
}

void DisplayDriver::lvgl_touch_cb(lv_indev_t* indev, lv_indev_data_t* data) {
    // Implémentation tactile basique - à adapter selon le contrôleur tactile réel
    data->state = LV_INDEV_STATE_RELEASED;
    data->point.x = 0;
    data->point.y = 0;
}

void DisplayDriver::set_brightness(uint8_t brightness) {
    exio_set_level(LCD_BL_EXIO, brightness ? 1 : 0);
    ESP_LOGI(TAG, "Rétroéclairage %s", brightness ? "activé" : "désactivé");
}

void DisplayDriver::update() {
    // Mise à jour LVGL (à appeler dans la boucle principale)
    lv_timer_handler();
}

void DisplayDriver::enable_screen() {
    if (panel_handle) {
        esp_lcd_panel_disp_on_off(panel_handle, true);
    }
}

void DisplayDriver::disable_screen() {
    if (panel_handle) {
        esp_lcd_panel_disp_on_off(panel_handle, false);
    }
}

void DisplayDriver::calibrate_touch() {
    // Implémentation de la calibration tactile si nécessaire
}

bool DisplayDriver::is_touch_calibrated() const {
    return true; // Temporaire
}