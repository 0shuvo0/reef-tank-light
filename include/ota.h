#pragma once
#include <Arduino.h>

// Returns true if a newer firmware is available (based on version.json)
bool checkFirmwareUpdate();

// Downloads and flashes the firmware (only call if checkFirmwareUpdate() returned true)
bool updateFirmware();