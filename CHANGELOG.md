# Changelog

All notable changes to the `videosdk/iot-sdk` component.

## v0.3.2

**Release Date**: 31st Jul 2026

- Docs & reliability: Reorganized and refreshed the guides (boards-as-rows tables with hardware links, clearer prerequisites, up-to-date version pins), plus more detailed connection error reporting and steadier data-channel handling in the refreshed prebuilt libraries.

## v0.3.1

**Release Date**: 27th Jul 2026

- Breaking: The separate stop-audio calls were removed. Teardown is handled for you now — call `leave()` to stop every direction. (The old `stopPublishAudio` / `stopSubscribeAudio` are gone.)
- Breaking: The `DUPLICATE_ID` (3024) result code was removed. The remaining codes are unchanged.
- Improvement: More reliable video receive on the Korvo-2, with smoother playback on the display.


## v0.3.0

**Release Date**: 11th Jul 2026

- Breaking: `startPublishAudio()` no longer takes an argument. Set the device's id once in the new `init_config_t.participantId` field instead:

  ```c
  // before
  startPublishAudio("my-device-id");

  // now
  init_config_t cfg = { ..., .participantId = "my-device-id" };
  init(&cfg);
  startPublishAudio();
  ```

  Leave `participantId` as `""` or `NULL` and you still get a random id, as before.
- Breaking: `init_config_t` has a new `participantId` field. Rebuild your application against the new header.
- Feature: Send and receive messages during a call. Open the channel with `startMessageChannel()`, send text or binary with `sendMessage()` (up to 24000 bytes per message), and close it with `stopMessageChannel()`. Register `setDataMessageHandler()` to receive what other participants send.
- Feature: `setConnectionStateHandler()` reports when the connection comes up or drops, so your application can rejoin after a disconnect.
- Feature: Audio can be sent as PCMU or Opus in addition to PCMA. Pick one with `init_config_t.audioCodec`.
- Update: Two new result codes. `DATA_CHANNEL_NOT_STARTED` (3025) means `sendMessage()` was called before the channel was opened, and `DATA_CHANNEL_QUEUE_FULL` (3026) means messages are being queued faster than they can be sent.
- Feature: Pick the log verbosity at runtime with `videosdk_set_log_mode()`. `VIDEOSDK_LOG_NORMAL` keeps the lifecycle, warning and error lines; `VIDEOSDK_LOG_DEBUG` adds the periodic heartbeats and diagnostics. Call it before `init()`.
- Fix: Building a bundled example no longer shows the board and VideoSDK menus twice in `menuconfig`.

## v0.2.2

**Release Date**: 1st Jul 2026

- Breaking: `startSubscribeAudio()` no longer takes arguments.
- Breaking: `audio_codec_t` temporarily offers only `AUDIO_CODEC_PCMA`. PCMU and Opus return in v0.3.0.
- Feature: Video support. Send camera frames to the meeting, and on the Korvo-2 show remote video on the display. Choose the format with the new `init_config_t.videoCodec` field, either `VIDEO_CODEC_NONE` or `VIDEO_CODEC_JPEG`. Receiving video is Korvo-2 only.
- Feature: `setSpeakerVolume()` changes playback volume while a call is running (Korvo-2 only).
- Update: The component now pulls in everything the video path needs, so you do not have to add those dependencies yourself. The minimum ESP-IDF version is now 5.4.
- Fix: More reliable receive path, and fewer leaks over long sessions.

## v0.0.4

**Release Date**: 27th Apr 2026

- Fix: Resolved undefined-reference errors that could appear when linking the component into an application.

## v0.0.3

**Release Date**: 27th Apr 2026

- Fix: The component now declares its own runtime dependencies, so adding it to a project no longer produces undefined-reference errors at link time.
- Update: The manifest now carries the repository, issue tracker and documentation links, along with tags and the supported targets.

## v0.0.2

**Release Date**: 20th Jan 2026

- Update: Refreshed the prebuilt libraries for the ESP32-S3-Korvo-2.

## v0.0.1

**Release Date**: 23rd Sep 2025

- Initial release of the VideoSDK IoT SDK.
