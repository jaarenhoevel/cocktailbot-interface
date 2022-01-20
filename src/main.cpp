#include <Arduino.h>
#include <ShiftRegister74HC595.h>
#include <HX711.h>
#include <FastLED.h>
#include <PixelAnimation.h>
#include <SerialCommands.h>

// SHIFT REGISTER //
#define SHIFT_REGISTER_COUNT        4

#define SHIFT_REGISTER_DATA_PIN     6
#define SHIFT_REGISTER_CLOCK_PIN    7
#define SHIFT_REGISTER_LATCH_PIN    8

// HX711 SCALE //
#define SCALE_DATA_PIN              2
#define SCALE_CLOCK_PIN             3

// LEDS //
#define LED_COUNT                   12
#define LED_TYPE                    WS2812B
#define LED_COLOR_ORDER             GRB
#define LED_DATA_PIN                4
#define LED_ANIMATION_FPS           50

// SERIAL CLI //
#define SERIAL_CLI_BAUDRATE         115200
#define SERIAL_CLI_LINE_BREAK       "\n"
#define SERIAL_CLI_SEPARATOR        " "

ShiftRegister74HC595<SHIFT_REGISTER_COUNT> shiftRegister(SHIFT_REGISTER_DATA_PIN, SHIFT_REGISTER_CLOCK_PIN, SHIFT_REGISTER_LATCH_PIN);

CRGB leds[LED_COUNT];
PixelAnimation *pixel;

char serialCommandBuffer[1024];
SerialCommands serialCommands_(&Serial, serialCommandBuffer, sizeof(serialCommandBuffer), SERIAL_CLI_LINE_BREAK, SERIAL_CLI_SEPARATOR);

void setup() {
  FastLED.addLeds<LED_TYPE, LED_DATA_PIN, LED_COLOR_ORDER>(leds, LED_COUNT);
  pixel = new PixelAnimation(leds, LED_ANIMATION_FPS);

  pixel->setBaseColor(CRGB::Red);

  Serial.begin(SERIAL_CLI_BAUDRATE);
}

void loop() {
  pixel->process();
}