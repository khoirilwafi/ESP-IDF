// masukkan lib standar
#include <string.h>

// masukkan lib esp32
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

// debug label (opsional)
static const char* tag = "wifi_ap";

// nama dan password wifi
#define ESP_WIFI_SSID   "esp32_wifi"
#define ESP_WIFI_PASS   "asdfghjkl"

// log all station 
void wifi_ap_sta_list (void)
{
    wifi_sta_list_t sta_list;
    esp_netif_sta_list_t netif_list;

    esp_wifi_ap_get_sta_list(&sta_list);
    esp_netif_get_sta_list(&sta_list, &netif_list);

    ESP_LOGW(tag, "number of statio : %d", sta_list.num);
    for(uint32_t i=0; i<sta_list.num; i++)
    {
        esp_ip4_addr_t ip_address = netif_list.sta[i].ip;

        char ip_str[16];
        esp_ip4addr_ntoa(&ip_address, ip_str, 16);

        ESP_LOGW(tag, "sta %d : %s ", i+1, ip_str);    
    }
}

// event handler 
static void wifi_event_handler (void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    // event wifi
    if (event_base == WIFI_EVENT)
    {
        // sta baru terhubung
        if (event_id == WIFI_EVENT_AP_STACONNECTED) 
        {
            wifi_event_ap_staconnected_t* ap_event = (wifi_event_ap_staconnected_t*) event_data;
            ESP_LOGW(tag, "station "MACSTR" connected", MAC2STR(ap_event->mac));
        } 

        // ada sta yang terputus
        else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) 
        {
            wifi_event_ap_stadisconnected_t* ap_event = (wifi_event_ap_stadisconnected_t*) event_data;
            ESP_LOGW(tag, "station "MACSTR" leave ", MAC2STR(ap_event->mac));
        } 

        // akses point berhasil di jalankan
        else if (event_id == WIFI_EVENT_AP_START)
        {
            ESP_LOGW(tag, "akses point ready, waiting client to connect ...");
        }
    }
    
    // event ip
    else if (event_base == IP_EVENT && event_id == IP_EVENT_AP_STAIPASSIGNED)
    {
        wifi_ap_sta_list();
    }
}

// fungsi inisialisasi wifi ap
void wifi_sta_init (void)
{
    // konfigurasi tcp_ip
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_ap();

    // init wifi
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&config);

    // daftarkan event handler
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);

    // konfig wifi
    wifi_config_t wifi_config =
    {
        .ap = {
            .ssid = ESP_WIFI_SSID,
            .password = ESP_WIFI_PASS,
            .channel = 1,
            .max_connection = 5,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .ssid_hidden = 0
        },
    };

    // konfigurasi wifi ap
    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    esp_wifi_start();
}


void app_main (void)
{
    nvs_flash_init();
    wifi_sta_init();
}
