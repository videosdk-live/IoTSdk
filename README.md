# IoT SDK

At Video SDK, we're building tools to help developers bring **real-time collaboration** to IoT and embedded devices. With the IoT SDK, you can integrate **live audio and video communication, meeting management, device-to-cloud connectivity, and session handling** directly into ESP32-S3 boards.

## Features

- **Real-time audio** — publish the on-board microphone and subscribe to remote audio, using **PCMA (G.711 A-law)**.
- **Real-time video** — publish camera frames and render remote frames as hardware **JPEG** over the WebRTC data channel (Korvo-2).
- **Connection-state callbacks** — get notified when signaling connects or drops so your app can rejoin.
- **Meeting management** — create a room, join, and leave.
- **Runtime speaker volume** control (Korvo-2).

## Supported boards

| Board | Audio publish | Audio subscribe | Video publish | Video subscribe | Speaker volume |
|-------|:---:|:---:|:---:|:---:|:---:|
| **ESP32-S3-Korvo-2 v3.0** | ✅ | ✅ | ✅ | ✅ | ✅ |
| **XIAO ESP32-S3 (Sense)** | ✅ | ❌ | ✅ | ❌ | ❌ |

> The XIAO is **send-only** — it has no speaker or display, so `startSubscribeAudio()` and `startSubscribeVideo()` return `DEVICE_NOT_SUPPORTED`. Board selection is compile-time (see [Configure](#4-configure-menuconfig)).

## Prerequisites

- **ESP-IDF 5.4+** (required for the camera / JPEG stack).
- A valid [Video SDK account](https://app.videosdk.live/) and an auth token.

## Use the IoT SDK component

### 1. Set up ESP-IDF

Follow **Step 1** of the VideoSDK [ESP-IDF setup guide](https://docs.videosdk.live/iot/guide/video-and-audio-calling-api-sdk/quickstart/quick-start#step-1-setup-for-esp-idf) to install the toolchain. You do **not** need to run the project-creation commands — once the environment is ready, continue from Step 2 below.

```bash
# In every new shell, activate the ESP-IDF environment
source ~/esp/esp-idf/export.sh
```

### 2. Add the IoT SDK component

Declare the component in your project's `main/idf_component.yml`. Either reference the published component from the registry:

```yaml
dependencies:
  videosdk/iot-sdk: "*"   # or pin a specific version, e.g. "0.2.2"
```

### 3. Add the required dependencies

The component pulls in its own dependencies automatically, but your **application** also needs the shared IDF example/networking components. Add these to your `main/idf_component.yml`:

```yaml
dependencies:
  idf:
    version: ">=5.4.0"
  mdns: "*"
  espressif/esp_audio_codec: "~2.3.0"
  espressif/esp_codec_dev: "~1.3.4"
  espressif/esp_audio_effects: "~1.1.0"
  espressif/esp_capture: "^0.7.6"
  espressif/esp_video_codec: "~0.5.2"
  espressif/esp_jpeg: "^1.3.1"
  espressif/esp32-camera: "^2.0.15"
  espressif/esp_websocket_client: "^1.2.0"
  sepfy/srtp: "^2.3.0"
  sepfy/usrsctp: "^0.9.5"
```

### 4. Configure (menuconfig)

Set your board target, Wi-Fi, and VideoSDK credentials:

```bash
idf.py set-target esp32s3
idf.py menuconfig
```

- **VideoSDK IoT SDK → Audio hardware board** — select **ESP32-S3-Korvo-2** or **ESP32-S3-XIAO**.
- **VideoSDK Configuration**
  - **Auth token (JWT)** → `CONFIG_VIDEOSDK_TOKEN`
  - **Meeting / room ID** → `CONFIG_VIDEOSDK_MEETING_ID`
  - **Speaker output volume (0–100)** → `CONFIG_SPEAKER_VOLUME` (Korvo-2 only)
- **Example Connection Configuration** → Wi-Fi SSID and password.
- **Component config → mbedTLS** → enable **Support DTLS** and **Support TLS**.
- **Partition Table** → enable **Custom partition table CSV**.
- **Serial flasher config → Flash size** → set to match your board (e.g. 8 MB).

> The token and meeting ID are read from menuconfig (stored in `sdkconfig`) — **never hardcode a real token in source or commit it**.

### 5. Build & flash

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
        .meetingID   = CONFIG_VIDEOSDK_MEETING_ID,
        .token       = CONFIG_VIDEOSDK_TOKEN,
        .displayName = "ESP32-Device",
        .audioCodec  = AUDIO_CODEC_PCMA,   
        .videoCodec  = VIDEO_CODEC_JPEG,   
    };
    if (init(&cfg) != RESULT_OK) {
        return;
    }

    // 2. Join the call. startPublishAudio is the foundation — call it first;
    //    video/subscribe calls reuse its transport. (Korvo-2 for video + subscribe.)
    startPublishAudio("");   // empty publisherId => a random one is generated
    startPublishVideo();
    startSubscribeAudio();
    startSubscribeVideo();

    // 3. Let the SDK's internal tasks run.
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    // Later: leave() stops all publish/subscribe streams.
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
| `audioCodec` | `audio_codec_t` | `AUDIO_CODEC_PCMA` (G.711 A-law) — the only supported audio codec. |
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
| `void setConnectionStateHandler(connection_state_cb_t cb, void *user)` | Register/clear the signaling connection-state handler. |
| `result_t leave()` | Leave the meeting; stops all publish/subscribe streams. |

### Result codes

`RESULT_OK` (0) means success. Errors are in the `3001`–`3024` range (e.g. `DEVICE_NOT_SUPPORTED`, `AUDIO_CODEC_INIT_FAILED`, `DTLS_HANDSHAKE_FAILED`, `INIT_NOT_CALLED`, `DUPLICATE_ID`) — see the header for the full list and meanings.

### Contract & threading

- Call all functions from a **single task** (e.g. `app_main`) — the API is **not thread-safe**.
- Call order: `init()` → (optional) register callbacks → `start{Publish,Subscribe}{Audio,Video}()` → `leave()`.
- The first publish or subscribe call brings up its transport; later calls reuse it.
- Callbacks run on internal SDK tasks — **do not block** in them, and copy any buffer you need to keep.

## Documentation

- For more details, see the [VideoSDK Documentation](https://docs.videosdk.live/iot/guide/video-and-audio-calling-api-sdk/concept-and-architecture).
