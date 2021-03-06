#include <Arduino.h>
#include <ShiftRegister74HC595.h>
#include <HX711.h>
#include <FastLED.h>
#include <PixelAnimation.h>
#include <RelayController.cpp>
#include <SerialCommands.h>
#include <EEPROM.h>

// SHIFT REGISTER //
#define SHIFT_REGISTER_COUNT        8

#define SHIFT_REGISTER_DATA_PIN     6
#define SHIFT_REGISTER_CLOCK_PIN    7
#define SHIFT_REGISTER_LATCH_PIN    8

// RELAYS //
#define RELAY_PIN_INVERTED          true // Relays use low level trigger

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
#define SERIAL_CLI_BAUDRATE         9600
#define SERIAL_CLI_LINE_BREAK       "\n"
#define SERIAL_CLI_SEPARATOR        " "

// CALIBRATION STORAGE //
#define LOADCELL_SCALE_ADDRESS      0 // Uses 4 Bytes of EEPROM

CRGB leds[LED_COUNT];
PixelAnimation *pixel;
RelayController<SHIFT_REGISTER_COUNT> *relayController;
HX711 loadcell;

float loadcellScale;

char cliLineBreak[] = SERIAL_CLI_LINE_BREAK;
char cliSeperator[] = SERIAL_CLI_SEPARATOR;
char serialCommandBuffer[256];
SerialCommands serialCommands(&Serial, serialCommandBuffer, sizeof(serialCommandBuffer), cliLineBreak, cliSeperator);

void cmdUnrecognized(SerialCommands* sender, const char* cmd);
void cmdHandshake(SerialCommands* sender);
void cmdSetRelay(SerialCommands* sender);
void cmdGetSensor(SerialCommands* sender);
void cmdCalibrateSensor(SerialCommands* sender);
void cmdSetLight(SerialCommands* sender);

SerialCommand cmdHandshake_("handshake", cmdHandshake);
SerialCommand cmdSetRelay_("set_relay", cmdSetRelay);
SerialCommand cmdGetSensor_("get_sensor", cmdGetSensor);
SerialCommand cmdCalibrateSensor_("calibrate_sensor", cmdCalibrateSensor);
SerialCommand cmdSetLight_("set_light", cmdSetLight);

float readFloat(unsigned int addr);
void writeFloat(unsigned int addr, float x);

void setup() {
  // Initiate components
  relayController = new RelayController<SHIFT_REGISTER_COUNT>(SHIFT_REGISTER_DATA_PIN, SHIFT_REGISTER_CLOCK_PIN, SHIFT_REGISTER_LATCH_PIN, RELAY_PIN_INVERTED);

  FastLED.addLeds<LED_TYPE, LED_DATA_PIN, LED_COLOR_ORDER>(leds, LED_COUNT);
  pixel = new PixelAnimation(leds, LED_ANIMATION_FPS);
  pixel->setBaseColor(CRGB::Red);

  // Get scale from EEPROM
  loadcellScale = readFloat(LOADCELL_SCALE_ADDRESS);
  if (isnan(loadcellScale)) loadcellScale = 1000.f;

  // Setup loadcell
  loadcell.begin(SCALE_DATA_PIN, SCALE_CLOCK_PIN);
  loadcell.set_scale(loadcellScale);
  loadcell.tare(10);

  // Setup CLI
  Serial.begin(SERIAL_CLI_BAUDRATE);

  serialCommands.SetDefaultHandler(cmdUnrecognized);
  serialCommands.AddCommand(&cmdHandshake_);
  serialCommands.AddCommand(&cmdSetRelay_);
  serialCommands.AddCommand(&cmdGetSensor_);
  serialCommands.AddCommand(&cmdCalibrateSensor_);
  serialCommands.AddCommand(&cmdSetLight_);

  while(!Serial);
}

void loop() {
  serialCommands.ReadSerial();
  pixel->process();
}

// Serial Commands //

void cmdUnrecognized(SerialCommands* sender, const char* cmd) {
	sender->GetSerial()->print(sender->Next());
  sender->GetSerial()->print(" ");
  
  sender->GetSerial()->print("ERROR Unrecognized command [");
	sender->GetSerial()->print(cmd);
	sender->GetSerial()->println("]!");
}

void cmdHandshake(SerialCommands* sender) {
	sender->GetSerial()->print(sender->Next());
  sender->GetSerial()->print(" ");
  
  sender->GetSerial()->println("OK :)");
}

void cmdSetRelay(SerialCommands* sender) {
  sender->GetSerial()->print(sender->Next());
  sender->GetSerial()->print(" ");

  bool close = false;
  char* parameterString = sender->Next();
  
  if (parameterString == NULL) {
    sender->GetSerial()->println("ERROR No action specified!");
    return;
  }

  if (strcmp(parameterString, "close") == 0) {
    close = true;
  } else if (strcmp(parameterString, "open") != 0) {
    sender->GetSerial()->println("ERROR No valid action specified!");
    return;  
  }

  parameterString = sender->Next();

  if (parameterString == NULL) {
    sender->GetSerial()->println("ERROR No relay ID specified!");
    return;
  }

  uint8_t relayId = atoi(parameterString);

  if (relayId == 255) {
    relayController->openAllRelays();
    sender->GetSerial()->println("OK");
    return;
  }

  if (relayController->setRelay(relayId, close)) {
    sender->GetSerial()->println("OK");
  } else {
    sender->GetSerial()->println("ERROR Invalid relay ID!");
  }
}

void cmdGetSensor(SerialCommands* sender) {
  sender->GetSerial()->print(sender->Next());
  sender->GetSerial()->print(" ");

  char* parameterString = sender->Next();

  if (parameterString == NULL) {
    sender->GetSerial()->println("ERROR No sensor specified!");
    return;
  }

  if (strcmp(parameterString, "scale") == 0) {
    if (loadcell.wait_ready_timeout(250)) {
      sender->GetSerial()->print("OK ");
      sender->GetSerial()->println(round(loadcell.get_units(1)));
      return;
    } else {
      sender->GetSerial()->println("ERROR Scale not ready!");
      return;
    }
  }

  if (strcmp(parameterString, "temperature") == 0) {
    sender->GetSerial()->print("OK ");
    sender->GetSerial()->println("-1");
    return;
  }

  sender->GetSerial()->println("ERROR Invalid sensor!");
}

void cmdCalibrateSensor(SerialCommands* sender) {
  sender->GetSerial()->print(sender->Next());
  sender->GetSerial()->print(" ");

  char* parameterString = sender->Next();

  if (parameterString == NULL) {
    sender->GetSerial()->println("ERROR No sensor specified!");
    return;
  }

  if (strcmp(parameterString, "scale") == 0) {
    parameterString = sender->Next();

    if (strcmp(parameterString, "tare") == 0) {
      loadcell.tare(10);
      
      sender->GetSerial()->println("OK");
      return;
    }

    if (strcmp(parameterString, "weight") == 0) {
      parameterString = sender->Next();

      if (parameterString == NULL) {
        sender->GetSerial()->println("ERROR No weight specified!");
        return;
      }

      uint32_t newWeight = atoi(parameterString);

      double currentWeight = loadcell.get_value(10);
      loadcellScale = currentWeight / (newWeight * 1.f);
      loadcell.set_scale(loadcellScale);

      writeFloat(LOADCELL_SCALE_ADDRESS, loadcellScale);

      sender->GetSerial()->println("OK");
      return;  
    }

    sender->GetSerial()->println("ERROR No valid calibration specified!");
    return;
  }

  sender->GetSerial()->println("ERROR No valid sensor specified!");
}

void cmdSetLight(SerialCommands* sender) {
  sender->GetSerial()->print(sender->Next());
  sender->GetSerial()->print(" ");

  char* parameterString = sender->Next();

  if (parameterString == NULL) {
    sender->GetSerial()->println("ERROR No action specified!");
    return;
  }

  if (strcmp(parameterString, "color") == 0) {
    uint8_t color[3];
    
    for (int i = 0; i < 3; i ++) {
      parameterString = sender->Next();
      
      if (parameterString == NULL) {
        sender->GetSerial()->println("ERROR No color specified!");
        return;
      }

      color[i] = atoi(parameterString);
    }

    pixel->setBaseColor(CRGB(color[0], color[1], color[2]));

    sender->GetSerial()->println("OK");
    return;
  }

  if (strcmp(parameterString, "intensity") == 0) {
    parameterString = sender->Next();
      
    if (parameterString == NULL) {
      sender->GetSerial()->println("ERROR No color specified!");
      return;
    }

    uint8_t intensity = atoi(parameterString);
    pixel->setBrightness(intensity);

    sender->GetSerial()->println("OK");
    return;
  }

  sender->GetSerial()->println("ERROR No valid action specified!");
}

float readFloat(unsigned int addr) {
  union {
    byte b[4];
    float f;
  } data;
  for (int i = 0; i < 4; i++) {
    data.b[i] = EEPROM.read(addr + i);
  }
  return data.f;
}

void writeFloat(unsigned int addr, float x) {
  union {
    byte b[4];
    float f;
  } data;
  data.f = x;
  for (int i = 0; i < 4; i++) {
    EEPROM.write(addr + i, data.b[i]);
  }
}