#ifndef VIDEOSDK_H_
#define VIDEOSDK_H_

#ifdef __cplusplus
extern "C"
{
#endif

    // enum for the supported codecs
    typedef enum
    {
        AUDIO_CODEC_PCMA,
        AUDIO_CODEC_PCMU,
        AUDIO_CODEC_OPUS,
    } audio_codec_t;

    // struct for the init method
    typedef struct
    {
        char *meetingID;
        char *token;
        char *displayName;
        audio_codec_t audioCodec;
    } init_config_t;

    // enum for the errors
    typedef enum
    {
        RESULT_OK = 0,                            // No error, same as return 0
        SSL_CONNECT_FAILED = 3001,                // ssl handshake failed
        HTTP_REQUEST_FAILED = 3002,               // Failed to send the HTTP request
        MEMORY_ALLOC_FAILED = 3003,               // Task memory allocation Failed
        DEVICE_NOT_SUPPORTED = 3004,              // Other device used will throw this error
        NULL_PARAMETER = 3005,                    // Function when pass with null parameter
        INIT_BOARD_FAILED = 3006,                 // init board for init board codec
        PEER_INIT_FAILED = 3007,                  // srtp init erorr
        TASK_ALREADY_STARTED = 3008,              // running the task and if it is already sstarted will throw this error
        PUBLISH_MUTEX_CREATE_FAILED = 3009,       // Mutex failed for publish method
        AUDIO_CODEC_INIT_FAILED = 3010,           // audio codec init failed for both subscribe/publish
        PUBLISH_PEER_CONNECTION_FAILED = 3011,    // Mutex failed for publish method
        PUBLISH_MEMORY_ALLOC_FAILED = 3012,       // publish memory allocation failed
        PUBLISH_TASK_CREATE_FAILED = 3013,        //  publish task failed
        SUBSCRIBE_MUTEX_CREATE_FAILED = 3014,     // Mutex failed for subscribe method
        SUBSCRIBE_PEER_CONNECTION_FAILED = 3015,  // Mutex failed for subscribe method
        SUBSCRIBE_MEMORY_ALLOC_FAILED = 3016,     // subscribe memory allocation failed
        SUBSCRIBE_TASK_CREATE_FAILED = 3017,      //  subscribe task failed
        STOP_PUBLISH_TASK_CREATE_FAILED = 3018,   // stop publish task failed
        STOP_SUBSCRIBE_TASK_CREATE_FAILED = 3019, // stop subscribe task failed
        CANDIDATE_PAIR_FAILED = 3020,             // failed after the checking state. Candidate Pair not matched
        DTLS_HANDSHAKE_FAILED = 3021,             // DTLS handshake failed
        LEAVE_FAILED = 3022,                      // Leave function Failed
        INIT_NOT_CALLED = 3023,                   // init method not called
        DUPLICATE_ID = 3024,                      // id can't be same
    } result_t;

    // struct to return the created Meeting and also the error code
    typedef struct
    {
        result_t code;
        char *room_id;
    } create_meeting_result_t;

    // create meeting
    create_meeting_result_t create_meeting(char *token);
    // initialize the meeting
    result_t init(init_config_t *cfg);
    // Publish Audio
    result_t startPublishAudio(char *publisherId);
    // subscribe Audio
    result_t startSubscribeAudio(char *subscriberId, char *subscribeToId);
    // Publish stop
    result_t stopPublishAudio();
    // Subscribe Audio Stop
    result_t stopSubscribeAudio();
    // leave method to stop the
    result_t leave();

#ifdef __cplusplus
}
#endif

#endif // VIDEOSDK_H_