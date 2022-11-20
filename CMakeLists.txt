cmake_minimum_required(
 VERSION 3.13
)

set(
 PICO_BOARD "pico_w"
)

include(
 pico_sdk_import.cmake
)

project(
 mqtt_client C CXX ASM
)

set(CMAKE_C_STANDARD 11)

set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(
 mqtt_client mqtt_client.c
)

target_compile_definitions(
 mqtt_client PRIVATE
 WIFI_SSID="hanabi2"
 WIFI_PASSWORD="UQurpM2Aln6gl89D5x8S3Q7q1L8HqcI0Gagx5KFLuDBrqvIa3bYoY0F622NFRTr" 
)

pico_enable_stdio_usb(mqtt_client 1)
pico_enable_stdio_uart(mqtt_client 1)

pico_add_extra_outputs( 
 mqtt_client
)

target_include_directories(
 mqtt_client PRIVATE ${CMAKE_CURRENT_LIST_DIR} 
)

target_link_libraries(
 mqtt_client
 pico_cyw43_arch_lwip_threadsafe_background
 pico_stdlib
)

target_sources(
 mqtt_client PRIVATE ${PICO_SDK_PATH}/lib/lwip/src/apps/mqtt/mqtt.c
)
