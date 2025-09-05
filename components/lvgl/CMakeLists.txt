# Configuration LVGL comme composant externe

idf_component_register(
    INCLUDE_DIRS 
        "."
        "src"
    SRCS 
        "lvgl.c"
    REQUIRES 
        driver
        esp_timer
)

# Définir les macros de configuration LVGL
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    LV_CONF_PATH="${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h"
    LV_COLOR_DEPTH=16
    LV_COLOR_16_SWAP=1
    LV_MEM_SIZE=65536
    LV_DISP_DEF_REFR_PERIOD=33
    LV_INDEV_DEF_READ_PERIOD=10
    LV_USE_PERF_MONITOR=1
    LV_USE_MEM_MONITOR=1
)