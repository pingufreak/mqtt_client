#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"                                                                                  
#include "lwip/apps/mqtt.h"

#define LWIP_MQTT_EXAMPLE_IPADDR_INIT = IPADDR4_INIT(LWIP_MAKEU32(48,0,168,192))

static ip_addr_t mqtt_ip LWIP_MQTT_EXAMPLE_IPADDR_INIT;
static mqtt_client_t* client;

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

void mqttPublish(mqtt_client_t *client, char *msg) {
  err_t err;
  u8_t qos = 2;
  u8_t retain = 0;
  err = mqtt_publish(client, "pub_topic", msg, strlen(msg), 0, retain, mqttPubCb, &mqtt_client_info);
  if(err != ERR_OK) {
    printf("Publish err: %d\n", err);
  }
}

int main() {
 stdio_init_all();
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

 while(true) {
  mqttPublish(client, "test");
  printf("loop\n");
  sleep_ms(5000);
 }
 return 0;
}
