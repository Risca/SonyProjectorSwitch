#include "projector.h"

// LED pin number
#define LED_PIN 13

byte currentValue = 0;
unsigned long updateTimestamp = 0;
enum Command {
  CMD_TURN_ON_PROJECTOR,
  CMD_TURN_OFF_PROJECTOR,
  CMD_NONE,
};
static enum Command currentCommand = CMD_NONE;

// next macro sets up the Z-Uno channels
// in this example we set up 1 switch binary channel
// you can read more on http://z-uno.z-wave.me/Reference/ZUNO_SWITCH_BINARY/
ZUNO_SETUP_CHANNELS(ZUNO_SWITCH_BINARY(getter, setter));

static void UpdateProjectorState()
{
  byte lastValue = currentValue;
  switch (GetProjectorState()) {
  case STATE_POWER_ON:
    currentValue = 255;
    digitalWrite (LED_PIN, HIGH);
    break;
  default:
    currentValue = 0;
    digitalWrite(LED_PIN, LOW);
    break;
  }
  if (currentValue != lastValue) {
    zunoSendReport(1);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.print("Setting up peripherals... ");
  Serial0.begin(38400);
  Serial0.setTimeout(3000);
  pinMode(LED_PIN, OUTPUT); // setup pin as output
  Serial.println("done.");
  Serial.print("Querying projector state... ");
  /*
   * Determine last value
   */
  switch (GetProjectorState()) {
  case STATE_POWER_ON:
    currentValue = 255;
    break;
  default:
    currentValue = 0;
    break;
  }
  Serial.print("done.");
  Serial.println(currentValue);
  Serial.println("Ready, let's go!");
}

void loop() {
  // every second
  if(millis() - updateTimestamp > 3000) {
    // send data to channel
    Serial.print("Updating projector state... ");
    UpdateProjectorState();
    Serial.println("done.");
    updateTimestamp = millis();
  }
  
  switch (currentCommand) {
  case CMD_TURN_ON_PROJECTOR:
  case CMD_TURN_OFF_PROJECTOR: {
    enum Command cmd = currentCommand;
    currentCommand = CMD_NONE;
    switch (cmd) {
    case CMD_TURN_ON_PROJECTOR:
      if (!TurnOnProjector()) {
        currentValue = 0;
        zunoSendReport(1);
      }
      break;
    
    case CMD_TURN_OFF_PROJECTOR:
      if (!TurnOffProjector()) {
        currentValue = 255;
        zunoSendReport(1);
      }
    default:
      break;
    }
    updateTimestamp = millis();
    break;
  }

  case CMD_NONE:
  default:
    break;
  }
}

void setter (byte value) {
  currentValue = value;
  if (value != 0) {
    currentCommand = CMD_TURN_ON_PROJECTOR;
  }
  else {
    currentCommand = CMD_TURN_OFF_PROJECTOR;
  }
  Serial.print("Got new value: ");
  Serial.println(value);
}

byte getter () {
  return currentValue;
}


