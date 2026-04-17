idf_component_register(
    SRCS
        "app_main.c"
        "app_state.c"
        "battery.c"
        "button.c"
        "leds.c"
        "rpc.c"
        "storage.c"
        "ota_pull.c"
        "web_server.c"
    INCLUDE_DIRS "include"
)

spiffs_create_partition_image(storage web FLASH_IN_PROJECT)
