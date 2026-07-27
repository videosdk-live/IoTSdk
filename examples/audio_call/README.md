# IoT SDK: Audio Call Example

Joins a VideoSDK meeting and streams audio only. It sends the on-board
microphone, and on the Korvo-2 it also plays the other participants through the
speaker. The session runs with `videoCodec = VIDEO_CODEC_NONE`.

| Feature | XIAO ESP32-S3 (Sense) | ESP32-S3-Korvo-2 v3.0 |
|---------|:---:|:---:|
| Send audio | ✅ | ✅ |
| Receive audio | ❌ | ✅ |

On the XIAO (no speaker) `startSubscribeAudio()` returns `DEVICE_NOT_SUPPORTED`.

## Create the project

```bash
idf.py create-project-from-example "videosdk/iot-sdk=0.3.1:audio_call"
```

(This example pulls the published component pinned in `main/idf_component.yml`:
`videosdk/iot-sdk: "^0.3.0"`.)

## Configure

```bash
idf.py set-target esp32s3
idf.py menuconfig
```

- SET Microcontroller > Audio hardware board: pick `ESP32-S3-Korvo-2` or `ESP32-S3-XIAO`. The default board is `ESP32-S3-XIAO`.
- VideoSDK Configuration: set Auth token (JWT) and Meeting / room ID. On the
  Korvo-2 you can also set Speaker output volume.
- Example Connection Configuration: Wi-Fi SSID and password.

The token and meeting ID live in `sdkconfig`, not in source. Don't hardcode a
real token and don't commit one.

## Build & flash

```bash
idf.py build
idf.py -p <PORT> flash monitor
```

See the [component README](../../README.md) for the full API reference.
