/*
 * Example sketch for reporting on readings from the LD2410 using whatever settings are currently configured.
 * 
 */


#define MONITOR_SERIAL Serial
#define RADAR_SERIAL Serial2

#include <ld2410.h>

ld2410 radar;

uint32_t lastReading = 0;
bool radarConnected = false;

void setup(void)
{
  MONITOR_SERIAL.begin(115200); //Feedback over Serial Monitor

  RADAR_SERIAL.begin(256000); //UART for monitoring the radar
  while (!radar.begin(RADAR_SERIAL)) {
     MONITOR_SERIAL.println(F("not connected"));
  }

  delay(500);
  
  MONITOR_SERIAL.print(F("LD2410 radar sensor initialising: "));
 
  MONITOR_SERIAL.println(F("OK"));
  MONITOR_SERIAL.print(F("LD2410 firmware version: "));
  MONITOR_SERIAL.print(radar.firmware_major_version);
  MONITOR_SERIAL.print('.');
  MONITOR_SERIAL.print(radar.firmware_minor_version);
  MONITOR_SERIAL.print('.');
  MONITOR_SERIAL.println(radar.firmware_bugfix_version, HEX);
}

void loop()
{
  radar.read();
  if(radar.isConnected() && millis() - lastReading > 1000)  //Report every 1000ms
  {
    lastReading = millis();
    if(radar.presenceDetected())
    {
      if(radar.stationaryTargetDetected())
      {
        Serial.print(F("Stationary target: "));
        Serial.print(radar.stationaryTargetDistance());
        Serial.print(F("cm energy:"));
        Serial.print(radar.stationaryTargetEnergy());
        Serial.print(' ');
      }
      if(radar.movingTargetDetected())
      {
        Serial.print(F("Moving target: "));
        Serial.print(radar.movingTargetDistance());
        Serial.print(F("cm energy:"));
        Serial.print(radar.movingTargetEnergy());
      }
      Serial.println();
    }
    else
    {
      Serial.println(F("No target"));
    }
  }
}
