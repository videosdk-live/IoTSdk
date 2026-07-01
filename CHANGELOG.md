## v0.2.2

**Release Date**: 1st Jul 2026

- Feature: Full audio **and video** API — publish/subscribe video (hardware JPEG over the data channel) in addition to audio, plus runtime speaker volume control (`setSpeakerVolume`). *Video and audio subscribe are Korvo-2 only; the XIAO is send-only.*
- Feature: Signaling connection-state notifications (`setConnectionStateHandler`).
- Update: `init_config_t` gains a `videoCodec` field (`VIDEO_CODEC_NONE` / `VIDEO_CODEC_JPEG`); `startSubscribeAudio()` is now no-arg.
- Update: Audio is **PCMA (G.711 A-law) only** — `audio_codec_t` now exposes just `AUDIO_CODEC_PCMA` (PCMU / Opus removed from the public API).
- Update: Receive path hardened — incoming data is routed by SCTP stream id, and reliability/leak fixes in the connection lifecycle.
- Update: Declare the additional runtime dependencies the A/V path needs (`esp_jpeg`, `esp32-camera`, `esp_websocket_client`, `sepfy/usrsctp`) in `idf_component.yml` and the CMake `REQUIRES`; bump the minimum ESP-IDF to `5.4`.

## v0.0.4

**Release Date**: 27th Apr 2026

- Fix: Use `add_prebuilt_library` with explicit `REQUIRES` so CMake places dependency archives after the prebuilt `.a` on the linker command line. Previously, raw `target_link_libraries(... INTERFACE lib.a)` did not propagate transitive deps, and single-pass `ld` left `esp_capture_*` / `esp_audio_*` / `esp_codec_dev_*` symbols unresolved when the component happened to be loaded after its providers (alphabetical namespace ordering).

## v0.0.3

**Release Date**: 27th Apr 2026

- Fix: Declare runtime dependencies (`esp_audio_codec`, `esp_codec_dev`, `esp_audio_effects`, `esp_capture`, `esp_video_codec`, `sepfy/srtp`, `mdns`) in `idf_component.yml` so consumers no longer hit undefined-reference linker errors.
- Fix: Add `REQUIRES` for IDF drivers (`esp_driver_i2c`, `esp_driver_i2s`, `esp_driver_gpio`, `sdmmc`, `mbedtls`, `esp_netif`, `esp_event`, `esp_wifi`, `nvs_flash`) needed by the prebuilt static libraries.
- Update: Manifest enriched with `url`, `repository`, `issues`, `documentation`, `tags`, and `targets`.

## v0.0.2

**Release Date**: 20th Jan 2026

- Update: Update IoT SDK static libraries for Korvo v2 platforms.

## v0.0.1

**Release Date**: 23rd Sep 2025

- VideoSdk IoT SDK Initial release

 