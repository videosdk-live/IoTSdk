# IoT SDK

At Video SDK, we're building tools to help developers bring **real-time collaboration** to IoT and embedded devices. With the IoT SDK, you can integrate **live audio and video communication, meeting management, device-to-cloud connectivity, and session handling** directly into ESP32-S3 boards.

## Features

- **Real-time audio** — publish the on-board microphone and subscribe to remote audio, using **PCMA (G.711 A-law)**.
- **Real-time video** — publish camera frames and render remote frames as hardware **JPEG** over the WebRTC data channel (Korvo-2).
- **Meeting management** — create a room, join, and leave.

## Supported boards

| Board | Audio publish | Audio subscribe | Video publish | Video subscribe | Speaker volume |
|-------|:---:|:---:|:---:|:---:|:---:|
| **ESP32-S3-Korvo-2 v3.0** | ✅ | ✅ | ✅ | ✅ | ✅ |
| **XIAO ESP32-S3 (Sense)** | ✅ | ❌ | ✅ | ❌ | ❌ |

> The XIAO is **send-only** — it has no speaker or display, so `startSubscribeAudio()` and `startSubscribeVideo()` return `DEVICE_NOT_SUPPORTED`. Board selection is compile-time (see [Configure](#4-configure-menuconfig)).

## Prerequisites

- **ESP-IDF 5.4+** 
- A valid [Video SDK Account](https://app.videosdk.live/)
- Python >= 3.11

## Use the IoT SDK component

### 1. Set up ESP-IDF

Follow **Step 1** of the VideoSDK [ESP-IDF setup guide](https://docs.videosdk.live/iot/guide/video-and-audio-calling-api-sdk/quickstart/quick-start#step-1-setup-for-esp-idf) to install the toolchain. You do **not** need to run the project-creation commands — once the environment is ready, continue from Step 2 below.

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
idf.py add-dependency "videosdk/iot-sdk==0.2.3"

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

         a. Inside the Component config:
                |
                |———> mbedtls
                      | ——> Enable Support DTLS    <!-- It enables 3 way handshake -->
                      | ——> Enable Support TLS     <!-- It enables 3 way handshake -->
            And click S to save and again enter

         b. Inside Example Connection Configuration:
                |
                |———> WIFI SSID          <!-- replace it with your WiFi name -->
                |———> WIFI Password      <!-- replace it with your WiFi password -->
            And click S to save and again enter

         c. Inside VideoSDK Configuration:
                |
                |———> Auth token (JWT)    <!-- paste your VideoSDK token -->
                |———> Meeting / room ID   <!-- the room you want to join -->
            And click S to save and again enter

         d. Inside the Partition table:
                |
                |———> Partition table (custom partition table CSV)
                      |———> Enable Custom partition table CSV

         e. Adjust the flash size inside Serial flasher config
            (the examples ship an 8 MB config — the 4 MB factory app needs it)
                | ——> flash size: 8MB
            And click S to save and again enter

         f. Inside SET Microcontroller:
                        |——> Audio hardware board (example: ESP32-S3-Korvo-2)
                            |——> Select your board name
                                    |———> ESP32-S3-Korvo-2
                                    |———> ESP32-S3-XIAO   (default)
                        |——> Speaker output volume (0-100)   [Korvo-2 only]

Then press "S" to save and press "Enter" to confirm, then "Esc" or "q" to exit menuconfig.

```


### 4. Build & flash

```bash
idf.py build
idf.py -p <PORT> flash monitor
```

## Usage

Include the single public header and drive the API from **one task** (it is not thread-safe):

```c
#include "videosdk.h"
#include "sdkconfig.h"

void app_main(void)
{
    // ... init NVS, netif, event loop, and connect Wi-Fi first ...

    // 1. Initialize the session (call exactly once, first).
    init_config_t cfg = {
        .meetingID   = CONFIG_VIDEOSDK_MEETING_ID,  // your VideoSDK Meeting Id
        .token       = CONFIG_VIDEOSDK_TOKEN,   // your VideoSDK Token
        .displayName = "ESP32-Device", // You can use any custom name also
        .audioCodec  = AUDIO_CODEC_PCMA,   
        .videoCodec  = VIDEO_CODEC_JPEG,   
    };
    if (init(&cfg) != RESULT_OK) {
        return;
    }

    // 2. Join the call. startPublishAudio is the foundation — call it first;

    // use startPublishAudio to capture and send audio to VideoSDK meeting room
    startPublishAudio("");   // empty publisherId => a random one is generated
    // use startSubscribeAudio to receive audio from VideoSDK meeting room
    startSubscribeAudio();
    // use startPublishVideo to capture and send video to VideoSDK meeting room
    startPublishVideo();
    // use startSubscribeVideo to receive video from VideoSDK meeting room
    startSubscribeVideo();

    // 3. Let the SDK's internal tasks run.
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    // Later: call leave(); stops all publish/subscribe streams.
}


```

## API reference

All declarations live in [`include/videosdk.h`](include/videosdk.h).

### `init_config_t`

| Field | Type | Notes |
|-------|------|-------|
| `meetingID` | `char *` | Room / meeting ID. **Not copied — keep alive for the whole session.** |
| `token` | `char *` | VideoSDK JWT auth token (same lifetime note). |
| `displayName` | `char *` | Name shown in the meeting. |
| `audioCodec` | `audio_codec_t` | `AUDIO_CODEC_PCMA` (G.711 A-law)  |
| `videoCodec` | `video_codec_t` | `VIDEO_CODEC_NONE` (audio only) or `VIDEO_CODEC_JPEG`. |

### Functions

| Function | Description |
|----------|-------------|
| `create_meeting_result_t create_meeting(char *token)` | Create a room. `room_id` is malloc'd — **caller must `free()` it**. |
| `result_t init(init_config_t *cfg)` | Initialize the session and board. Call **once, first**. |
| `result_t startPublishAudio(char *publisherId)` | Mic → data channel. Foundation call; empty `publisherId` = random. |
| `result_t startPublishVideo(void)` | Camera JPEG → data channel. Call **after** `startPublishAudio`. *Korvo-2 only.* |
| `result_t startSubscribeAudio(void)` | Remote audio → speaker. Call **after** `startPublishAudio`. *Korvo-2 only.* |
| `result_t startSubscribeVideo(void)` | Remote JPEG → LCD. Call **after** `startSubscribeAudio`. *Korvo-2 only.* |
| `result_t stopPublishAudio()` | Stop publishing audio. |
| `result_t stopSubscribeAudio()` | Stop subscribing to audio. |
| `void setSpeakerVolume(int volume)` | Set playback volume 0–100 (clamped). *Korvo-2 only.* |
| `result_t leave()` | Leave the meeting; stops all publish/subscribe streams. |

### Result codes

`RESULT_OK` (0) means success. Errors are in the `3001`–`3024` range (e.g. `DEVICE_NOT_SUPPORTED`, `AUDIO_CODEC_INIT_FAILED`, `DTLS_HANDSHAKE_FAILED`, `INIT_NOT_CALLED`, `DUPLICATE_ID`) — see the header for the full list and meanings.


## Documentation

- For more details, see the [VideoSDK Documentation](https://docs.videosdk.live/iot/guide/video-and-audio-calling-api-sdk/concept-and-architecture).
