#include "include/display_driver.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include <cstring>

static const char* TAG = "DisplayDriver";

#define CH422_ADDR 0x40
#define FT5X06_ADDR 0x38

static esp_err_t exio_set_level(uint8_t exio, int level) {
    uint8_t cmd[2] = { static_cast<uint8_t>(0x70 | ((exio & 0x06) << 1)),
                       static_cast<uint8_t>(level ? (1 << (exio & 0x07)) : 0) };
    return i2c_master_write_to_device(I2C_NUM_0, CH422_ADDR, cmd, sizeof(cmd), pdMS_TO_TICKS(100));
}

static esp_err_t ft5x06_read(uint8_t reg, uint8_t* data, size_t len) {
    return i2c_master_write_read_device(I2C_NUM_0, FT5X06_ADDR, &reg, 1, data, len, pdMS_TO_TICKS(100));
}

// Buffers LVGL statiques
lv_display_t* DisplayDriver::lvgl_display = nullptr;
lv_color_t* DisplayDriver::buf1 = nullptr;
lv_color_t* DisplayDriver::buf2 = nullptr;
lv_indev_t* DisplayDriver::touch_indev = nullptr;

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

    lv_init();

    if (!configure_touch_interface()) {
        ESP_LOGE(TAG, "Échec configuration tactile");
        return false;
    }

    exio_set_level(LCD_VDD_EN_EXIO, 1);

    if (!configure_lcd_interface()) {
        ESP_LOGE(TAG, "Échec configuration LCD");
        return false;
    }

    if (!configure_lvgl()) {
        ESP_LOGE(TAG, "Échec configuration LVGL");
        return false;
    }

    set_brightness(100);

    ESP_LOGI(TAG, "Pilote d'affichage initialisé avec succès");
    return true;
}

bool DisplayDriver::configure_lcd_interface() {
    static const int rgb_pins[16] = {
        LCD_PIN_B3, LCD_PIN_B4, LCD_PIN_B5, LCD_PIN_B6, LCD_PIN_B7,   // D0..D4
        LCD_PIN_G2, LCD_PIN_G3, LCD_PIN_G4, LCD_PIN_G5, LCD_PIN_G6, LCD_PIN_G7, // D5..D10
        LCD_PIN_R3, LCD_PIN_R4, LCD_PIN_R5, LCD_PIN_R6, LCD_PIN_R7,   // D11..D15
    };

    esp_lcd_rgb_panel_config_t panel_config = {};
    memcpy(panel_config.data_gpio_nums, rgb_pins, sizeof(rgb_pins));

    panel_config.data_width        = 16;
    panel_config.bits_per_pixel    = 16;
    panel_config.clk_src           = LCD_CLK_SRC_DEFAULT;
    panel_config.timings           = {
        .pclk_hz         = 12000000,
        .h_res           = LCD_WIDTH,
        .v_res           = LCD_HEIGHT,
        .hsync_pulse_width = 10,
        .hsync_back_porch  = 20,
        .hsync_front_porch = 10,
        .vsync_pulse_width = 2,
        .vsync_back_porch  = 8,
        .vsync_front_porch = 4,
        .flags = {
            .hsync_idle_low   = 0,
            .vsync_idle_low   = 0,
            .de_idle_high     = 0,
            .pclk_active_neg  = 0,
        },
    };
    panel_config.hsync_gpio_num    = LCD_PIN_HSYNC;
    panel_config.vsync_gpio_num    = LCD_PIN_VSYNC;
    panel_config.de_gpio_num       = LCD_PIN_DE;
    panel_config.pclk_gpio_num     = LCD_PIN_PCLK;
    panel_config.disp_gpio_num     = -1;
    panel_config.num_fbs           = 1;
    panel_config.bounce_buffer_size_px = 0;
    panel_config.flags.fb_in_psram = 1;

    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
    return true;
}

bool DisplayDriver::configure_touch_interface() {
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

    // Reset du contrôleur tactile via l'expander
    exio_set_level(TOUCH_RST_EXIO, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    exio_set_level(TOUCH_RST_EXIO, 1);
    vTaskDelay(pdMS_TO_TICKS(100));

    uint8_t reg = 0xA8; // registre ID
    uint8_t id = 0;
    if (ft5x06_read(reg, &id, 1) != ESP_OK) {
        ESP_LOGE(TAG, "FT5x06 non détecté");
        return false;
    }
    ESP_LOGI(TAG, "Contrôleur tactile ID: 0x%02X", id);

    touch_indev = lv_indev_create();
    lv_indev_set_type(touch_indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(touch_indev, lvgl_touch_cb);
    lv_indev_set_user_data(touch_indev, this);

    return true;
}

bool DisplayDriver::configure_lvgl() {
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

    if (touch_indev) {
        lv_indev_set_display(touch_indev, lvgl_display);
    }

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
    uint8_t buf[5];
    if (ft5x06_read(0x02, buf, sizeof(buf)) == ESP_OK) {
        uint8_t touches = buf[0] & 0x0F;
        if (touches) {
            uint16_t x = ((buf[1] & 0x0F) << 8) | buf[2];
            uint16_t y = ((buf[3] & 0x0F) << 8) | buf[4];
            data->state = LV_INDEV_STATE_PRESSED;
            data->point.x = x;
            data->point.y = y;
            return;
        }
    }
    data->state = LV_INDEV_STATE_RELEASED;
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