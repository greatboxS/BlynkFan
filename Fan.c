/*************************************************************
  Download latest Blynk library here:
    https://github.com/blynkkk/blynk-library/releases/latest

  Blynk is a platform with iOS and Android apps to control
  Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build graphic interfaces for all your
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: http://www.blynk.cc
    Sketch generator:           http://examples.blynk.cc
    Blynk community:            http://community.blynk.cc
    Follow us:                  http://www.fb.com/blynkapp
                                http://twitter.com/blynk_app

  Blynk library is licensed under MIT license
  This example code is in public domain.

 *************************************************************

  You can send/receive any data using WidgetTerminal object.

  App project setup:
    Terminal widget attached to Virtual Pin V1
 *************************************************************/

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <TimeLib.h>
#include <WidgetRTC.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
//char auth[] = "5WKqO0pR3K-BmVbOdZG6fez_TZK2lxqu";
char auth[] = "pb9oPmS95HcyrJ93ZYT6PugvOF07e6Ed";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "THU HANG";
char pass[] = "1234567891";

String Exception = "Status: OK";
bool FanState = false;
#define FAN_PIN GPIO_NUM_2

bool HandControl = false;
int CounterTick = 0;
int SettingCounterMax = 30 * 60;

typedef struct SettingTimer
{
  uint8_t hour;
  uint8_t sec;
  uint8_t min;
};

typedef struct SettingTimeGroup
{
  SettingTimer StartTime, StopTime;
};

SettingTimeGroup Setting[5];
BlynkTimer timer;
WidgetRTC rtc;

BLYNK_CONNECTED()
{
  // Synchronize time on connection
  rtc.begin();
  // Request Blynk server to re-send latest values for all pins
  Blynk.syncAll();
}

BLYNK_WRITE(V1)
{
  // You'll get uptime value here as result of syncAll()
  int uptime = param.asInt();
  Serial.println("Write to V1");
  Serial.println(uptime);
  if (uptime == 0)
  {
    FanState = false;
    HandControl = false;
    Exception = "Hand control Stop: " + String(hour()) + ":" + String(minute()) + ":" + String(second());
    Blynk.virtualWrite(V0, Exception);
  }
  else
  {
    FanState = true;
    HandControl = true;
    CounterTick = 0;
    Exception = "Hand control Start: " + String(hour()) + ":" + String(minute()) + ":" + String(second());
    Blynk.virtualWrite(V0, Exception);
  }
}

BLYNK_WRITE(V2)
{
  TimeInputParam t(param);
  GetSettingTime(t, 0);
}

BLYNK_WRITE(V3)
{
  TimeInputParam t(param);
  GetSettingTime(t, 1);
}

BLYNK_WRITE(V4)
{
  TimeInputParam t(param);
  GetSettingTime(t, 2);
}

BLYNK_WRITE(V5)
{
  TimeInputParam t(param);
  GetSettingTime(t, 3);
}

BLYNK_WRITE(V6)
{
  TimeInputParam t(param);
  GetSettingTime(t, 4);
}

BLYNK_WRITE(V7)
{
  int counterSet = param.asInt();
  if (counterSet > 0 && counterSet < 1000)
  {
    SettingCounterMax = counterSet * 60;
    Exception = "Set Counter: " + String(counterSet) + "(min) : " + "OK";
    Blynk.virtualWrite(V0, Exception);
  }
  else
  {
    Exception = "Set Counter: Error";
    Blynk.virtualWrite(V0, Exception);
  }
}

void GetSettingTime(TimeInputParam &t, int index)
{
  Serial.print("Setting :");
  Serial.println(index);

  if (t.hasStartTime())
  {
    Serial.println(String("Start: ") + t.getStartHour() + ":" + t.getStartMinute() + ":" + t.getStartSecond());
    Setting[index].StartTime.hour = t.getStartHour();
    Setting[index].StartTime.min = t.getStartMinute();
    Setting[index].StartTime.sec = t.getStartSecond();
  }

  if (t.hasStopTime())
  {
    Serial.println(String("Stop: ") + t.getStopHour() + ":" + t.getStopMinute() + ":" + t.getStopSecond());
    Setting[index].StopTime.hour = t.getStopHour();
    Setting[index].StopTime.min = t.getStopMinute();
    Setting[index].StopTime.sec = t.getStopSecond();
  }
}

void TimerCompare()
{
  uint8_t currentH = hour();
  uint8_t currentM = minute();
  uint8_t currentS = second();

  if (HandControl)
    return;

  for (int i = 0; i < 5; i++)
  {
    if ((Setting[i].StartTime.hour == currentH) &&
        (Setting[i].StartTime.min == currentM) &&
        (Setting[i].StartTime.sec <= (currentS + 5)) &&
        (Setting[i].StartTime.sec >= (currentS - 5)))
    {
      if (!FanState)
      {
        FanState = true;
        Exception = "Start at:" + String(currentH) + ":" + String(currentM) + ":" + String(currentS) + " - " +
                    String(day()) + " " + String(month()) + " " + String(year());
        Blynk.virtualWrite(V0, Exception);
        Blynk.virtualWrite(V1, 1);
      }
    }

    if ((Setting[i].StopTime.hour == currentH) &&
        (Setting[i].StopTime.min == currentM) &&
        (Setting[i].StopTime.sec <= (currentS + 5)) &&
        (Setting[i].StopTime.sec >= (currentS - 5)))
    {
      if (FanState)
      {
        FanState = false;
        Exception = "Stop at:" + String(currentH) + ":" + String(currentM) + ":" + String(currentS) + " - " +
                    String(day()) + " " + String(month()) + " " + String(year());
        Blynk.virtualWrite(V0, Exception);
        Blynk.virtualWrite(V1, 0);
      }
    }
  }
}

// Digital clock display of the time
void ClockTick()
{
  CounterTick++;
  if ((CounterTick >= SettingCounterMax) && HandControl)
  {
    FanState = false;
    HandControl = false;
    Exception = "Hand control Stop: " + String(hour()) + ":" + String(minute()) + ":" + String(second());
    Blynk.virtualWrite(V0, Exception);
    Blynk.virtualWrite(V1, 0);
  }
  // You can call hour(), minute(), ... at any time
  // Please see Time library examples for details
  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  String currentDate = String(day()) + " " + month() + " " + year();
  Serial.print("Current time: ");
  Serial.print(currentTime);
  Serial.print(" ");
  Serial.print(currentDate);
  Serial.println();

  TimerCompare();
}

void setup()
{
  // Debug console
  Serial.begin(9600);

  Blynk.begin(auth, ssid, pass);
  setSyncInterval(10 * 60); // Sync interval in seconds (10 minutes)
  // Display digital clock every 10 seconds
  timer.setInterval(1000L, ClockTick);

  Blynk.virtualWrite(V1, 0);

  Blynk.virtualWrite(V0, Exception);

  pinMode(FAN_PIN, OUTPUT);
}

void loop()
{
  Blynk.run();
  timer.run();

  if (FanState)
    digitalWrite(FAN_PIN, LOW);
  else
    digitalWrite(FAN_PIN, HIGH);
}