#include <Arduino.h>
#include <PixelAnimation.h>
#include <FastLED.h>

PixelAnimation::PixelAnimation(CRGB *leds, uint8_t fps) {
    timePerFrame = 1000 / fps;
    lastFrame = 0;

    pixelCount = sizeof(leds) / sizeof(leds[0]);
}

void PixelAnimation::process() {
    if (millis() - lastFrame < timePerFrame) return;

    lastFrame = millis();
    FastLED.show();
}

void PixelAnimation::setBaseColor(CRGB color) {
    targetColor = color;
}

void PixelAnimation::setBrightness(uint8_t value) {
    targetBrightness = value;
}

void PixelAnimation::setTransitionTime(uint16_t time) {
    transitionTime = time;
}