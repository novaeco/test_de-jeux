# Configuration LVGL pour ESP-IDF 5.5
set(LVGL_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Recherche de tous les fichiers sources LVGL
file(GLOB_RECURSE SOURCES 
    "${LVGL_ROOT_DIR}/src/*.c"
    "${LVGL_ROOT_DIR}/*.c"
)

# Exclusion des sources de test et démonstration
list(FILTER SOURCES EXCLUDE REGEX ".*/(tests?|examples?|demos?)/.*")

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS 
        "."
        "src"
    REQUIRES 
        driver
        esp_timer
        esp_lcd
)

# Configuration des définitions pour LVGL
target_compile_definitions(${COMPONENT_LIB} PUBLIC
    LV_CONF_PATH="${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h"
    LV_CONF_INCLUDE_SIMPLE=1
)