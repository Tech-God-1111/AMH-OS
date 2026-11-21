idf_component_register(
    SRCS "kernel_boot.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_timer esp_system log freertos
)
