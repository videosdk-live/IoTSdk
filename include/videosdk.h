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
// create_meeting() hands back a malloc'd room_id that you free(). init() copies
// the strings in init_config_t, so you can free your own buffers as soon as it
// returns. Callbacks run on SDK tasks: don't block, and copy anything you keep.

// enum for the supported audio codecs
typedef enum {
  AUDIO_CODEC_PCMA = 0,  // G.711 A-law, 8 kHz mono
  AUDIO_CODEC_PCMU,      // G.711 u-law, 8 kHz mono
  AUDIO_CODEC_OPUS,      // Opus, 16 kHz 
} audio_codec_t;

// enum for the supported video codecs
typedef enum {
  VIDEO_CODEC_NONE = 0,  // no video (default)
  VIDEO_CODEC_JPEG,      // hardware-JPEG camera frames
} video_codec_t;

// log verbosity for the SDK
typedef enum {
  VIDEOSDK_LOG_NORMAL = 0,  // lifecycle lines, warnings, errors
  VIDEOSDK_LOG_DEBUG,       // adds periodic heartbeats and diagnostics
} videosdk_log_mode_t;

// struct for the init method. Holds the session identity and picks the codec
// each direction uses once it is started. 
typedef struct {
  char* meetingID;
  char* token;
  char* displayName;         // user-configurable name for display in the meeting
  char* participantId;       // this device's peer id; 
  audio_codec_t audioCodec;  // PCMA / PCMU / Opus
  video_codec_t videoCodec;  // JPEG, or VIDEO_CODEC_NONE for no video
} init_config_t;

// enum for the errors
typedef enum {
  RESULT_OK = 0,                             // No error, same as return 0
  SSL_CONNECT_FAILED = 3001,                 // ssl handshake failed
  HTTP_REQUEST_FAILED = 3002,                // Failed to send the HTTP request
  MEMORY_ALLOC_FAILED = 3003,                // Task memory allocation Failed
  DEVICE_NOT_SUPPORTED = 3004,               // This board does not support that direction
  NULL_PARAMETER = 3005,                     // Function when pass with null parameter
  INIT_BOARD_FAILED = 3006,                  // init board for init board codec
  PEER_INIT_FAILED = 3007,                   // Peer initialization failed
  TASK_ALREADY_STARTED = 3008,               // That direction is already running
  PUBLISH_MUTEX_CREATE_FAILED = 3009,        // Mutex failed for publish method
  AUDIO_CODEC_INIT_FAILED = 3010,            // audio codec init failed for both subscribe/publish
  PUBLISH_PEER_CONNECTION_FAILED = 3011,     // Publish peer connection failed
  PUBLISH_MEMORY_ALLOC_FAILED = 3012,        // publish memory allocation failed
  PUBLISH_TASK_CREATE_FAILED = 3013,         //  publish task failed
  SUBSCRIBE_MUTEX_CREATE_FAILED = 3014,      // Mutex failed for subscribe method
  SUBSCRIBE_PEER_CONNECTION_FAILED = 3015,   // Subscribe peer connection failed
  SUBSCRIBE_MEMORY_ALLOC_FAILED = 3016,      // subscribe memory allocation failed
  SUBSCRIBE_TASK_CREATE_FAILED = 3017,       //  subscribe task failed
  STOP_PUBLISH_TASK_CREATE_FAILED = 3018,    // stop publish task failed
  STOP_SUBSCRIBE_TASK_CREATE_FAILED = 3019,  // stop subscribe task failed
  CANDIDATE_PAIR_FAILED = 3020,              // failed after the checking state. Candidate Pair not matched
  DTLS_HANDSHAKE_FAILED = 3021,              // DTLS handshake failed
  LEAVE_FAILED = 3022,                       // Leave function Failed
  INIT_NOT_CALLED = 3023,                    // init method not called
  DUPLICATE_ID = 3024,                       // id can't be same
  DATA_CHANNEL_NOT_STARTED = 3025,           // sendMessage() before startMessageChannel()
  DATA_CHANNEL_QUEUE_FULL = 3026,            // Outgoing message queue is full
} result_t;

// struct to return the created Meeting and also the error code
typedef struct {
  result_t code;
  char* room_id;  // malloc'd -- caller must free()
} create_meeting_result_t;

// Set how much the SDK logs. Call once, before init(). Only the SDK's own log
// tags are touched. DEBUG output shows up only when the library was built with
// debug-level logging available.
void videosdk_set_log_mode(videosdk_log_mode_t mode);

// create meeting
create_meeting_result_t create_meeting(char* token);
// initialize the meeting
result_t init(init_config_t* cfg);

// Capture the microphone and send it to the meeting.
result_t startPublishAudio(void);
// Capture the camera and send it to the meeting.
result_t startPublishVideo(void);
// Receive remote audio and play it on the speaker. Korvo-2 only.
result_t startSubscribeAudio(void);
// Receive remote video and render it on the display. Korvo-2 only.
result_t startSubscribeVideo(void);
// Publish stop
result_t stopPublishAudio();
// Subscribe Audio Stop
result_t stopSubscribeAudio();

// Speaker playback volume, 0-100. Out-of-range values are clamped. Starts at
// CONFIG_SPEAKER_VOLUME (menuconfig). Korvo-2 only; the XIAO has no speaker.
void setSpeakerVolume(int volume);

// Called for each message from another participant. `data`/`len` is the whole
// message, already reassembled. `is_binary` is 1 for binary, 0 for UTF-8 text.
// `sid` is the stream it came in on. The buffer only lives for the call, so
// copy what you keep. Runs on an SDK task, so don't block.
typedef void (*data_message_cb_t)(const uint8_t* data, size_t len,
                                  int is_binary, uint16_t sid);
// Set the message handler, or pass NULL to clear it. With no handler the
// messages are dropped. Audio and video are unaffected either way.
void setDataMessageHandler(data_message_cb_t cb);

// Open the message channel. sendMessage() needs this first.
result_t startMessageChannel(void);
// Send a message to the other participants. `len` tops out at 24000 bytes.
// Gives back DATA_CHANNEL_NOT_STARTED if the channel was never opened, or
// DATA_CHANNEL_QUEUE_FULL when the outgoing queue backs up.
result_t sendMessage(const uint8_t* data, size_t len, int is_binary);
// Close the message channel.
result_t stopMessageChannel(void);

// Called when signaling comes up or goes away. `connected == false` means the
// session is gone and the app should leave() and rejoin. Runs on an SDK task,
// so don't block. `user` comes back untouched.
typedef void (*connection_state_cb_t)(bool connected, void* user);
// Set the connection-state handler, or pass NULL to clear it. Register it after
// init() and before the start* calls, otherwise you can miss an early drop.
void setConnectionStateHandler(connection_state_cb_t cb, void* user);

// leave method to stop the call and close all publish/subscribe streams
result_t leave();

#ifdef __cplusplus
}
#endif

#endif  // VIDEOSDK_H_
