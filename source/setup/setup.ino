/*
 * Example sketch to show using configuration commands on the LD2410.
 */

#define MONITOR_SERIAL Serial
#define RADAR_SERIAL Serial2

#include <ld2410.h>

ld2410 radar;
bool engineeringMode = false;
String command;

uint32_t lastReading = 0;
bool radarConnected = false;

void setup(void)
{
  MONITOR_SERIAL.begin(115200); //Feedback over Serial Monitor
  RADAR_SERIAL.begin(256000); //UART for monitoring the radar
  delay(500);
  MONITOR_SERIAL.print(F("LD2410 radar sensor initialising: "));
  while (!radar.begin(RADAR_SERIAL)) {
     MONITOR_SERIAL.println(F("not connected"));
  }

  MONITOR_SERIAL.println(F("OK"));
  MONITOR_SERIAL.print(F("LD2410 firmware version: "));
  MONITOR_SERIAL.print(radar.firmware_major_version);
  MONITOR_SERIAL.print('.');
  MONITOR_SERIAL.print(radar.firmware_minor_version);
  MONITOR_SERIAL.print('.');
  MONITOR_SERIAL.println(radar.firmware_bugfix_version, HEX);
  MONITOR_SERIAL.println(F("Supported commands\nread: read current values from the sensor\nreadconfig: read the configuration from the sensor\nsetmaxvalues <motion gate> <stationary gate> <inactivitytimer>\nsetsensitivity <gate> <motionsensitivity> <stationarysensitivity>\nenableengineeringmode: enable engineering mode\ndisableengineeringmode: disable engineering mode\nrestart: restart the sensor\nreadversion: read firmware version\nfactoryreset: factory reset the sensor\n"));
}

void loop()
{
  radar.read(); //Always read frames from the sensor
  if(MONITOR_SERIAL.available())
  {
    char typedCharacter = MONITOR_SERIAL.read();
    if(typedCharacter == '\r' || typedCharacter == '\n')
    {
      command.trim();
      if(command.equals("read"))
      {
        command = "";
        MONITOR_SERIAL.print(F("Reading from sensor: "));
        if(radar.isConnected())
        {
          MONITOR_SERIAL.println(F("OK"));
          if(radar.presenceDetected())
          {
            if(radar.stationaryTargetDetected())
            {
              MONITOR_SERIAL.print(F("Stationary target: "));
              MONITOR_SERIAL.print(radar.stationaryTargetDistance());
              MONITOR_SERIAL.print(F("cm energy: "));
              MONITOR_SERIAL.println(radar.stationaryTargetEnergy());
            }
            if(radar.movingTargetDetected())
            {
              MONITOR_SERIAL.print(F("Moving target: "));
              MONITOR_SERIAL.print(radar.movingTargetDistance());
              MONITOR_SERIAL.print(F("cm energy: "));
              MONITOR_SERIAL.println(radar.movingTargetEnergy());
            }
          }
          else
          {
            MONITOR_SERIAL.println(F("nothing detected"));
          }
        }
        else
        {
          MONITOR_SERIAL.println(F("failed to read"));
        }
      }
      else if(command.equals("readconfig"))
      {
        command = "";
        MONITOR_SERIAL.print(F("Reading configuration from sensor: "));
        if(radar.requestCurrentConfiguration())
        {
          MONITOR_SERIAL.println(F("OK"));
          MONITOR_SERIAL.print(F("Maximum gate ID: "));
          MONITOR_SERIAL.println(radar.max_gate);
          MONITOR_SERIAL.print(F("Maximum gate for moving targets: "));
          MONITOR_SERIAL.println(radar.max_moving_gate);
          MONITOR_SERIAL.print(F("Maximum gate for stationary targets: "));
          MONITOR_SERIAL.println(radar.max_stationary_gate);
          MONITOR_SERIAL.print(F("Idle time for targets: "));
          MONITOR_SERIAL.println(radar.sensor_idle_time);
          MONITOR_SERIAL.println(F("Gate sensitivity"));
          for(uint8_t gate = 0; gate <= radar.max_gate; gate++)
          {
            MONITOR_SERIAL.print(F("Gate "));
            MONITOR_SERIAL.print(gate);
            MONITOR_SERIAL.print(F(" moving targets: "));
            MONITOR_SERIAL.print(radar.motion_sensitivity[gate]);
            MONITOR_SERIAL.print(F(" stationary targets: "));
            MONITOR_SERIAL.println(radar.stationary_sensitivity[gate]);
          }
        }
        else
        {
          MONITOR_SERIAL.println(F("Failed"));
        }
      }
      else if(command.startsWith("setmaxvalues"))
      {
        uint8_t firstSpace = command.indexOf(' ');
        uint8_t secondSpace = command.indexOf(' ',firstSpace + 1);
        uint8_t thirdSpace = command.indexOf(' ',secondSpace + 1);
        uint8_t newMovingMaxDistance = (command.substring(firstSpace,secondSpace)).toInt();
        uint8_t newStationaryMaxDistance = (command.substring(secondSpace,thirdSpace)).toInt();
        uint16_t inactivityTimer = (command.substring(thirdSpace,command.length())).toInt();
        if(newMovingMaxDistance > 0 && newStationaryMaxDistance > 0 && newMovingMaxDistance <= 8 && newStationaryMaxDistance <= 8)
        {
          MONITOR_SERIAL.print(F("Setting max values to gate "));
          MONITOR_SERIAL.print(newMovingMaxDistance);
          MONITOR_SERIAL.print(F(" moving targets, gate "));
          MONITOR_SERIAL.print(newStationaryMaxDistance);
          MONITOR_SERIAL.print(F(" stationary targets, "));
          MONITOR_SERIAL.print(inactivityTimer);
          MONITOR_SERIAL.print(F("s inactivity timer: "));
          command = "";
          if(radar.setMaxValues(newMovingMaxDistance, newStationaryMaxDistance, inactivityTimer))
          {
            MONITOR_SERIAL.println(F("OK, now restart to apply settings"));
          }
          else
          {
            MONITOR_SERIAL.println(F("failed"));
          }
        }
        else
        {
          MONITOR_SERIAL.print(F("Can't set distances to "));
          MONITOR_SERIAL.print(newMovingMaxDistance);
          MONITOR_SERIAL.print(F(" moving "));
          MONITOR_SERIAL.print(newStationaryMaxDistance);
          MONITOR_SERIAL.println(F(" stationary, try again"));
          command = "";
        }
      }
      else if(command.startsWith("setsensitivity"))
      {
        uint8_t firstSpace = command.indexOf(' ');
        uint8_t secondSpace = command.indexOf(' ',firstSpace + 1);
        uint8_t thirdSpace = command.indexOf(' ',secondSpace + 1);
        uint8_t gate = (command.substring(firstSpace,secondSpace)).toInt();
        uint8_t motionSensitivity = (command.substring(secondSpace,thirdSpace)).toInt();
        uint8_t stationarySensitivity = (command.substring(thirdSpace,command.length())).toInt();
        if(motionSensitivity >= 0 && stationarySensitivity >= 0 && motionSensitivity <= 100 && stationarySensitivity <= 100)
        {
          MONITOR_SERIAL.print(F("Setting gate "));
          MONITOR_SERIAL.print(gate);
          MONITOR_SERIAL.print(F(" motion sensitivity to "));
          MONITOR_SERIAL.print(motionSensitivity);
          MONITOR_SERIAL.print(F(" & stationary sensitivity to "));
          MONITOR_SERIAL.print(stationarySensitivity);
          MONITOR_SERIAL.println(F(": "));
          command = "";
          if(radar.setGateSensitivityThreshold(gate, motionSensitivity, stationarySensitivity))
          {
            MONITOR_SERIAL.println(F("OK, now restart to apply settings"));
          }
          else
          {
            MONITOR_SERIAL.println(F("failed"));
          }
        }
        else
        {
          MONITOR_SERIAL.print(F("Can't set gate "));
          MONITOR_SERIAL.print(gate);
          MONITOR_SERIAL.print(F(" motion sensitivity to "));
          MONITOR_SERIAL.print(motionSensitivity);
          MONITOR_SERIAL.print(F(" & stationary sensitivity to "));
          MONITOR_SERIAL.print(stationarySensitivity);
          MONITOR_SERIAL.println(F(", try again"));
          command = "";
        }
      }
      else if(command.equals("enableengineeringmode"))
      {
        command = "";
        MONITOR_SERIAL.print(F("Enabling engineering mode: "));
        if(radar.requestStartEngineeringMode())
        {
          MONITOR_SERIAL.println(F("OK"));
        }
        else
        {
          MONITOR_SERIAL.println(F("failed"));
        }
      }
      else if(command.equals("disableengineeringmode"))
      {
        command = "";
        MONITOR_SERIAL.print(F("Disabling engineering mode: "));
        if(radar.requestEndEngineeringMode())
        {
          MONITOR_SERIAL.println(F("OK"));
        }
        else
        {
          MONITOR_SERIAL.println(F("failed"));
        }
      }
      else if(command.equals("restart"))
      {
        command = "";
        MONITOR_SERIAL.print(F("Restarting sensor: "));
        if(radar.requestRestart())
        {
          MONITOR_SERIAL.println(F("OK"));
        }
        else
        {
          MONITOR_SERIAL.println(F("failed"));
        }
      }
      else if(command.equals("readversion"))
      {
        command = "";
        MONITOR_SERIAL.print(F("Requesting firmware version: "));
        if(radar.requestFirmwareVersion())
        {
          MONITOR_SERIAL.print('v');
          MONITOR_SERIAL.print(radar.firmware_major_version);
          MONITOR_SERIAL.print('.');
          MONITOR_SERIAL.print(radar.firmware_minor_version);
          MONITOR_SERIAL.print('.');
          MONITOR_SERIAL.println(radar.firmware_bugfix_version,HEX);
        }
        else
        {
          MONITOR_SERIAL.println(F("Failed"));
        }
      }
      else if(command.equals("factoryreset"))
      {
        command = "";
        MONITOR_SERIAL.print(F("Factory resetting sensor: "));
        if(radar.requestFactoryReset())
        {
          MONITOR_SERIAL.println(F("OK, now restart sensor to take effect"));
        }
        else
        {
          MONITOR_SERIAL.println(F("failed"));
        }
      }
      else
      {
        MONITOR_SERIAL.print(F("Unknown command: "));
        MONITOR_SERIAL.println(command);
        command = "";
      }
    }
    else
    {
      command += typedCharacter;
    }
  }
}
