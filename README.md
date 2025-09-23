# IOT SDK

At Video SDK, we’re building tools to help developers bring **real-time collaboration** to IoT and embedded devices. With the IoT SDK, you can integrate **live audio communication, meeting management, device-to-cloud connectivity, and session handling** directly into ESP32.

## Prerequisites

- Python >= 3.11
- A valid [Video SDK Account](https://app.videosdk.live/)

## Use IoT SDK Component

Follow these steps to use the IoT SDK component in your project:

### 1. Configure ESP-IDF enviorment 

- For setting up **ESP-IDF**, follow only **Step 1** from videosdk's [documentation](https://docs.videosdk.live/iot/guide/video-and-audio-calling-api-sdk/quickstart/quick-start#step-1-setup-for-esp-idf).
- Inside Step 1, you **do not need to run the project creation commands** — 
```
// Create an esp-idf project
cd ~/esp
idf.py create-project your-project-name
cd your-project-name
```
Once the tools and environment are set up, jump directly to **Step 2 in this README**.

### 2: Add IoT SDK component

To integrate the IoT SDK component, clone the repository below.

```
git clone https://github.com/videosdk-live/IoTSdk
```

After cloning, copy the component's local path and provide it in the `idf_component.yml` file as shown below.
```
dependencies:
  IoTSdk: 
    path: // Full local path of the component
```

### 3: Configure Dependencies

Open your `idf_component.yml` file and add the required dependencies. Place them below your existing ones:

```
  // Add the following dependencies after your existing ones
  mdns: '*'
  espressif/esp_audio_codec: ~2.3.0
  espressif/esp_codec_dev: ~1.3.4
  espressif/esp_audio_effects: ~1.1.0
  protocol_examples_common:
    path: ${IDF_PATH}/examples/common_components/protocol_examples_common
    ## Required IDF version
  idf:
    version: '>=4.1.0'
  sepfy/srtp: ^2.3.0
```

### 4. Generate a token from [VideoSDK's Dashboard](https://app.videosdk.live) and modify the token variable in `quick_start.c` file
```
const char *token = "GENRERATED_TOKEN"; // Your VideoSDK Authentication token
```

### 5. Build & Flash Project

Configure, build, and flash the firmware onto your ESP32 board. This compiles the code, applies the configurations, and uploads it to your device.
```
1. <!-- Run this command to set your board as the target-->
idf.py set-target esp32-s3

2. <!-- Run this command to do menuconfig -->

idf.py menuconfig  

         a. Inside the component config:
                |
                |———> mbedtls
                      | ——>Enable Support DTLS    <!-- It enables 3 way handshake  -->
                      | ——>Enable Support TLS      <!-- It enables 3 way handshake  -->
          And click S to save and again enter       

          b. Inside Example Connection Configuration:
                |
                |———> WIFI SSID         <!-- replace it with your WiFi name  -->
                |———> WIFI Password     <!-- replace it with your WiFi password -->
          And click S to save and again enter 

          c. Inside the Partition table :
                |
                |———> Partition table (custom partition table CSV)        
                      |———> Enable Custom partition table CSV

          d. Adjust the flash size inside Serial flasher config 
             (because some boards have limited flash memory)
                | ——> flash size: 2MB (esp32s3 XIAO sense)
                | ——> flash size: 4MB (esp32s3 qorvo2 v3.1)
          And click S to save and again enter

          e. Inside the Set Microcontroller : 
                | ——>**Audio hardware board (example : ESP32-S3-Korvo-2)**
                      | ——> Select your board name
                              |———> ESP32-S3-XIAO       
                              |———> ESP32- ESP32-S3-Korvo
          And click S to save and again enter

3. <!-- Run this command to build the project  -->
idf.py build

4. <!-- Run this command to flash the project to your microcontroller -->
idf.py flash monitor 
```

## Documentation

- For more details, check out the [VideoSDK Documentation](https://docs.videosdk.live/iot/guide/video-and-audio-calling-api-sdk/concept-and-architecture)

