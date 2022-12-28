#include <string.h>
#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"                                                                                  
#include "lwip/apps/mqtt.h"
#include "hardware/gpio.h"

#define LWIP_MQTT_EXAMPLE_IPADDR_INIT = IPADDR4_INIT(LWIP_MAKEU32(48,0,168,192))

static ip_addr_t mqtt_ip LWIP_MQTT_EXAMPLE_IPADDR_INIT;
static mqtt_client_t* client;

const uint DHT_PIN = 28;
const uint MAX_TIMINGS = 85;
typedef struct {
 float humidity;
 float temp_celsius;
} dht_reading;

void read_from_dht(dht_reading *result);

void read_from_dht(dht_reading *result) {
 int data[5] = {0, 0, 0, 0, 0};
 uint last = 1;
 uint j = 0;
 gpio_set_dir(DHT_PIN, GPIO_OUT);
 gpio_put(DHT_PIN, 0);
 sleep_ms(20);
 gpio_set_dir(DHT_PIN, GPIO_IN);

 for (uint i = 0; i < MAX_TIMINGS; i++) {
  uint count = 0;
  while (gpio_get(DHT_PIN) == last) {
   count++;
   sleep_us(1);
   if (count == 255) break;
  }
  last = gpio_get(DHT_PIN);
  if (count == 255) break;
  if ((i >= 4) && (i % 2 == 0)) {
   data[j / 8] <<= 1;
   if (count > 16) data[j / 8] |= 1;
   j++;
  }
 }

 if ((j >= 40) && (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF))) {
  result->humidity = (float) ((data[0] << 8) + data[1]) / 10;
  if (result->humidity > 100) {
   result->humidity = data[0];
  }
  result->temp_celsius = (float) (((data[2] & 0x7F) << 8) + data[3]) / 10;
  if (result->temp_celsius > 125) {
   result->temp_celsius = data[2];
  }
  if (data[2] & 0x80) {
   result->temp_celsius = -result->temp_celsius;
  }
 } 
 else {
  printf("Bad data\n"); 
 }
}

static struct mqtt_connect_client_info_t mqtt_client_info = {
  "test", // topic
  NULL,   // user
  NULL,   // pass
  100,    // keep alive
  NULL,   // will_topic
  NULL,   // will_msg
  0,      // will_qos
  0       // will_retain
};

static void mqttPubCb(void *arg, err_t err) {
  printf("mqttPubCb() called.\n");
}

static void mqttConnCb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
  printf("mqttConnCb() called.\n");
}

void mqttConnect(mqtt_client_t *client) {
  printf("mqttConnect() called.\n");
  mqtt_client_connect(client, &mqtt_ip, MQTT_PORT, mqttConnCb, &mqtt_client_info, &mqtt_client_info);
}

int mqttPublish(mqtt_client_t *client, char *msg) {
  err_t err;
  u8_t qos = 2;
  u8_t retain = 0;
  err = mqtt_publish(client, "pub_topic", msg, strlen(msg), 0, retain, mqttPubCb, &mqtt_client_info);
  if(err != ERR_OK) {
    return 1;
    printf("Publish err: %d\n", err);
  }
  return 0;
}

int main() {
 stdio_init_all();
 gpio_init(DHT_PIN);
 
 if(cyw43_arch_init_with_country(CYW43_COUNTRY_GERMANY)) {
  printf("Failed to initialise cyw43_arch\n");
 }
 cyw43_arch_enable_sta_mode();
 if(cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
  printf("Failed to connect to WiFi\n");
 }
 mqtt_client_t *client = mqtt_client_new();
 if(client != NULL) {
  mqttConnect(client);
 }
 else {
  printf("Failed to allocate mqtt client structure.\n");
 }

 int iterations = 0;
 while(true) {
  dht_reading reading;
  read_from_dht(&reading);
  float fahrenheit = (reading.temp_celsius * 9 / 5) + 32;
  printf("Humidity = %.1f%%, Temperature = %.1fC (%.1fF)\n", reading.humidity, reading.temp_celsius, fahrenheit);
  char str[1024] = "";
  sprintf(str, "%d", iterations);
  if(mqttPublish(client,str) == 0) {
   iterations++;
  }
  else {
   printf("Skipped iteration, save flash, send later!\n");
  }
  sleep_ms(2000);
  
 }
 return 0;
}
