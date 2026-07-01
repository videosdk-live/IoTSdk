#ifndef VIDEOSDK_H_
#define VIDEOSDK_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ===========================================================================
// VideoSDK IoT SDK -- usage contract
// ---------------------------------------------------------------------------
// Call order (from a single task, e.g. app_main -- the API is NOT thread-safe;
// do not call these concurrently from multiple tasks):
//   1. init(&cfg)                     -- exactly once; validates + sets up board.
//   2. (optional) setConnectionStateHandler -- register the callback before the
//      start* calls so a mid-session drop is delivered.
//   3. start{Publish,Subscribe}{Audio,Video}() -- in any combination/order. The
//      first publish OR subscribe call brings up its transport; later calls
//      reuse it. Publish* and startSubscribeVideo are Korvo-2 only.
//   4. leave()                        -- stops publish + subscribe (async).
//
// Board selection is compile-time (idf.py menuconfig -> "VideoSDK IoT SDK"):
// Korvo-2 = full-duplex A/V; XIAO ESP32-S3 = audio+video SEND only.
//
// Memory ownership: create_meeting() returns a malloc'd room_id the caller must
// free(). Config strings passed to init() are NOT copied -- keep them alive for
// the whole session. Callbacks run on internal tasks; do not block in them.
// ===========================================================================

// enum for the supported audio codec. Only G.711 A-law (PCMA) is supported.
typedef enum {
  AUDIO_CODEC_PCMA = 0,  // G.711 A-law, 8 kHz mono, 160 B / 20 ms
} audio_codec_t;

// enum for the supported video codecs (DataChannel send, Korvo-2 only)
typedef enum {
  VIDEO_CODEC_NONE = 0,  // audio only (default)
  VIDEO_CODEC_JPEG,      // hardware-JPEG camera frames over the data channel
} video_codec_t;

// struct for the init method. Declares the session's media formats. Each
// direction is still enabled per-call (startPublish*/startSubscribe*); these
// fields only pick the codec used when a direction is started.
typedef struct {
  char* meetingID;
  char* token;
  char* displayName;
  audio_codec_t audioCodec;  // PCMA (G.711 A-law) -- the only supported codec
  video_codec_t videoCodec;  // JPEG (currently the only supported video format)
} init_config_t;

// enum for the errors
typedef enum {
  RESULT_OK = 0,                             // No error, same as return 0
  SSL_CONNECT_FAILED = 3001,                 // ssl handshake failed
  HTTP_REQUEST_FAILED = 3002,                // Failed to send the HTTP request
  MEMORY_ALLOC_FAILED = 3003,                // Task memory allocation Failed
  DEVICE_NOT_SUPPORTED = 3004,               // Other device used will throw this error
  NULL_PARAMETER = 3005,                     // Function when pass with null parameter
  INIT_BOARD_FAILED = 3006,                  // init board for init board codec
  PEER_INIT_FAILED = 3007,                   // srtp init erorr
  TASK_ALREADY_STARTED = 3008,               // running the task and if it is already sstarted will throw this error
  PUBLISH_MUTEX_CREATE_FAILED = 3009,        // Mutex failed for publish method
  AUDIO_CODEC_INIT_FAILED = 3010,            // audio codec init failed for both subscribe/publish
  PUBLISH_PEER_CONNECTION_FAILED = 3011,     // Mutex failed for publish method
  PUBLISH_MEMORY_ALLOC_FAILED = 3012,        // publish memory allocation failed
  PUBLISH_TASK_CREATE_FAILED = 3013,         //  publish task failed
  SUBSCRIBE_MUTEX_CREATE_FAILED = 3014,      // Mutex failed for subscribe method
  SUBSCRIBE_PEER_CONNECTION_FAILED = 3015,   // Mutex failed for subscribe method
  SUBSCRIBE_MEMORY_ALLOC_FAILED = 3016,      // subscribe memory allocation failed
  SUBSCRIBE_TASK_CREATE_FAILED = 3017,       //  subscribe task failed
  STOP_PUBLISH_TASK_CREATE_FAILED = 3018,    // stop publish task failed
  STOP_SUBSCRIBE_TASK_CREATE_FAILED = 3019,  // stop subscribe task failed
  CANDIDATE_PAIR_FAILED = 3020,              // failed after the checking state. Candidate Pair not matched
  DTLS_HANDSHAKE_FAILED = 3021,              // DTLS handshake failed
  LEAVE_FAILED = 3022,                       // Leave function Failed
  INIT_NOT_CALLED = 3023,                    // init method not called
  DUPLICATE_ID = 3024,                       // id can't be same
} result_t;

// struct to return the created Meeting and also the error code
typedef struct {
  result_t code;
  char* room_id;
} create_meeting_result_t;

// create meeting
create_meeting_result_t create_meeting(char* token);
// initialize the meeting
result_t init(init_config_t* cfg);
// Publish Audio (mic -> data channel). Foundation: also brings up the send &
// recv transports and the publish peer connection.
result_t startPublishAudio(char* publisherId);
// Publish Video (camera JPEG -> data channel, sid 1). Call AFTER
// startPublishAudio. Korvo-2 only.
result_t startPublishVideo(void);
// Subscribe Audio (data channel -> ES8311 speaker). Call AFTER startPublishAudio.
result_t startSubscribeAudio(void);
// Subscribe Video (remote JPEG -> ST7789 LCD). Call AFTER startSubscribeAudio.
// Korvo-2 only.
result_t startSubscribeVideo(void);
// Publish stop
result_t stopPublishAudio();
// Subscribe Audio Stop
result_t stopSubscribeAudio();
// Set speaker playback volume at runtime, 0-100 (out-of-range values are
// clamped). Seeded from CONFIG_SPEAKER_VOLUME (menuconfig); call this to
// override. Korvo-2 only (the XIAO has no speaker).
void setSpeakerVolume(int volume);

// Handler for signaling connection-state changes. `connected == false` means the
// protoo WebSocket dropped and the session is dead -- the app should stop/leave
// and rejoin; `connected == true` fires when the signaling socket is up. Runs on
// the signaling event task, so do not block. `user` is passed back verbatim.
typedef void (*connection_state_cb_t)(bool connected, void* user);
// Register (or clear, with NULL) the connection-state handler. Call after init(),
// before startPublish*/startSubscribe*, so a mid-session drop is delivered.
void setConnectionStateHandler(connection_state_cb_t cb, void* user);
// leave method to stop the
result_t leave();

#ifdef __cplusplus
}
#endif

#endif  // VIDEOSDK_H_
