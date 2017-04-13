#include <Arduino.h>

//ESP8266 libs
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArtnetWifi.h>

//FastLED settings
#define  FASTLED_ESP8266_RAW_PIN_ORDER
#include "FastLED.h"
#define NUM_LEDS 24
#define DATA_PIN 0
#define CLOCK_PIN 2
#define CHIPSET WS2801
#define COLOR_ORDER RGB
#define BRIGHTNESS 255
CRGB leds[NUM_LEDS];

//Wifi settings
const char* ssid = "aether2G";
const char* password = "0,1%Fett";

// Artnet settings
ArtnetWifi artnet;
const int startUniverse = 1;

// Check if we got all universes
const int numberOfChannels = NUM_LEDS * 3;
const int maxUniverses = numberOfChannels / 512 + ((numberOfChannels % 512) ? 1 : 0);
bool universesReceived[maxUniverses];
bool sendFrame = 1;
int previousDataLength = 0;

// connect to wifi – returns true if successful or false if not
boolean ConnectWifi(void)
{
  boolean state = true;
  int i = 0;

  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Connecting to WiFi");

  // Wait for connection
  Serial.print("Connecting ");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("*");
    if (i > 20){
      state = false;
      break;
    }
    i++;
  }
  if (state){
    Serial.println("");
    Serial.print("Connected to: ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("Connection failed.");
  }
  return state;
}

//Artnet callback function
void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{
  sendFrame = 1;
  // set brightness of the whole strip
  if (universe == 15)
  {
    fill_solid( leds, NUM_LEDS, CRGB::Black);
  }

  // Store which universe has got in
  if ((universe - startUniverse) < maxUniverses)
    universesReceived[universe - startUniverse] = 1;

  for (int i = 0 ; i < maxUniverses ; i++)
  {
    if (universesReceived[i] == 0)
    {
      //Serial.println("Broke");
      sendFrame = 0;
      break;
    }
  }

  // read universe and put into the right part of the display buffer
  for (int i = 0; i < length / 3; i++)
  {
    int led = i + (universe - startUniverse) * (previousDataLength / 3);
    if (led < NUM_LEDS)
      leds[led] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
  }
  previousDataLength = length;

  if (sendFrame)
  {
    FastLED.show();
    // Reset universeReceived to 0
    memset(universesReceived, 0, maxUniverses);
  }
}


void RGBtest(){
  for (int i = 0; i < NUM_LEDS; ++i){
    leds[i] = CRGB::Red;
    FastLED.show();
    delay(100);
    leds[i] = CRGB::Black;
    FastLED.show();
    delay(100);
    leds[i] = CRGB::Green;
    FastLED.show();
    delay(100);
    leds[i] = CRGB::Black;
    FastLED.show();
    delay(100);
    leds[i] = CRGB::Blue;
    FastLED.show();
    delay(100);
    leds[i] = CRGB::Black;
    FastLED.show();
  }
}

void setup() {
  //LED object
  FastLED.addLeds<CHIPSET, DATA_PIN, CLOCK_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  //set total brightness
  FastLED.setBrightness(BRIGHTNESS);
  //start Serial for monitor
  Serial.begin(9600);
  //connect to Wifi
  ConnectWifi();
  //init artnet object
  artnet.begin();
  // this will be called for each packet received
  artnet.setArtDmxCallback(onDmxFrame);
  //LED test
  RGBtest();
}



void loop() {
  artnet.read();
}
