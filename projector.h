#pragma once

enum ProjectorState {
  STATE_STANDBY = 0,
  STATE_START_UP = 1,
  STATE_STARTUP_LAMP = 2,
  STATE_POWER_ON = 3,
  STATE_COOLING_1 = 4,
  STATE_COOLING_2 = 5,
  STATE_SAVING_COOLING_1 = 6,
  STATE_SAVING_COOLING_2 = 7,
  STATE_SAVING_STANDBY = 8,
  STATE_UNKNOWN,
};

ProjectorState GetProjectorState();

int TurnOnProjector();

int TurnOffProjector();

