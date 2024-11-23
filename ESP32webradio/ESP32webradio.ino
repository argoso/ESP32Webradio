// You need to use audio libary from: https://github.com/schreibfaul1/ESP32-audioI2S
// Inspired by https://en.polluxlabs.net/esp32-esp8266-projects/esp32-internet-radio/ and
// https://www.az-delivery.de/en/blogs/azdelivery-blog-fur-arduino-und-raspberry-pi/internet-radio-mit-dem-esp32
#include <Arduino.h>
#include <WiFi.h>
#include <Audio.h>
#include <AiEsp32RotaryEncoder.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "FreeMono6pt8b.h" 
#include <Preferences.h>
#include <unordered_map>
#include <String>

#define I2S_DOUT 19 //DIN
#define I2S_BCLK 23 //BCK
#define I2S_LRC 22 //LCK

#define ROTARY_ENCODER_A_PIN 14 //DT
#define ROTARY_ENCODER_B_PIN 12 //CLK
#define ROTARY_ENCODER_BUTTON_PIN 27 //SW
#define ROTARY_ENCODER_STEPS 4

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
#define OLED_SDA 17
#define OLED_SCL 16
//#define OLED_RST 16

#define LINE1_Y 10 // Esimese rea Y-koordinaat
#define LINE2_Y 20 // Teise rea Y-koordinaat

const char* stations[] = {
    "https://ice.leviracloud.eu/starFMEesti96-aac",
    "https://c4.radioboss.fm:18123/stream.m3u",
    "http://icecast.err.ee/vikerraadio.mp3",
    "https://cdn.treraadio.ee/ringfm",
    "https://ice.leviracloud.eu/star96-aac",
    "https://stream.skymedia.ee/live/SKY",
    "https://stream.skymedia.ee/live/SKYPLUS",
    "https://edge02.cdn.bitflip.ee:8888/rck",
    "https://stream.relaxfm.ee/relax",
    "https://stream.relaxfm.ee/cafe",
    "https://router.euddn.net/8103046e16b71d15d692b57c187875c7/kuku_high.mp3",
    "https://edge06.cdn.bitflip.ee:8888/kadi",
    "https://router.euddn.net/8103046e16b71d15d692b57c187875c7/elmar.aac",
    "http://router.euddn.net/8103046e16b71d15d692b57c187875c7/radioduo.aac",
    "http://icecast.err.ee/raadio2.mp3",
    "http://icecast.err.ee/r2rock.mp3",
    "http://icecast.err.ee/r2pop.mp3",
    "https://icecast.err.ee/r2p.mp3",
    "http://icecast.err.ee/r2chill.mp3",
    "http://icecast.err.ee/r2alternatiiv.mp3",
    "http://icecast.err.ee/r2eesti.mp3",
    "https://ice.leviracloud.eu/phr96-aac",
    "https://edge03.cdn.bitflip.ee:8888/NRJ",
    "http://router.euddn.net/8103046e16b71d15d692b57c187875c7/myhits.aac",
    "http://5.39.16.62:8004/tre",
    "http://icecast.err.ee/klassikaraadio.mp3",
    "http://icecast.err.ee/klaranostalgia.mp3",
    "http://icecast.err.ee/klaraklassika.mp3",
    "http://icecast.err.ee/klarajazz.mp3",
    "https://stream.skymedia.ee/live/KissPride",
    "https://stream.skymedia.ee/live/KissEurovisioon",
    "https://stream.skymedia.ee/live/KISS",
    "https://locomanoco.vlevelscdn.net/radio/kaguraadio.stream/playlist.m3u8",
    "https://stream.hitmix.ee:8443/HIT_320",
    "http://s5.radio.co/s69e02764f/listen",
    "http://www.happyu.ee/streaming/happyu.m3u",
    "http://icecast.err.ee/raadio2.mp3",
    "http://icecast.err.ee/klassikaraadio.mp3",
    "http://router.euddn.net/8103046e16b71d15d692b57c187875c7/elmaritantsuohtu.aac",
    "http://router.euddn.net/8103046e16b71d15d692b57c187875c7/rokk.aac",
    "https://router.euddn.net/8103046e16b71d15d692b57c187875c7/duodance.aac",
    "http://router.euddn.net/8103046e16b71d15d692b57c187875c7/dc_duocountry.aac",
    "http://router.euddn.net/8103046e16b71d15d692b57c187875c7/dfm.mp3",
    "https://www.aripaev.ee/raadio/stream.mp3"
};
const char* stationNames[] = {
    "00: Star FM Eesti",
    "01: Võmba FM",
    "02: Vikerraadio",
    "03: Tre Raadio Ring FM",
    "04: Star FM",
    "05: Sky-Radio",
    "06: Sky Plus",
    "07: Rock FM",
    "08: Relax FM Eesti",
    "09: Relax Cafe",
    "10: Raadio Kuku",
    "11: Raadio Kadi",
    "12: Raadio Elmar",
    "13: Raadio Duo",
    "14: Raadio 2 HQ",
    "15: R2Rock",
    "16: R2Pop",
    "17: R2p",
    "18: R2Chill",
    "19: R2Altpop",
    "20: R2 Eesti",
    "21: Power Hit Radio",
    "22: NRJ Tallinn",
    "23: MyHits",
    "24: Kuma FM 101.0",
    "25: Klassikaraadio",
    "26: Klara Nostalgia",
    "27: Klara Klassika",
    "28: Klara Jazz",
    "29: Kiss FM Pride",
    "30: Kiss FM Eurovision",
    "31: Kiss FM",
    "32: Kaguraadio",
    "33: Hitmix Estonia",
    "34: Hard FM Estonia",
    "35: Happy U",
    "36: ERR Raadio 2",
    "37: ERR Klassikaraadio",
    "38: Elmari tantsuõhtu",
    "39: Duo Rock",
    "40: Duo Party",
    "41: Duo Country",
    "42: DFM",
    "43: Äripäeva Raadio"
};
const int NUM_STATIONS = sizeof(stations) / sizeof(stations[0]);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Audio audio; // for external DAC
//Audio audio(true, I2S_DAC_CHANNEL_BOTH_EN); // for internal DAC ESP32
AiEsp32RotaryEncoder rotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, -1, ROTARY_ENCODER_STEPS);
Preferences preferences; // Loon Preferences objekti

int currentStation = 10;
int currentVolume = 10;  // Algväärtus helitugevusele 10 (0-20 vahemikus)
bool volumeMode = false; // False tähendab jaama valimist, true tähendab helitugevuse muutmist

char streamTitle[64] = "";  // Voogesituse pealkirja salvestamiseks

void IRAM_ATTR readEncoderISR() {
    rotaryEncoder.readEncoder_ISR();
}

// Loome map'i, et seostada Unicode väärtused fondifailis olevate märkidega
const std::unordered_map<uint16_t, char> FontMap = {
    {0x00E4, 0xC4}, // ä
    {0x00F6, 0xD6}, // ö
    {0x00F5, 0xD5}, // õ
    {0x00FC, 0xDC}, // ü
    {0x00C4, 0xA4}, // Ä
    {0x00D6, 0xB6}, // Ö
    {0x00D5, 0xB5}, // Õ
    {0x00DC, 0xBC}, // Ü
    {0x20AC, 0x84}, // €
};

// Dekodeerimise funktsioon
String decodeUTF8(const char *input) {
    String output = "";
    while (*input) {
        if ((*input & 0x80) == 0) {
            // ASCII märk
            output += *input;
        } else if ((*input & 0xE0) == 0xC0) {
            // 2-baidine UTF-8 märk
            uint8_t first = *input & 0x1F;
            uint8_t second = *(input + 1) & 0x3F;
            uint16_t code = (first << 6) | second;

            // Kasutame FontMap'i sümboli leidmiseks
            if (FontMap.find(code) != FontMap.end()) {
                output += FontMap.at(code);
            } else {
                output += '?'; // Tundmatu sümbol
            }
            input++; // Liigu järgmise baidi juurde
        }
        input++;
    }
    return output;
}

void setup() {
    Serial.begin(115200);
    rotaryEncoder.begin();
    rotaryEncoder.setup(readEncoderISR);

    // Loen salvestatud väärtused mälust
    preferences.begin("radio", false);
    currentStation = preferences.getInt("lastStation", 10);
    currentVolume = preferences.getInt("lastVolume", 10);
    preferences.end();

    rotaryEncoder.setBoundaries(0, NUM_STATIONS - 1, false);
    rotaryEncoder.setEncoderValue(currentStation);

    //pinMode(OLED_RST, OUTPUT);
    //digitalWrite(OLED_RST, LOW);
    //delay(20);
    //digitalWrite(OLED_RST, HIGH);
    Wire.begin(OLED_SDA, OLED_SCL);
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("OLED init failed"));
        return;
    }
    //display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    connectToWiFi();
    initializeAudio();
    updateDisplay();
}

void loop() {
    if (rotaryEncoder.encoderChanged()) {
        if (volumeMode) {
            currentVolume = constrain(rotaryEncoder.readEncoder(), 0, 20);
            audio.setVolume(currentVolume);
        } else {
            currentStation = constrain(rotaryEncoder.readEncoder(), 0, NUM_STATIONS - 1);
            connectToStation(currentStation);
        }
        savePreferences(); // Salvestan väärtused kohe, kui need muutuvad
        updateDisplay();
    }

    if (rotaryEncoder.isEncoderButtonClicked()) {
        volumeMode = !volumeMode;
        rotaryEncoder.setBoundaries(volumeMode ? 0 : 0, volumeMode ? 20 : NUM_STATIONS - 1, false);
        updateDisplay();  // Kuvatakse uus režiim OLED ekraanil
    }

    audio.loop();
    delay(10);
}

// Funktsioon salvestamiseks NVS-i
void savePreferences() {
    preferences.begin("radio", false);
    preferences.putInt("lastStation", currentStation);
    preferences.putInt("lastVolume", currentVolume);
    preferences.end();
}

// Audio tagasiside voogesituse pealkirja jaoks
void audio_showstreamtitle(const char *info) {
    //Serial.print("streamtitle: "); Serial.println(info);
    String decodedTitle = decodeUTF8(info);
    decodedTitle.toCharArray(streamTitle, sizeof(streamTitle));
    updateDisplay();  // Kuvatakse dekodeeritud voogesituse pealkiri
}
// Wifi'ga ühendamine protseduur wlanconfig.ino failist
void connectToWiFi() {
    while (!makeWLAN()) {
        Serial.println("Cannot connect :(");
        delay(1000);
    }
    Serial.println("Connected");
}

void initializeAudio() {
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT); // for external DAC
    audio.setVolume(currentVolume);
    connectToStation(currentStation);
}

void connectToStation(int stationIndex) {
    audio.stopSong();
    audio.connecttohost(stations[stationIndex]);
    updateDisplay();
}

// Uuenda ekraanikuva
void updateDisplay() {
    display.clearDisplay();
    display.setFont(&FreeMono6pt8b);
    display.setCursor(0, LINE1_Y);

    if (volumeMode) {
        display.print("Volume: ");
        display.println(currentVolume);
    } else {
        // display.print("Station: ");
        String stationName = decodeUTF8(stationNames[currentStation]);
        if (stationName.length() > 17) {
            stationName = stationName.substring(0, 17);
        }
        display.println(stationName);
        Serial.println(stationName);
        display.setCursor(0, LINE2_Y);
        display.println(streamTitle);
        Serial.println(streamTitle);
    }
    display.display();
}
