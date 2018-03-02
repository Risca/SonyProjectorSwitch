#include "projector.h"
#include <Arduino.h>
#include <HardwareSerial.h>

#define START_CODE ((char)0xA9)
#define ON_CODE    ((char)0x2E)
#define OFF_CODE   ((char)0x2F)
#define END_CODE   ((char)0x9A)

static ProjectorState ParseStateResponse(char *buf)
{
  ProjectorState state = STATE_UNKNOWN;
  if (buf[0] == START_CODE &&
      buf[1] == 0x01 &&
      buf[2] == 0x02 &&
      buf[3] == 0x02 &&
      buf[4] == 0x00 &&
      buf[6] == (buf[5] | 0x03) &&
      buf[7] == END_CODE)
  {
    state = (ProjectorState)buf[5];
  }
  else {
    Serial.println("bad state!");
    Serial.println(buf[0], HEX);
    Serial.println(buf[1], HEX);
    Serial.println(buf[2], HEX);
    Serial.println(buf[3], HEX);
    Serial.println(buf[4], HEX);
    Serial.println(buf[5], HEX);
    Serial.println(buf[6], HEX);
    Serial.println(buf[7], HEX);
  }
  return state;
}

static size_t SerialWriteBuf(const char *buffer, const size_t size)
{
  size_t bytesLeft = size;
  while (bytesLeft) {
    Serial0.write((uint8_t)*buffer);
    buffer++;
    bytesLeft--;
  }
  return size - bytesLeft;
}

static int timedRead(unsigned long timeout)
{
  unsigned long start = millis();
  do {
    if (Serial0.available()) {
      return Serial0.read();
    }
  } while(millis() - start < timeout);
  return -1;     // -1 indicates timeout
}

static size_t SerialReadBuf(char *buffer, const size_t size, unsigned long timeout)
{
  size_t count = 0;
  while (count < size) {
    int c = timedRead(timeout);
    if (c < 0) {
      break;
    }
    *buffer++ = (char)c;
    count++;
  }
  return count;
}

static ProjectorState ChangePowerState(bool on)
{
  char buf[8] = {START_CODE, 0x17, OFF_CODE, 0x00, 0x00, 0x00, 0x3F, END_CODE };
  if (on) {
    buf[2] = ON_CODE;
  }
  else {
    buf[2] = OFF_CODE;
  }
  const ProjectorState wantedState = on ? STATE_POWER_ON : STATE_STANDBY;
  ProjectorState state = GetProjectorState();
  if (state != wantedState) {
    if (SerialWriteBuf(buf, 8) == 8) {
      delay(200);
      unsigned long start = millis();
      do {
        state = GetProjectorState();
      } while (!(state == wantedState || state == STATE_UNKNOWN) && (millis() - start < 30000));
    }
  }
  return state;
}

ProjectorState GetProjectorState()
{
  const char req[8] = { START_CODE, 0x01, 0x02, 0x01, 0x00, 0x00, 0x03, END_CODE };
  char buf[8];
  SerialWriteBuf(req, 8);
  delay(200);
  
  size_t rv = SerialReadBuf(buf, 8, 4000);
  if (rv == 8) {
    return ParseStateResponse(buf);
  }
  else {
    Serial.println  ("no resp!");
  }
  return STATE_UNKNOWN;
}

int TurnOnProjector()
{
  Serial.print("Turning ON projector... ");
  const ProjectorState state = ChangePowerState(true);
  if (state == STATE_POWER_ON) {
    Serial.println("done.");
    return 1;
  }
  else {
    Serial.println("error!");
    return 0;
  }
}

int TurnOffProjector()
{
  Serial.print("Turning OFF projector... ");
  const ProjectorState state = ChangePowerState(false);
  if (state == STATE_STANDBY) {
    Serial.println("done.");
    return 1;
  }
  else {
    Serial.println("error!");
    return 0;
  }
}

