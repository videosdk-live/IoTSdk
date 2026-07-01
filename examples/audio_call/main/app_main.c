#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/time.h>
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "esp_partition.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "mdns.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include "sdkconfig.h"
#include "videosdk.h"

static const char *TAG = "IOT-SDK-AUDIO";
// Token and meeting ID come from menuconfig (VideoSDK Configuration), stored in
// sdkconfig -- never hardcode a real token in source.
const char *token = CONFIG_VIDEOSDK_TOKEN;

void app_main(void)
{
  static char deviceid[32] = {0};
  uint8_t mac[8] = {0};

  ESP_LOGI(TAG, "[APP] Startup..");
  ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
  ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

  esp_log_level_set("*", ESP_LOG_INFO);

  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  ESP_ERROR_CHECK(example_connect());

  if (esp_read_mac(mac, ESP_MAC_WIFI_STA) == ESP_OK)
  {
    sprintf(deviceid, "esp32-%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    ESP_LOGI(TAG, "Device ID: %s", deviceid);
  }

  if (token[0] == '\0' || CONFIG_VIDEOSDK_MEETING_ID[0] == '\0')
  {
    ESP_LOGE(TAG, "VideoSDK token/meeting not set. Run 'idf.py menuconfig' -> "
                  "VideoSDK Configuration.");
    return;
  }

  // Audio-only session: VIDEO_CODEC_NONE, no video start calls.
  init_config_t init_cfg = {
      .meetingID = CONFIG_VIDEOSDK_MEETING_ID,
      .token = token,
      .displayName = "ESP32-Audio",
      .audioCodec = AUDIO_CODEC_PCMA,
      .videoCodec = VIDEO_CODEC_NONE,
  };

  result_t init_result = init(&init_cfg);
  ESP_LOGI(TAG, "init: %d", init_result);
  if (init_result != RESULT_OK)
  {
    return;
  }

  // Publish the microphone into the meeting, and play back remote audio.
  // startSubscribeAudio drives the ES8311 speaker and is Korvo-2 only -- on the
  // XIAO (no speaker) it returns DEVICE_NOT_SUPPORTED, which is fine.
  startPublishAudio(""); // empty publisherId => a random one is generated
  startSubscribeAudio();

  while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
