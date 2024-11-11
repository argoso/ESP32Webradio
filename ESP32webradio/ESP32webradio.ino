// You need to use audio libary from: https://github.com/schreibfaul1/ESP32-audioI2S
// Inspirde by https://en.polluxlabs.net/esp32-esp8266-projects/esp32-internet-radio/ and
// https://www.az-delivery.de/en/blogs/azdelivery-blog-fur-arduino-und-raspberry-pi/internet-radio-mit-dem-esp32
#include <Arduino.h>
#include <WiFi.h>
#include <Audio.h>
#include <AiEsp32RotaryEncoder.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define I2S_DOUT 17
#define I2S_BCLK 16
#define I2S_LRC 4

#define ROTARY_ENCODER_A_PIN 12 //DT
#define ROTARY_ENCODER_B_PIN 13 //CLK
#define ROTARY_ENCODER_BUTTON_PIN 14 //SW
#define ROTARY_ENCODER_STEPS 4

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
#define OLED_SDA 2
#define OLED_SCL 15 
#define OLED_RST 16

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
    "http://icecast.err.ee/raadio2.opus",
    "http://icecast.err.ee/r2rock.opus",
    "http://icecast.err.ee/r2pop.opus",
    "https://icecast.err.ee/r2p.opus",
    "http://icecast.err.ee/r2chill.opus",
    "http://icecast.err.ee/r2alternatiiv.opus",
    "http://icecast.err.ee/r2eesti.opus",
    "https://ice.leviracloud.eu/phr96-aac",
    "https://edge03.cdn.bitflip.ee:8888/NRJ",
    "http://router.euddn.net/8103046e16b71d15d692b57c187875c7/myhits.aac",
    "http://5.39.16.62:8004/tre",
    "http://icecast.err.ee/klassikaraadio.opus",
    "http://icecast.err.ee/klaranostalgia.opus",
    "http://icecast.err.ee/klaraklassika.opus",
    "http://icecast.err.ee/klarajazz.opus",
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
    "Star FM Eesti",
    "Võmba FM",
    "Vikerraadio",
    "Tre Raadio Ring FM",
    "Star FM",
    "Sky-Radio",
    "Sky Plus",
    "Rock FM",
    "Relax FM Eesti",
    "Relax Cafe",
    "Raadio Kuku",
    "Raadio Kadi",
    "Raadio Elmar",
    "Raadio Duo",
    "Raadio 2 HQ",
    "R2Rock",
    "R2Pop",
    "R2p",
    "R2Chill",
    "R2Altpop",
    "R2 Eesti",
    "Power Hit Radio",
    "NRJ Tallinn",
    "MyHits",
    "Kuma FM 101.0",
    "Klassikaraadio",
    "Klara Nostalgia",
    "Klara Klassika",
    "Klara Jazz",
    "Kiss FM Pride",
    "Kiss FM Eurovision",
    "Kiss FM",
    "Kaguraadio",
    "Hitmix Estonia",
    "Hard FM Estonia",
    "Happy U",
    "ERR Raadio 2",
    "ERR Klassikaraadio",
    "Elmari tantsuõhtu",
    "Duo Rock",
    "Duo Party",
    "Duo Country",
    "DFM",
    "Äripäeva Raadio"
};
const int NUM_STATIONS = sizeof(stations) / sizeof(stations[0]);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Audio audio; // for external DAC
// Audio audio(true, I2S_DAC_CHANNEL_BOTH_EN); // for internal DAC
AiEsp32RotaryEncoder rotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, -1, ROTARY_ENCODER_STEPS);

int currentStation = 10;
int currentVolume = 10;  // Algväärtus helitugevusele 10 (0-20 vahemikus)
bool volumeMode = false; // False tähendab jaama valimist, true tähendab helitugevuse muutmist

char streamTitle[64] = "";  // Voogesituse pealkirja salvestamiseks

void IRAM_ATTR readEncoderISR() {
    rotaryEncoder.readEncoder_ISR();
}

void setup() {
    Serial.begin(115200);
    rotaryEncoder.begin();
    rotaryEncoder.setup(readEncoderISR);
    rotaryEncoder.setBoundaries(0, NUM_STATIONS - 1, false);

    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, LOW);
    delay(20);
    digitalWrite(OLED_RST, HIGH);
    Wire.begin(OLED_SDA, OLED_SCL);
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("OLED init failed"));
        return;
    }
    display.setTextSize(1);
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

void connectToWiFi() {
    while (!makeWLAN())
    {
      Serial.println("Cannot connect :(");
      delay(1000);
    }
    Serial.println("Connected");
}

void initializeAudio() {
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
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
    display.setCursor(0, 0);
    
    if (volumeMode) {
        // Helitugevuse kuvamine
        display.print("Volume: ");
        display.println(currentVolume);
    } else {
        // Jaama nimi ja voogesituse pealkiri
        display.print("Station: ");
        display.println(stationNames[currentStation]);
        display.setCursor(0, 16);
        display.println(streamTitle);  // Kuvab voogesituse pealkirja
    }
    
    display.display();
}

// Audio tagasiside voogesituse pealkirja jaoks
void audio_showstreamtitle(const char *info) {
    Serial.print("streamtitle: "); Serial.println(info);
    strncpy(streamTitle, info, sizeof(streamTitle) - 1);
    streamTitle[sizeof(streamTitle) - 1] = '\0'; // Tagab null-lõpetuse
    updateDisplay();  // Kuvatakse voogesituse pealkiri
}
