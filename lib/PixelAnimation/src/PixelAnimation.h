#include <FastLED.h>

class PixelAnimation {
    public:
        PixelAnimation(CRGB* leds, uint8_t fps);
        void process();
        void setBaseColor(CRGB color);
        void setTransitionTime(uint16_t time);
        void setBrightness(uint8_t value);
    private:
        uint8_t pixelCount;
        uint16_t transitionTime;
        
        uint16_t timePerFrame;
        uint64_t lastFrame;
        
        CRGB baseColor;
        CRGB targetColor;
        uint16_t colorTransitionProgress;

        uint8_t brightness;
        uint8_t targetBrightness;
        uint16_t brightnessTransitionProgress;

        CRGB *leds;
};