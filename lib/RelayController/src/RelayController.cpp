#include <Arduino.h>
#include <RelayController.h>
#include <ShiftRegister74HC595.h>

template<uint8_t NUM>
RelayController<NUM>::RelayController(uint8_t dataPin, uint8_t clockPin, uint8_t latchPin, bool invertRelays) {
    shiftRegister = new ShiftRegister74HC595<NUM>(dataPin, clockPin, latchPin);
    relayCount = NUM * 8;
    inverted = invertRelays;

    openAllRelays();
}

template<uint8_t NUM>
bool RelayController<NUM>::setRelay(uint8_t relayId, bool closed) {
    if (relayId >= relayCount) return false;

    shiftRegister->set(relayId, closed != inverted);

    return true;
}

template<uint8_t NUM>
void RelayController<NUM>::openAllRelays() {
    if (inverted) shiftRegister->setAllHigh();
    else shiftRegister->setAllLow();
}