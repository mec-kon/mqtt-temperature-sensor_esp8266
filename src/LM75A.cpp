/*
 * \brief I2C LM75A temperature sensor library (implementation)
 *
 * \author Quentin Comte-Gaz <quentin@comte-gaz.com>
 * \date 8 July 2016 & 14 January 2018
 * \license MIT License (contact me if too restrictive)
 * \copyright Copyright (c) 2016 Quentin Comte-Gaz
 * \version 1.1
 */

#include <Wire.h>

#include "LM75A.h"

using namespace LM75AConstValues;

LM75A::LM75A()
{
  i2c_device_address = LM75A_BASE_ADDRESS;

  Wire.begin(I2C_SDA, I2C_SCL);
}

float LM75A::fahrenheitToDegrees(float temperature_in_fahrenheit)
{
  return ((temperature_in_fahrenheit - 32.0) / 1.8);
}

float LM75A::degreesToFahrenheit(float temperature_in_degrees)
{
  return ((temperature_in_degrees * 1.8) + 32.0);
}

float LM75A::getTemperatureInFahrenheit() const
{
  float temperature_in_degrees = getTemperatureInDegrees();

  if (temperature_in_degrees == INVALID_LM75A_TEMPERATURE) {
    return INVALID_LM75A_TEMPERATURE;
  }

  return degreesToFahrenheit(temperature_in_degrees);
}

float LM75A::getTemperatureInDegrees() const
{
  uint16_t real_result = INVALID_LM75A_TEMPERATURE;
  uint16_t i2c_received = 0;

  // Go to temperature data register
  Wire.beginTransmission(i2c_device_address);
  Wire.write(LM75A_REG_ADDR_TEMP);
  if(Wire.endTransmission()) {
    // Transmission error
    return real_result;
  }

  // Get content
  if (Wire.requestFrom(i2c_device_address, 2)) {
    Wire.readBytes((uint8_t*)&i2c_received, 2);
  } else {
    // Can't read temperature
    return real_result;
  }

  // Modify the value (only 11 MSB are relevant if swapped)
  int16_t refactored_value;
  uint8_t* ptr = (uint8_t*)&refactored_value;

  // Swap bytes
  *ptr = *((uint8_t*)&i2c_received + 1);
  *(ptr + 1) = *(uint8_t*)&i2c_received;

  // Shift data (left-aligned)
  refactored_value >>= 5;

  float real_value;
  if (refactored_value & 0x0400) {
    // When sign bit is set, set upper unused bits, then 2's complement
    refactored_value |= 0xf800;
    refactored_value = ~refactored_value + 1;
    real_value = (float)refactored_value * (-1) * LM75A_DEGREES_RESOLUTION;
  }
  else {
    real_value = (float)refactored_value *  LM75A_DEGREES_RESOLUTION;
  }

  return real_value;
}
