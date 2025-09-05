#pragma once

#include "esp_lcd_panel_ops.h"
#include "lvgl.h"
#include "driver/spi_master.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

// Configuration spécifique Waveshare ESP32-S3 7" Touch LCD
#define LCD_WIDTH           1024
#define LCD_HEIGHT          600
#define LCD_BIT_PER_PIXEL   16

// Pins de connexion pour la Waveshare ESP32-S3
#define LCD_SCLK_PIN        GPIO_NUM_14
#define LCD_MOSI_PIN        GPIO_NUM_13
#define LCD_MISO_PIN        GPIO_NUM_12
#define LCD_CS_PIN          GPIO_NUM_15
#define LCD_DC_PIN          GPIO_NUM_2
#define LCD_RST_PIN         GPIO_NUM_NC  // Non connecté
#define LCD_BL_PIN          GPIO_NUM_21  // Contrôle rétroéclairage

#define TOUCH_SDA_PIN       GPIO_NUM_19
#define TOUCH_SCL_PIN       GPIO_NUM_20
#define TOUCH_INT_PIN       GPIO_NUM_NC  // Non utilisé
#define TOUCH_RST_PIN       GPIO_NUM_NC  // Non utilisé

class DisplayDriver {
private:
    esp_lcd_panel_handle_t panel_handle;
    
    // Buffers LVGL
    static lv_display_t* lvgl_display;
    static lv_color_t* buf1;
    static lv_color_t* buf2;
    
    // Driver callbacks
    static void lvgl_flush_cb(lv_display_t* display, const lv_area_t* area, uint8_t* color_map);
    static void lvgl_touch_cb(lv_indev_t* indev, lv_indev_data_t* data);
    
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