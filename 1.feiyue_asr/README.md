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

| Board Name |
|-------------------|
| ESP32-LyraT | 
| ESP32-LyraTD-MSC | 
| ESP32-LyraT-Mini | 
| ESP32-Dul1906 | 
| ESP32-S2-Kaluga-1 Kit | 
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
