# IoT SDK — Audio Call Example

Joins a VideoSDK meeting and streams **audio only**: it publishes the on-board
microphone and (on Korvo-2) plays remote participants' audio through the speaker.
The session is initialized with `videoCodec = VIDEO_CODEC_NONE`.

| Board | Publish audio | Subscribe audio (speaker) |
|-------|:---:|:---:|
| ESP32-S3-Korvo-2 v3.0 | ✅ | ✅ |
| XIAO ESP32-S3 (Sense) | ✅ | ❌ (`DEVICE_NOT_SUPPORTED`) |

## Create the project

```bash
idf.py create-project-from-example "videosdk/iot-sdk=*:audio_call"
```

(Within this repo the example already builds against the local component via
`override_path` in `main/idf_component.yml`.)

## Configure

```bash
idf.py set-target esp32s3
idf.py menuconfig
```

- **SET Microcontroller → Audio hardware board** — `ESP32-S3-Korvo-2` or `ESP32-S3-XIAO`.
- **VideoSDK Configuration** — set **Auth token (JWT)** (`CONFIG_VIDEOSDK_TOKEN`) and
  **Meeting / room ID** (`CONFIG_VIDEOSDK_MEETING_ID`). On Korvo-2 you can also set
  **Speaker output volume**.
- **Example Connection Configuration** — Wi-Fi SSID and password.

> The token and meeting ID are read from menuconfig (stored in `sdkconfig`) —
> never hardcode a real token in source or commit it.

## Build & flash

```bash
idf.py build
idf.py -p <PORT> flash monitor
```

See the [component README](../../README.md) for the full API reference.
