# FeiYue Asr example

This example is the use of offline local wake up framework and Baidu online voice recognition

## WakeUp words

| # | word    | English Meaning | Pronunciation|
|:-:|---------|--------------------|-------------------|
| 1 | 嗨，乐鑫 | Hi, Espressif      | Hāi, lè xīn        |

## Key words

You can say everything.

### Note

If your Chinese pronunciation is not good, you can try the "Alexa" keyword. You need to modify some configurations to use them, see "Usage" for details.

## Compatibility

This example is will run on boards marked with green checkbox. Please remember to select the board in menuconfig as discussed is section *Usage* below.

| Board Name | Getting Started | Chip | Compatible |
|-------------------|:--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------:|:--------------------------------------------------------------------:|:-----------------------------------------------------------------:|
| ESP32-LyraT | [![alt text](../../../docs/_static/esp32-lyrat-v4.3-side-small.jpg "ESP32-LyraT")](https://docs.espressif.com/projects/esp-adf/en/latest/get-started/get-started-esp32-lyrat.html) | <img src="../../../docs/_static/ESP32.svg" height="85" alt="ESP32"> | ![alt text](../../../docs/_static/yes-button.png "Compatible") |
| ESP32-LyraTD-MSC | [![alt text](../../../docs/_static/esp32-lyratd-msc-v2.2-small.jpg "ESP32-LyraTD-MSC")](https://docs.espressif.com/projects/esp-adf/en/latest/get-started/get-started-esp32-lyratd-msc.html) | <img src="../../../docs/_static/ESP32.svg" height="85" alt="ESP32"> | ![alt text](../../../docs/_static/yes-button.png "Compatible") |
| ESP32-LyraT-Mini | [![alt text](../../../docs/_static/esp32-lyrat-mini-v1.2-small.jpg "ESP32-LyraT-Mini")](https://docs.espressif.com/projects/esp-adf/en/latest/get-started/get-started-esp32-lyrat-mini.html) | <img src="../../../docs/_static/ESP32.svg" height="85" alt="ESP32"> | ![alt text](../../../docs/_static/yes-button.png "Compatible") |
| ESP32-Dul1906 | ![alt text](../../../docs/_static/esp32-korvo-dul1906-v1.1-small.jpg "ESP32-Korvo-DUL1906") | <img src="../../../docs/_static/ESP32.svg" height="85" alt="ESP32"> | ![alt text](../../../docs/_static/yes-button.png "Compatible") |
| ESP32-S2-Kaluga-1 Kit | ![alt text](../../../docs/_static/esp32-s2-kaluga-1-kit-small.png "ESP32-S2-Kaluga-1 Kit") | <img src="../../../docs/_static/ESP32-S2.svg" height="100" alt="ESP32-S2"> | ![alt text](../../../docs/_static/no-button.png "Compatible") |
| Aithinker-Audio_Kit |
## Usage

Prepare the audio board:

- Insert a MP3 files 'dingdong.mp3'

Configure the example:

- Select compatible audio board in `menuconfig` > `Audio HAL`.
- If you want to use "nihaoxiaozhi" as a wakeup word, open menuconfig, go to `Speech Recognition Configuration` and select:
    - `Wake word engine` > `WakeNet 5 (quantized)` or > `WakeNet 6 (quantized)`
    - `Wake word name` > `nihaoxiaozhi`
- If you want to use "nihaoxiaozhi" as a wakeup word, open menuconfig, go to `Speech Recognition Configuration` and select:
    - `Wake word engine` > `WakeNet 5 (quantized)`  
    - `Wake word name` > `hi jeson`
Load and run the example:

- Say the keywords to the board and you should see them printed out in the monitor.
