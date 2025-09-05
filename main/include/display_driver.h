#pragma once

#include "esp_lcd_panel_ops.h"
#include "esp_lcd_touch.h"
#include "lvgl.h"

// Configuration spécifique Waveshare ESP32-S3 7" Touch LCD
#define LCD_WIDTH           1024
#define LCD_HEIGHT          600
#define LCD_BIT_PER_PIXEL   16

// Pins de connexion pour la Waveshare ESP32-S3
#define LCD_SCLK_PIN        14
#define LCD_MOSI_PIN        13
#define LCD_MISO_PIN        12
#define LCD_CS_PIN          15
#define LCD_DC_PIN          2
#define LCD_RST_PIN         -1  // Connecté au reset de l'ESP32
#define LCD_BL_PIN          21  // Contrôle rétroéclairage

#define TOUCH_SDA_PIN       19
#define TOUCH_SCL_PIN       20
#define TOUCH_INT_PIN       -1  // Non utilisé
#define TOUCH_RST_PIN       -1  // Non utilisé

class DisplayDriver {
private:
    esp_lcd_panel_handle_t panel_handle;
    esp_lcd_touch_handle_t touch_handle;
    
    // Buffers LVGL
    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t buf1[LCD_WIDTH * 60];
    static lv_color_t buf2[LCD_WIDTH * 60];
    
    // Driver callbacks
    static void lvgl_flush_cb(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* color_map);
    static void lvgl_touch_cb(lv_indev_drv_t* drv, lv_indev_data_t* data);
    
    // Configuration LCD
    bool configure_lcd_interface();
    bool configure_touch_interface();
    bool configure_lvgl();
    
    // Gestion rétroéclairage
    void set_backlight_duty(uint8_t duty_percent);
    
public:
    DisplayDriver();
    ~DisplayDriver();
    
    bool initialize();
    void update();
    
    // Contrôles d'affichage
    void set_brightness(uint8_t brightness);  // 0-100%
    void enable_screen();
    void disable_screen();
    
    // Informations d'affichage
    uint16_t get_width() const { return LCD_WIDTH; }
    uint16_t get_height() const { return LCD_HEIGHT; }
    
    // Calibration tactile (si nécessaire)
    void calibrate_touch();
    bool is_touch_calibrated() const;
};