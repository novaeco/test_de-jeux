#pragma once

#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "lvgl.h"
#include "driver/i2c.h"
#include "driver/gpio.h"

// Configuration spécifique Waveshare ESP32-S3 7" Touch LCD
#define LCD_WIDTH           1024
#define LCD_HEIGHT          600
#define LCD_BIT_PER_PIXEL   16

// Brochage RGB (d'après le tableau "LCD port" du wiki)
#define LCD_PIN_R0          GPIO_NUM_NC
#define LCD_PIN_R1          GPIO_NUM_NC
#define LCD_PIN_R2          GPIO_NUM_NC
#define LCD_PIN_R3          GPIO_NUM_1
#define LCD_PIN_R4          GPIO_NUM_2
#define LCD_PIN_R5          GPIO_NUM_42
#define LCD_PIN_R6          GPIO_NUM_41
#define LCD_PIN_R7          GPIO_NUM_40

#define LCD_PIN_G0          GPIO_NUM_NC
#define LCD_PIN_G1          GPIO_NUM_NC
#define LCD_PIN_G2          GPIO_NUM_39
#define LCD_PIN_G3          GPIO_NUM_0
#define LCD_PIN_G4          GPIO_NUM_45
#define LCD_PIN_G5          GPIO_NUM_48
#define LCD_PIN_G6          GPIO_NUM_47
#define LCD_PIN_G7          GPIO_NUM_21

#define LCD_PIN_B0          GPIO_NUM_NC
#define LCD_PIN_B1          GPIO_NUM_NC
#define LCD_PIN_B2          GPIO_NUM_NC
#define LCD_PIN_B3          GPIO_NUM_14
#define LCD_PIN_B4          GPIO_NUM_38
#define LCD_PIN_B5          GPIO_NUM_18
#define LCD_PIN_B6          GPIO_NUM_17
#define LCD_PIN_B7          GPIO_NUM_10

#define LCD_PIN_HSYNC       GPIO_NUM_46
#define LCD_PIN_VSYNC       GPIO_NUM_3
#define LCD_PIN_DE          GPIO_NUM_5
#define LCD_PIN_PCLK        GPIO_NUM_7

// Commandes via expander CH422G
#define LCD_BL_EXIO         2   // EXIO2 : rétroéclairage
#define LCD_VDD_EN_EXIO     6   // EXIO6 : alimentation LCD

#define TOUCH_SDA_PIN       GPIO_NUM_8
#define TOUCH_SCL_PIN       GPIO_NUM_9
#define TOUCH_INT_PIN       GPIO_NUM_4
#define TOUCH_RST_PIN       GPIO_NUM_NC

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