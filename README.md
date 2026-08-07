# IoT SDK

Official ESP32 SDK of [videosdk.live](https://videosdk.live).

Add live audio and video calling to ESP32-S3 boards. The device joins the same room as a phone or a browser. It sends whatever the microphone and camera pick up, and on a board with a speaker and a screen it plays back what the others are sending.

## Features

- Send audio and video from the on-board microphone and camera. Both boards can do this.
- Receive audio and video. This needs a speaker and a display, so Korvo-2 only.
- Send and receive text or binary messages during a call, on either board.
- Create a room, join it, and leave it.
- Switch the log verbosity between normal and debug at runtime.

Audio is sent as PCMA, PCMU, or Opus. Video is sent as JPEG.

## Supported boards

| Board | Send audio | Receive audio | Send video | Receive video | Send & receive messages |
|-------|:---:|:---:|:---:|:---:|:---:|
| [XIAO ESP32-S3 (Sense)](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/) | ✅ | ❌ | ✅ | ❌ | ✅ |
| [ESP32-S3-Korvo-2 v3.0](https://docs.espressif.com/projects/esp-adf/en/latest/design-guide/dev-boards/user-guide-esp32-s3-korvo-2.html) | ✅ | ✅ | ✅ | ✅ | ✅ |

Both boards can send. Only the Korvo-2 can receive, since the XIAO has no speaker and no screen; there, `startSubscribeAudio()` and `startSubscribeVideo()` return `DEVICE_NOT_SUPPORTED`. The board is picked at build time (see [Configure](#3-configure-menuconfig)).

## Prerequisites

- A supported **ESP32-S3** board: **[Seeed XIAO ESP32-S3 (Sense)](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/)** or **[ESP32-S3-Korvo-2 v3.0](https://docs.espressif.com/projects/esp-adf/en/latest/design-guide/dev-boards/user-guide-esp32-s3-korvo-2.html)**
- ESP-IDF 5.4.2, 5.4.3, or 5.4.4
- A [VideoSDK account](https://app.videosdk.live/) and an auth token
- Python 3.11 or newer (required by ESP-IDF)

## Use the IoT SDK component

### 1. Set up ESP-IDF

Follow **Step 1** of the VideoSDK [ESP-IDF setup guide](https://docs.videosdk.live/iot/guide/video-and-audio-calling-api-sdk/quickstart/quick-start#step-1-setup-for-esp-idf) to install the toolchain. You do not need to run the project-creation commands. Once the environment is ready, continue from Step 2 below.

```bash
# In every new shell, activate the ESP-IDF environment
source ~/esp/esp-idf/export.sh
```

### 2. Add the IoT SDK component

Add it to your project's `main/idf_component.yml`:

```yaml
dependencies:
  videosdk/iot-sdk: "^0.4.0"   # 0.4.0 or newer
  idf:
    version: ">=5.4.2,<=5.4.4"

  # Wi-Fi helper used by the sample code below. Skip it if your app
  # already connects to Wi-Fi its own way.
  protocol_examples_common:
    path: ${IDF_PATH}/examples/common_components/protocol_examples_common
```

Or add it from the terminal:

```bash
cd <your project path>

idf.py add-dependency "videosdk/iot-sdk==0.4.0"    # use this to pin exact version
idf.py add-dependency "videosdk/iot-sdk"   # use this to pull the latest version
```
### 3. Configure (menuconfig)

```bash
idf.py set-target esp32s3
idf.py menuconfig
```

Then set the following, and press `S` to save and `Q` to exit:

- **SET Microcontroller → Audio hardware board** — pick `ESP32-S3-Korvo-2` or `ESP32-S3-XIAO` (default). On the Korvo-2 you can also set the speaker volume.
- **VideoSDK Configuration** — paste your Auth token (JWT) and the Room ID.
- **Example Connection Configuration** — your Wi-Fi SSID and password.
- **Partition Table** — choose "Custom partition table CSV".
- **Serial flasher config → Flash size** — 8 MB (the 4 MB app doesn't fit in less).
- **VideoSDK Logging** — Normal (default) or Debug.

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
        .roomId        = CONFIG_VIDEOSDK_ROOM_ID,
        .token         = CONFIG_VIDEOSDK_TOKEN,
        .displayName   = "ESP32S3-Device", // any name you like; shown in the room
        .participantId = "",               // "" gets you a random id
        .audioCodec    = AUDIO_CODEC_PCMA, // or AUDIO_CODEC_PCMU / AUDIO_CODEC_OPUS
        .videoCodec    = VIDEO_CODEC_JPEG, // or VIDEO_CODEC_NONE for no video

        // Optional. Leave it NULL to use "api.videosdk.live".
        .signalingBaseUrl = NULL,
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

### Stopping one stream

Each `start*` has a matching `stop*` that stops just that stream and leaves you in the room. The matching `start*` brings it back — no rejoin.

```c
startPublishAudio();
startPublishVideo();

stopPublishVideo();   // audio keeps flowing, still in the room
startPublishVideo();  // video is back
```

They are safe to call unconditionally — stopping a stream that is not running returns `RESULT_OK`. Use `leave()` when you want to end the session; it stops everything regardless of what you already stopped.

### Application messages

Text and binary messages travel alongside the media streams. Open the channel with `startMessageChannel()` before you call `sendMessage()`. To receive, register a handler.

```c
#include "videosdk.h"


// Runs on an SDK task. Don't block here, and copy anything you want to keep --
// the buffer is only valid for the length of this call.
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
    // Call this after init() has already run (see the Usage section above),
    // and before you start any audio/video direction.

    // 1. Register the handlers first, so you don't miss an early message or an
    //    early disconnect. Both are optional: skip setDataMessageHandler() if
    //    you only send, and setConnectionStateHandler() if you don't track it.
    setDataMessageHandler(on_data_message);
    setConnectionStateHandler(on_connection_state, NULL);

    // 2. Open the channel once, before any sendMessage(). Until this succeeds,
    //    sendMessage() returns DATA_CHANNEL_NOT_STARTED.
    if (startMessageChannel() != RESULT_OK) {
        return;
    }

    // 3. Send as often as you like while the channel is open.
    const char *text = "hello from esp32";
    sendMessage((const uint8_t *)text, strlen(text), /* is_binary */ 0);

    const uint8_t blob[4] = {0xDE, 0xAD, 0xBE, 0xEF};
    sendMessage(blob, sizeof(blob), /* is_binary */ 1);

    // 4. Close the channel when you're done sending. leave() also closes it,
    //    so you only need this to stop messaging while staying in the room.
    stopMessageChannel();
}
```

`sendMessage()` takes at most 24000 bytes per message. It returns `DATA_CHANNEL_NOT_STARTED` if the channel was never opened, or `DATA_CHANNEL_QUEUE_FULL` when the outgoing queue backs up. Passing `NULL` to either handler setter clears it.

## API reference

All declarations live in [`include/videosdk.h`](include/videosdk.h).

### `init_config_t`

| Field | Type | Notes |
|-------|------|-------|
| `roomId` | `char *` | The room ID to join.|
| `token` | `char *` | VideoSDK JWT auth token. |
| `displayName` | `char *` | Name shown in the room. |
| `participantId` | `char *` | This device's peer id. `""` or `NULL` gives you a random 8-char id. |
| `audioCodec` | `audio_codec_t` | `AUDIO_CODEC_PCMA`, `AUDIO_CODEC_PCMU`, or `AUDIO_CODEC_OPUS`. |
| `videoCodec` | `video_codec_t` | `VIDEO_CODEC_NONE` (no video) or `VIDEO_CODEC_JPEG`. |

### Functions

| Function | Description |
|----------|-------------|
| `create_room_result_t create_room(char *token)` | Create a room. Returns a malloc'd `room_id` that the caller frees. |
| `void videosdk_set_log_mode(videosdk_log_mode_t mode)` | Set log verbosity (`VIDEOSDK_LOG_NORMAL` or `VIDEOSDK_LOG_DEBUG`). Call before `init()`. |
| `result_t init(init_config_t *cfg)` | Initialize the session and the board. Call once, before anything else. |
| `result_t startPublishAudio(void)` | Capture the microphone and send it to the room. |
| `result_t startPublishVideo(void)` | Capture the camera and send it to the room. |
| `result_t startSubscribeAudio(void)` | Receive remote audio and play it on the speaker. Korvo-2 only. |
| `result_t startSubscribeVideo(void)` | Receive remote video and show it on the display. Korvo-2 only. |
| `result_t stopPublishAudio(void)` | Stop sending the microphone. Stays in the room. |
| `result_t stopPublishVideo(void)` | Stop sending the camera and release it. Stays in the room. |
| `result_t stopSubscribeAudio(void)` | Stop receiving audio. Korvo-2 only. |
| `result_t stopSubscribeVideo(void)` | Stop receiving video. Korvo-2 only. |
| `void setSpeakerVolume(int volume)` | Set playback volume, 0 to 100 (out-of-range values are clamped). Korvo-2 only. |
| `result_t startMessageChannel(void)` | Open the message channel. Needed before `sendMessage()`. |
| `result_t sendMessage(const uint8_t *data, size_t len, int is_binary)` | Send a text or binary message. `len` is capped at 24000 bytes. |
| `result_t stopMessageChannel(void)` | Close the message channel. |
| `void setDataMessageHandler(data_message_cb_t cb)` | Set the incoming-message handler. `NULL` clears it. |
| `void setConnectionStateHandler(connection_state_cb_t cb, void *user)` | Set the connection-state handler. `NULL` clears it. |
| `result_t leave()` | Leave the room and stop every direction that was started. |

Directions are independent, so start them in any combination and any order. Callbacks run on SDK tasks, so don't block in them, and copy any buffer you want to keep.

### Result codes

`RESULT_OK` (0) means success. Errors fall in the `3001` to `3026` range, for example `DEVICE_NOT_SUPPORTED`, `AUDIO_CODEC_INIT_FAILED`, `DTLS_HANDSHAKE_FAILED`, `INIT_NOT_CALLED`, `DATA_CHANNEL_NOT_STARTED` and `DATA_CHANNEL_QUEUE_FULL`. The header lists them all.


## Documentation

- For more details, see the [VideoSDK Documentation](https://docs.videosdk.live/iot/guide/video-and-audio-calling-api-sdk/concept-and-architecture).
