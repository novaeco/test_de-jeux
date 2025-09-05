#include "include/display_driver.h"
#include "driver/spi_master.h"
#include "driver/i2c.h"
#include "driver/ledc.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_touch_gt911.h"  // Contrôleur tactile GT911 couramment utilisé
#include "esp_log.h"

static const char* TAG = "DisplayDriver";

// Buffers LVGL statiques
lv_disp_draw_buf_t DisplayDriver::draw_buf;
lv_color_t DisplayDriver::buf1[LCD_WIDTH * 60];
lv_color_t DisplayDriver::buf2[LCD_WIDTH * 60];

DisplayDriver::DisplayDriver() : panel_handle(nullptr), touch_handle(nullptr) {
}

DisplayDriver::~DisplayDriver() {
    if (panel_handle) {
        esp_lcd_panel_del(panel_handle);
    }
    if (touch_handle) {
        esp_lcd_touch_del(touch_handle);
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
    
    if (!configure_touch_interface()) {
        ESP_LOGE(TAG, "Échec configuration tactile");
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
        .max_transfer_sz = LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t)
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
    
    // Configuration du panel LCD (ST7796 ou similaire pour écran 7")
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_RST_PIN,
        .rgb_endian = LCD_RGB_ENDIAN_BGR,
        .bits_per_pixel = LCD_BIT_PER_PIXEL,
    };
    
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7796(io_handle, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, false));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, true, false));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
    
    return true;
}

bool DisplayDriver::configure_touch_interface() {
    // Configuration I2C pour le contrôleur tactile
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = TOUCH_SDA_PIN,
        .scl_io_num = TOUCH_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master = {
            .clk_speed = 400000, // 400kHz
        },
        .clk_flags = 0,
    };
    
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &i2c_conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, i2c_conf.mode, 0, 0, 0));
    
    // Configuration du contrôleur tactile GT911
    esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();
    tp_io_config.scl_speed_hz = 400000;
    
    esp_lcd_panel_io_handle_t tp_io_handle = nullptr;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)I2C_NUM_0, &tp_io_config, &tp_io_handle));
    
    esp_lcd_touch_config_t tp_cfg = {
        .x_max = LCD_WIDTH,
        .y_max = LCD_HEIGHT,
        .rst_gpio_num = TOUCH_RST_PIN,
        .int_gpio_num = TOUCH_INT_PIN,
        .flags = {
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0,
        },
    };
    
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_gt911(tp_io_handle, &tp_cfg, &touch_handle));
    
    return true;
}

bool DisplayDriver::configure_lvgl() {
    // Initialisation LVGL
    lv_init();
    
    // Configuration des buffers de rendu
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, LCD_WIDTH * 60);
    
    // Configuration du pilote d'affichage LVGL
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LCD_WIDTH;
    disp_drv.ver_res = LCD_HEIGHT;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.user_data = this;
    
    lv_disp_t* disp = lv_disp_drv_register(&disp_drv);
    
    // Configuration du pilote tactile LVGL
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = lvgl_touch_cb;
    indev_drv.user_data = this;
    
    lv_indev_drv_register(&indev_drv);
    
    return true;
}

void DisplayDriver::lvgl_flush_cb(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* color_map) {
    DisplayDriver* display = static_cast<DisplayDriver*>(drv->user_data);
    
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    
    esp_lcd_panel_draw_bitmap(display->panel_handle, offsetx1, offsety1, 
                             offsetx2 + 1, offsety2 + 1, color_map);
    
    lv_disp_flush_ready(drv);
}

void DisplayDriver::lvgl_touch_cb(lv_indev_drv_t* drv, lv_indev_data_t* data) {
    DisplayDriver* display = static_cast<DisplayDriver*>(drv->user_data);
    
    uint16_t touch_x[1], touch_y[1];
    uint8_t touch_cnt = 0;
    
    bool touched = esp_lcd_touch_read_data(display->touch_handle) == ESP_OK;
    
    if (touched) {
        bool pressed = esp_lcd_touch_get_coordinates(display->touch_handle, touch_x, touch_y, NULL, &touch_cnt, 1);
        
        if (pressed && touch_cnt > 0) {
            data->point.x = touch_x[0];
            data->point.y = touch_y[0];
            data->state = LV_INDEV_STATE_PRESSED;
            
            ESP_LOGD(TAG, "Touch: x=%d, y=%d", touch_x[0], touch_y[0]);
        } else {
            data->state = LV_INDEV_STATE_RELEASED;
        }
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
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