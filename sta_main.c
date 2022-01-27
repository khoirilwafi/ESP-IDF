// masukkan library C# standar (opsional)
#include <string.h>
#include <stdio.h>

// masukkan freeRTOS untuk fungsi delay
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// masukkan wifi driver
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_log.h"

// masukkan nsv flash untuk menyimpan config wifi
#include "nvs_flash.h"

// log tag (opsional)
static const char* tag = "wifi_sta";

// konfigurasi ssid dan password
#define SSID "ESP32 Streaming"
#define PASS "12345678"

// konfigurasi wifi event handler
static void wifi_sta_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    // handler wifi
    if (event_base == WIFI_EVENT)
    {
        // driver wifi berhasil diinstall
        if (event_id == WIFI_EVENT_STA_START)
        {
            ESP_LOGW(tag, "wifi start, trying connect to %s", SSID);
            esp_wifi_connect();
        }

        // koneksi wifi terputus / gagal
        else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
        {
            ESP_LOGE(tag, "connection failed, wait to re-connecting ...");
            vTaskDelay(500/portTICK_PERIOD_MS);
            esp_wifi_connect();
        }
    }

    // wifi mendapatkan alamat IP
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t* the_ip = (ip_event_got_ip_t*) event_data;

        ESP_LOGW(tag, "connected to %s", SSID);
        ESP_LOGW(tag, "IP : " IPSTR, IP2STR(&the_ip->ip_info.ip) );
    }
}

// fungsi koneksi wifi
static void wifi_sta_init(void)
{
    // aktifkan wifi event dan tcp_ip driver
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    // mulai wifi dengan config standar
    wifi_init_config_t wifi_init = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_init);

    // daftarkan setiap event yang akan dijalankan
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_sta_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, &wifi_sta_handler, NULL, NULL);

    // parameter wifi
    wifi_config_t wifi_config =
    {
        .sta = 
        {
            .ssid = SSID,
            .password = PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            
            .pmf_cfg = 
            {
                .capable = true,
                .required = false
            },
        },
    };

    // konfigurasi driver (mode, config)
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);

    // mulai koneksi wifi
    esp_wifi_start();
}

// fungsi utama program
void app_main(void)
{
    // aktifkan nvs
    nvs_flash_init();

    // koneksi wifi
    wifi_sta_init();
}
