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

static const char *TAG = "IOT-SDK-VIDEO";
// Set these under "VideoSDK Configuration" in menuconfig. They land in
// sdkconfig, so a real token never has to sit in source.
const char *token = CONFIG_VIDEOSDK_TOKEN;

void app_main(void)
{
  static char deviceid[32] = {0};
  uint8_t mac[8] = {0};

  ESP_LOGI(TAG, "[APP] Startup..");
  ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
  ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

  // Log verbosity, chosen in menuconfig -> "VideoSDK Logging". Only the SDK's own
  // tags move; ESP-IDF / registry components keep their levels. Call before
  // init() so it covers the join.
#if CONFIG_VIDEOSDK_LOG_MODE_DEBUG
  videosdk_set_log_mode(VIDEOSDK_LOG_DEBUG);
#else
  videosdk_set_log_mode(VIDEOSDK_LOG_NORMAL);
#endif

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

  // Video only. audioCodec is still set, it just goes unused here because no
  // audio direction is started.
  init_config_t init_cfg = {
      .meetingID = CONFIG_VIDEOSDK_MEETING_ID,
      .token = token,
      .displayName = "ESP32S3-Video", // any name you like; shown in the meeting
      .participantId = deviceid,      // this device's id
      .audioCodec = AUDIO_CODEC_PCMA,
      .videoCodec = VIDEO_CODEC_JPEG,
  };

  result_t init_result = init(&init_cfg);
  ESP_LOGI(TAG, "init: %d", init_result);
  if (init_result != RESULT_OK)
  {
    return;
  }

  // Both boards can publish. startSubscribeVideo needs the LCD, so on the XIAO
  // it comes back with DEVICE_NOT_SUPPORTED. That's expected.
  result_t result_publish = startPublishVideo();
  printf("Result:%d\n", result_publish);
  result_t result_subscribe = startSubscribeVideo();
  printf("Result:%d\n", result_subscribe);

  // Keep the session active for a defined duration (adjust as per your application use case)
  vTaskDelay(pdMS_TO_TICKS(100000));

  // Leave the meeting
  result_t result_leave = leave();
  printf("Result:%d\n", result_leave);

  while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}