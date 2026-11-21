idf_component_register(
    SRCS "hal_board.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_hw_support esp_system spi_flash heap esp_psram
)
