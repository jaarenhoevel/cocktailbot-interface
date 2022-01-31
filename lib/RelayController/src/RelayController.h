#include <ShiftRegister74HC595.h>

template <uint8_t NUM>
class RelayController {
    public:
        RelayController(uint8_t dataPin, uint8_t clockPin, uint8_t latchPin, bool invertRelays);
        bool setRelay(uint8_t relayId, bool closed);
        void openAllRelays();
    private:
        ShiftRegister74HC595<NUM> *shiftRegister;
        uint8_t relayCount;
        bool inverted;
};