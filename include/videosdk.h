#ifndef VIDEOSDK_H_
#define VIDEOSDK_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Usage
//
// Call init() once, first, from a single task (app_main is fine). The API is
// not thread-safe, so don't drive it from several tasks at once.
//
// If you want early events, register the callbacks before the start* calls.
// Then start whichever directions you need, in any order. leave() shuts them
// all down again.
//
// The board is chosen at build time (idf.py menuconfig -> "SET Microcontroller").
// Korvo-2 does every direction. The XIAO has no speaker and no screen, so
// startSubscribeAudio() and startSubscribeVideo() return DEVICE_NOT_SUPPORTED.
//
// create_room() hands back a malloc'd room_id that you free(). init() copies
// the strings in init_config_t, so you can free your own buffers as soon as it
// returns. Callbacks run on SDK tasks: don't block, and copy anything you keep.

// Audio codecs you can send.
typedef enum {
  AUDIO_CODEC_PCMA = 0,  // G.711 A-law, 8 kHz mono
  AUDIO_CODEC_PCMU,      // G.711 u-law, 8 kHz mono
  AUDIO_CODEC_OPUS,      // Opus, 16 kHz
} audio_codec_t;

// Video codecs you can send.
typedef enum {
  VIDEO_CODEC_NONE = 0,  // no video (default)
  VIDEO_CODEC_JPEG,      // hardware-JPEG camera frames
} video_codec_t;

// How much the SDK prints. See videosdk_set_log_mode().
typedef enum {
  VIDEOSDK_LOG_NORMAL = 0,  // lifecycle lines, warnings, errors
  VIDEOSDK_LOG_DEBUG,       // adds periodic heartbeats and diagnostics
} videosdk_log_mode_t;

// Options passed to init().
typedef struct {
  char* roomId;              // the room to join
  char* token;               // your VideoSDK auth token (JWT)
  char* displayName;         // the name shown for this device in the room
  char* participantId;       // this device's id; "" or NULL for a random one
  audio_codec_t audioCodec;  // PCMA / PCMU / Opus
  video_codec_t videoCodec;  // JPEG, or VIDEO_CODEC_NONE for no video
  // Optional. Leave it NULL to use "api.videosdk.live".
  char* signalingBaseUrl;
} init_config_t;

// Return codes. RESULT_OK is success; anything else is a failure.
typedef enum {
  RESULT_OK = 0,                             // success
  SSL_CONNECT_FAILED = 3001,                 // could not open the secure connection to the server
  HTTP_REQUEST_FAILED = 3002,                // a request to the server failed
  MEMORY_ALLOC_FAILED = 3003,                // out of memory
  DEVICE_NOT_SUPPORTED = 3004,               // this board can't do that direction (e.g. subscribe on the XIAO)
  NULL_PARAMETER = 3005,                     // a required argument was NULL
  INIT_BOARD_FAILED = 3006,                  // the audio/video board failed to start up
  PEER_INIT_FAILED = 3007,                   // the media/security layer failed to start up
  TASK_ALREADY_STARTED = 3008,               // that direction is already running
  PUBLISH_MUTEX_CREATE_FAILED = 3009,        // could not start publishing (out of resources)
  AUDIO_CODEC_INIT_FAILED = 3010,            // the audio codec failed to start up
  PUBLISH_PEER_CONNECTION_FAILED = 3011,     // could not set up the publish connection
  PUBLISH_MEMORY_ALLOC_FAILED = 3012,        // out of memory while starting publish
  PUBLISH_TASK_CREATE_FAILED = 3013,         // could not start publishing (out of resources)
  SUBSCRIBE_MUTEX_CREATE_FAILED = 3014,      // could not start subscribing (out of resources)
  SUBSCRIBE_PEER_CONNECTION_FAILED = 3015,   // could not set up the subscribe connection
  SUBSCRIBE_MEMORY_ALLOC_FAILED = 3016,      // out of memory while starting subscribe
  SUBSCRIBE_TASK_CREATE_FAILED = 3017,       // could not start subscribing (out of resources)
  STOP_PUBLISH_TASK_CREATE_FAILED = 3018,    // could not stop publishing
  STOP_SUBSCRIBE_TASK_CREATE_FAILED = 3019,  // could not stop subscribing
  CANDIDATE_PAIR_FAILED = 3020,              // could not find a working network path to the other side
  DTLS_HANDSHAKE_FAILED = 3021,              // the encrypted media handshake failed
  LEAVE_FAILED = 3022,                       // leave() failed
  INIT_NOT_CALLED = 3023,                    // call init() first
  DATA_CHANNEL_NOT_STARTED = 3025,           // call startMessageChannel() before sendMessage()
  DATA_CHANNEL_QUEUE_FULL = 3026,            // sending faster than messages can go out; retry shortly
} result_t;

// Returned by create_room().
typedef struct {
  result_t code;
  char* room_id;  // the new room id, malloc'd -- free() it when done
} create_room_result_t;

// Set how much the SDK logs. Call once, before init(). Only the SDK's own log
// tags are touched. DEBUG output shows up only when the library was built with
// debug-level logging available.
void videosdk_set_log_mode(videosdk_log_mode_t mode);

// Create a new room. On success the room id is in room_id (free() it);
// check code for errors.
create_room_result_t create_room(char* token);
// Initialize the SDK. Call once, before anything else here.
result_t init(init_config_t* cfg);

// Capture the microphone and send it to the room.
result_t startPublishAudio(void);
// Capture the camera and send it to the room.
result_t startPublishVideo(void);
// Receive remote audio and play it on the speaker. Korvo-2 only.
result_t startSubscribeAudio(void);
// Receive remote video and show it on the display. Korvo-2 only.
result_t startSubscribeVideo(void);

// Stop one stream without leaving the room. The other streams keep running and
// the matching start* above brings this one back — no rejoin needed. Use them to
// drop video to save power, or to mute the microphone, during a call.
//
// Safe to call unconditionally: stopping a stream that is not running returns
// RESULT_OK. They return once the stream is down. INIT_NOT_CALLED if init() has
// not run yet.
//
// These stop media, they do not end the session — leave() is still what you call
// to leave the room, and it works whatever you have already stopped here.
result_t stopPublishAudio(void);
result_t stopPublishVideo(void);
result_t stopSubscribeAudio(void);
// Korvo-2 only, like startSubscribeVideo().
result_t stopSubscribeVideo(void);

// Set speaker playback volume, 0-100 (out-of-range values are clamped). Starts
// at the value picked in menuconfig. Korvo-2 only; the XIAO has no speaker.
void setSpeakerVolume(int volume);

// Called for each message another participant sends you. `data`/`len` is the
// whole message, already reassembled. `is_binary` is 1 for binary, 0 for UTF-8
// text. `sid` identifies the stream it came in on. The buffer only lives for the
// call, so copy what you keep. Runs on an SDK task, so don't block.
typedef void (*data_message_cb_t)(const uint8_t* data, size_t len,
                                  int is_binary, uint16_t sid);
// Set the handler for incoming messages, or pass NULL to clear it. With no
// handler, incoming messages are dropped. Audio and video are unaffected.
void setDataMessageHandler(data_message_cb_t cb);

// Open the message channel. Call this before sendMessage().
result_t startMessageChannel(void);
// Send a message to the other participants. `len` tops out at 24000 bytes.
// Returns DATA_CHANNEL_NOT_STARTED if the channel was never opened, or
// DATA_CHANNEL_QUEUE_FULL when the outgoing queue backs up.
result_t sendMessage(const uint8_t* data, size_t len, int is_binary);
// Close the message channel.
result_t stopMessageChannel(void);

// Called when the connection comes up or goes away. `connected == false` means
// the session is gone and you should leave() and rejoin. Runs on an SDK task, so
// don't block. `user` is whatever you passed to setConnectionStateHandler().
typedef void (*connection_state_cb_t)(bool connected, void* user);
// Set the connection-state handler, or pass NULL to clear it. Register it after
// init() and before the start* calls so you don't miss an early drop.
void setConnectionStateHandler(connection_state_cb_t cb, void* user);

// Leave the room. Stops every direction you started.
result_t leave();

#ifdef __cplusplus
}
#endif

#endif  // VIDEOSDK_H_
