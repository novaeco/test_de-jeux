#include "include/display_driver.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_io.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "DisplayDriver";

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
        free(buf1);
    }
    if (buf2) {
        free(buf2);
    }
}

bool DisplayDriver::initialize() {
    ESP_LOGI(TAG, "Initialisation du pilote d'affichage Waveshare ESP32-S3 7\"");
    
    // Configuration du rétroéclairage PWM
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 1000,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
    
    ledc_channel_config_t ledc_channel = {
        .gpio_num = LCD_BL_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
    
    // Configuration des interfaces
    if (!configure_lcd_interface()) {
        ESP_LOGE(TAG, "Échec configuration LCD");
        return false;
    }
    
    if (!configure_lvgl()) {
        ESP_LOGE(TAG, "Échec configuration LVGL");
        return false;
    }
    
    // Activer le rétroéclairage à 80%
    set_brightness(80);
    
    ESP_LOGI(TAG, "Pilote d'affichage initialisé avec succès");
    return true;
}

bool DisplayDriver::configure_lcd_interface() {
    // Configuration SPI pour le LCD
    spi_bus_config_t buscfg = {
        .mosi_io_num = LCD_MOSI_PIN,
        .miso_io_num = LCD_MISO_PIN,
        .sclk_io_num = LCD_SCLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t),
        .flags = 0,
        .isr_cpu_id = INTR_CPU_ID_AUTO,
        .intr_flags = 0,
    };
    
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    
    esp_lcd_panel_io_spi_config_t io_config = {
        .cs_gpio_num = LCD_CS_PIN,
        .dc_gpio_num = LCD_DC_PIN,
        .spi_mode = 0,
        .pclk_hz = 40 * 1000 * 1000, // 40MHz
        .trans_queue_depth = 10,
        .on_color_trans_done = nullptr,
        .user_ctx = nullptr,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .flags = {
            .dc_low_on_data = 0,
            .octal_mode = 0,
            .lsb_first = 0
        }
    };
    
    esp_lcd_panel_io_handle_t io_handle = nullptr;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_config, &io_handle));
    
    // Configuration du panel LCD générique
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_RST_PIN,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
        .bits_per_pixel = LCD_BIT_PER_PIXEL,
    };
    
    // Utiliser un driver LCD générique
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7796(io_handle, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, false));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, true, false));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
    
    return true;
}

bool DisplayDriver::configure_lvgl() {
    // Initialisation LVGL
    lv_init();
    
    // Allocation des buffers
    size_t buffer_size = LCD_WIDTH * 60; // 60 lignes de buffer
    buf1 = (lv_color_t*)malloc(buffer_size * sizeof(lv_color_t));
    buf2 = (lv_color_t*)malloc(buffer_size * sizeof(lv_color_t));
    
    if (!buf1 || !buf2) {
        ESP_LOGE(TAG, "Échec allocation buffers LVGL");
        return false;
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

void DisplayDriver::lvgl_flush_cb(lv_display_t* display, const lv_area_t* area, lv_color_t* color_map) {
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
    uint32_t duty = (brightness * 1023) / 100; // Convertir en valeur 10-bit
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));
    
    ESP_LOGI(TAG, "Luminosité réglée à %d%%", brightness);
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