# IoT SDK

Official ESP32 SDK of [videosdk.live](https://videosdk.live).

Add live audio and video calling to ESP32-S3 boards. The device joins the same meeting as a phone or a browser. It sends whatever the microphone and camera pick up, and on a board with a speaker and a screen it plays back what the others are sending.

## Features

- Send audio and video from the on-board microphone and camera. Both boards can do this.
- Receive audio and video. This needs a speaker and a display, so Korvo-2 only.
- Send and receive text or binary messages during a call, on either board.
- Create a meeting, join it, and leave it.
- Switch the log verbosity between normal and debug at runtime.

Audio is sent as PCMA, PCMU, or Opus. Video is sent as JPEG.

## Supported boards

| Feature | XIAO ESP32-S3 (Sense) | ESP32-S3-Korvo-2 v3.0 |
|---------|:---:|:---:|
| Send audio | ✅ | ✅ |
| Receive audio | ❌ | ✅ |
| Send video | ✅ | ✅ |
| Receive video | ❌ | ✅ |
| Send & receive messages | ✅ | ✅ |

Both boards can send. Only the Korvo-2 can receive, since the XIAO has no speaker and no screen; there, `startSubscribeAudio()` and `startSubscribeVideo()` return `DEVICE_NOT_SUPPORTED`. The board is picked at build time (see [Configure](#3-configure-menuconfig)).

## Prerequisites

- **ESP-IDF 5.4+** 
- A valid [Video SDK Account](https://app.videosdk.live/)
- Python >= 3.11

## Use the IoT SDK component

### 1. Set up ESP-IDF

Follow **Step 1** of the VideoSDK [ESP-IDF setup guide](https://docs.videosdk.live/iot/guide/video-and-audio-calling-api-sdk/quickstart/quick-start#step-1-setup-for-esp-idf) to install the toolchain. You do not need to run the project-creation commands. Once the environment is ready, continue from Step 2 below.

```bash
# In every new shell, activate the ESP-IDF environment
source ~/esp/esp-idf/export.sh
```

### 2. Add the IoT SDK component

In your project's `main/idf_component.yml`, declare the component:

```yaml
dependencies:
  idf:
    version: '>=5.4.0'
  mdns: '*'
  protocol_examples_common:
    path: ${IDF_PATH}/examples/common_components/protocol_examples_common
  videosdk/iot-sdk:
    version: '*'
  # other components
```

Or add it from the terminal. You can pin a specific version, or use `*` to always pull the latest:

```bash
cd <your project path>

# Pin an exact version (recommended for reproducible builds)
idf.py add-dependency "videosdk/iot-sdk==0.3.0"

# Or always use the latest published version
idf.py add-dependency "videosdk/iot-sdk*"
```
### 3. Configure (menuconfig)

Set your board target, Wi-Fi, and VideoSDK credentials:

```
1. <!-- Run this command to set your board as the target -->
idf.py set-target esp32s3

2. <!-- Run this command to do menuconfig -->

idf.py menuconfig

         a. Inside Example Connection Configuration:
                |
                |———> WIFI SSID          <!-- replace it with your WiFi name -->
                |———> WIFI Password      <!-- replace it with your WiFi password -->
            And click S to save and again enter

         b. Inside VideoSDK Configuration:
                |
                |———> Auth token (JWT)    <!-- paste your VideoSDK token -->
                |———> Meeting / room ID   <!-- the meeting you want to join -->
            And click S to save and again enter

         c. Inside the Partition table:
                |
                |———> Partition table (custom partition table CSV)
                      |———> Enable Custom partition table CSV

         d. Adjust the flash size inside Serial flasher config
            (the examples ship an 8 MB config; the 4 MB factory app needs it)
                | ——> flash size: 8MB
            And click S to save and again enter

         e. Inside SET Microcontroller:
                        |——> Audio hardware board (example: ESP32-S3-Korvo-2)
                            |——> Select your board name
                                    |———> ESP32-S3-Korvo-2
                                    |———> ESP32-S3-XIAO   (default)
                        |——> Speaker output volume (0-100)   [Korvo-2 only]

         f. Inside VideoSDK Logging:
                |
                |———> Log verbosity   <!-- Normal (default) or Debug -->

Then press "S" to save and press "Enter" to confirm, then "Esc" or "q" to exit menuconfig.

```


### 4. Build & flash

```bash
idf.py build
idf.py -p <PORT> flash monitor
```

## Usage

Include the header and call the API from one task. It is not thread-safe.

```c
#include "videosdk.h"
#include "sdkconfig.h"

void app_main(void)
{
    // ... init NVS, netif, event loop, and connect Wi-Fi first ...

    // Optional. Follow the "VideoSDK Logging" menuconfig choice. Call before init().
#ifdef CONFIG_VIDEOSDK_LOG_MODE_DEBUG
    videosdk_set_log_mode(VIDEOSDK_LOG_DEBUG);
#endif

    init_config_t cfg = {
        .meetingID     = CONFIG_VIDEOSDK_MEETING_ID,
        .token         = CONFIG_VIDEOSDK_TOKEN,
        .displayName   = "ESP32S3-Device",  // user configurable
        .participantId = "",               // "" gets you a random id
        .audioCodec    = AUDIO_CODEC_PCMA, // or AUDIO_CODEC_PCMU / AUDIO_CODEC_OPUS
        .videoCodec    = VIDEO_CODEC_JPEG, // or VIDEO_CODEC_NONE for no video
    };
    if (init(&cfg) != RESULT_OK) {
        return;
    }

    // Start whichever directions you need, in any order. Each call returns a
    // result_t: RESULT_OK on success, or an error such as DEVICE_NOT_SUPPORTED
    // when the board can't do that direction.
    result_t result_publish_audio = startPublishAudio();
    printf("startPublishAudio: %d\n", result_publish_audio);

    result_t result_subscribe_audio = startSubscribeAudio();
    printf("startSubscribeAudio: %d\n", result_subscribe_audio);

    result_t result_publish_video = startPublishVideo();
    printf("startPublishVideo: %d\n", result_publish_video);

    result_t result_subscribe_video = startSubscribeVideo();
    printf("startSubscribeVideo: %d\n", result_subscribe_video);

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    // leave() shuts every direction down.
}
```

`init()` copies the strings in `init_config_t`, so you can free your own buffers as soon as it returns.

### Application messages

Text and binary messages travel alongside the media streams. Open the channel with `startMessageChannel()` before you call `sendMessage()`. To receive, register a handler.

```c
#include "videosdk.h"

// Runs on an SDK task. Don't block here, and copy anything you want to keep.
static void on_data_message(const uint8_t *data, size_t len, int is_binary, uint16_t sid)
{
    if (is_binary) {
        printf("binary message, %u bytes\n", (unsigned)len);
    } else {
        printf("text message: %.*s\n", (int)len, (const char *)data);
    }
}

// Optional. Fires when signaling comes up or goes away.
static void on_connection_state(bool connected, void *user)
{
    if (!connected) {
        // Session is gone. leave() and rejoin.
    }
}

void messaging_example(void)
{
    // Register handlers after init() but before the start* calls.
    setDataMessageHandler(on_data_message);
    setConnectionStateHandler(on_connection_state, NULL);

    if (startMessageChannel() != RESULT_OK) {
        return;
    }

    const char *text = "hello from esp32";
    sendMessage((const uint8_t *)text, strlen(text), /* is_binary */ 0);

    const uint8_t blob[4] = {0xDE, 0xAD, 0xBE, 0xEF};
    sendMessage(blob, sizeof(blob), /* is_binary */ 1);

    stopMessageChannel();
}
```

`sendMessage()` takes at most 24000 bytes per message. It returns `DATA_CHANNEL_NOT_STARTED` if the channel was never opened, or `DATA_CHANNEL_QUEUE_FULL` when the outgoing queue backs up. Passing `NULL` to either handler setter clears it.

## API reference

All declarations live in [`include/videosdk.h`](include/videosdk.h).

### `init_config_t`

| Field | Type | Notes |
|-------|------|-------|
| `meetingID` | `char *` | The meeting ID to join.|
| `token` | `char *` | VideoSDK JWT auth token. |
| `displayName` | `char *` | Name shown in the meeting. |
| `participantId` | `char *` | This device's peer id. `""` or `NULL` gives you a random 8-char id. |
| `audioCodec` | `audio_codec_t` | `AUDIO_CODEC_PCMA`, `AUDIO_CODEC_PCMU`, or `AUDIO_CODEC_OPUS`. |
| `videoCodec` | `video_codec_t` | `VIDEO_CODEC_NONE` (no video) or `VIDEO_CODEC_JPEG`. |

### Functions

| Function | Description |
|----------|-------------|
| `create_meeting_result_t create_meeting(char *token)` | Create a meeting. Returns a malloc'd `room_id` that the caller frees. |
| `void videosdk_set_log_mode(videosdk_log_mode_t mode)` | Set log verbosity (`VIDEOSDK_LOG_NORMAL` or `VIDEOSDK_LOG_DEBUG`). Call before `init()`. |
| `result_t init(init_config_t *cfg)` | Initialize the session and the board. Call once, before anything else. |
| `result_t startPublishAudio(void)` | Capture the microphone and send it to the meeting. |
| `result_t startPublishVideo(void)` | Capture the camera and send it to the meeting. |
| `result_t startSubscribeAudio(void)` | Receive remote audio and play it on the speaker. Korvo-2 only. |
| `result_t startSubscribeVideo(void)` | Receive remote video and show it on the display. Korvo-2 only. |
| `result_t stopPublishAudio()` | Stop publishing audio. |
| `result_t stopSubscribeAudio()` | Stop subscribing to audio. |
| `void setSpeakerVolume(int volume)` | Set playback volume, 0 to 100 (out-of-range values are clamped). Korvo-2 only. |
| `result_t startMessageChannel(void)` | Open the message channel. Needed before `sendMessage()`. |
| `result_t sendMessage(const uint8_t *data, size_t len, int is_binary)` | Send a text or binary message. `len` is capped at 24000 bytes. |
| `result_t stopMessageChannel(void)` | Close the message channel. |
| `void setDataMessageHandler(data_message_cb_t cb)` | Set the incoming-message handler. `NULL` clears it. |
| `void setConnectionStateHandler(connection_state_cb_t cb, void *user)` | Set the connection-state handler. `NULL` clears it. |
| `result_t leave()` | Leave the meeting and stop every direction that was started. |

Directions are independent, so start them in any combination and any order. Callbacks run on SDK tasks, so don't block in them, and copy any buffer you want to keep.

### Result codes

`RESULT_OK` (0) means success. Errors fall in the `3001` to `3026` range, for example `DEVICE_NOT_SUPPORTED`, `AUDIO_CODEC_INIT_FAILED`, `DTLS_HANDSHAKE_FAILED`, `INIT_NOT_CALLED`, `DUPLICATE_ID`, `DATA_CHANNEL_NOT_STARTED` and `DATA_CHANNEL_QUEUE_FULL`. The header lists them all.


## Documentation

- For more details, see the [VideoSDK Documentation](https://docs.videosdk.live/iot/guide/video-and-audio-calling-api-sdk/concept-and-architecture).
