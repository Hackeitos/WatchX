#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <MPU6050.h>
#include <StopWatch.h>
#include "RTClib.h"
#include <SparkFun_MAG3110.h>
//#include <Keyboard.h>

RTC_DS3231 rtc;
MPU6050 mpu;
StopWatch sw;
Adafruit_BMP280 bme;
MAG3110 mag = MAG3110();

#define OLED_DC A3
#define OLED_CS A5
#define OLED_RESET A4
Adafruit_SSD1306 display(OLED_DC, OLED_RESET, OLED_CS);

char days[7][3] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
char months[12][3] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
long count = 0;
bool compass = false;
bool timer = false;
bool timerStarted = false;
short Ch, Cm, Cs;
int total = Ch * 3600 + Cm * 60 + Cs;
DateTime now;

//VALORES CONFIGURABLES:-------------------------------------------------------
float axis = 6.5;
float axisRange = 2;
//float seaPressure = 1231.75;
int timeout = 5;
bool dim = false;
float tmp = -7;
//-----------------------------------------------------------------------------

void setup()
{
  pinMode(8, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  rtc.begin();
  if (rtc.lostPower() || digitalRead(8) == LOW)
  {
    //Datetime(YYYY, MM, DD, HH, mm, ss)
    DateTime realTime(DateTime(F(__DATE__), F(__TIME__)) + TimeSpan(0, 0, 0, 8));
    rtc.adjust(realTime);
    digitalWrite(13, HIGH);
    digitalWrite(6, HIGH);
    delay(500);
    digitalWrite(13, LOW);
    digitalWrite(6, LOW);
  }
  mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G);
  //mpu.begin(0b11, 0b00);
  bme.begin();
  mag.initialize();
  mag.start();
  //Keyboard.begin();
  display.begin(SSD1306_SWITCHCAPVCC);
  count = millis();
}
void loop()
{
  if (axisRange > 0)
  {
    float x = mpu.readNormalizeAccel().XAxis;
    if (x >= axis - axisRange && x <= axis + axisRange)
    { // && z >= 3.4 - 2 && z <= 3.4 + 2) {
      count = millis();
    }
    else
    {
      if (!compass)
        delay(100);
    }
  }

  if (digitalRead(8) == LOW || digitalRead(10) == LOW || digitalRead(11) == LOW)
  { //IZQUIERDA, ABAJO, ARRIBA
    if (millis() - count > timeout * 1000)
      while (digitalRead(8) == LOW || digitalRead(10) == LOW || digitalRead(11) == LOW)
      {
      }
    count = millis();
  }

  if (millis() - count > timeout * 1000 && timeout > 0 && !timer && !compass)
  {
    display.clearDisplay();
    display.display();

    delay(100);
    return;
  }

  if (digitalRead(8) == LOW && digitalRead(11) == LOW)
  {
    while (digitalRead(8) == LOW)
    {
    }
    while (digitalRead(11) == LOW)
    {
    }
    if (timer)
    {
      //char text[], float val, float step, short mx, short mn
      Ch = settings2("Hours", Ch, 1, 99, 0);
      Cm = settings2("Minutes", Cm, 1, 59, 0);
      Cs = settings2("Seconds", Cs, 1, 59, 0);
    }
    else
    {
      settings();
    }
  }
  if (compass)
  {
    mag.exitStandby();
    mpu.setSleepEnabled(true);
  }
  else
  {
    mag.enterStandby();
    mpu.setSleepEnabled(false);
  }
  now = rtc.now();
  if (timer)
  {
    mpu.setSleepEnabled(true);
    if (digitalRead(11) == LOW)
    {
      while (digitalRead(11) == LOW)
      {
        if (digitalRead(8) == LOW)
          break;
      }
      if (digitalRead(8) == HIGH)
      {
        doTimer();
        sw.reset();
      }
    }
    if (digitalRead(10) == LOW)
    {
      if (sw.isRunning())
      {
        sw.stop();
      }
      else
      {
        if ((Ch * 3600 + Cm * 60 + Cs) - sw.elapsed() <= 0 && sw.elapsed() > 0)
          sw.reset();
        else
          sw.start();
      }
      while (digitalRead(10) == LOW)
        doTimer();
    }
    doTimer();
  }
  else
  {
    mpu.setSleepEnabled(false);
    if (digitalRead(10) == LOW)
    {
      while (digitalRead(10) == LOW)
      {
      }
      compass = !compass;
    }
    printTime(now.hour(), now.minute(), now.second());
  }
  if (digitalRead(8) == LOW)
  {
    while (digitalRead(8) == LOW)
    {
      if (digitalRead(11) == LOW)
        break;
    }
    if (digitalRead(11) == HIGH)
    {
      display.clearDisplay();
      //display.display();
      timer = !timer;
    }
  }
}

void doTimer()
{
  total = Ch * 3600 + Cm * 60 + Cs;
  if (total == 0)
  {
    if (sw.elapsed() >= 3600000)
    {
      printTime(((sw.elapsed() / 1000) % 86400) / 3600, ((sw.elapsed() / 1000) / 60) % 60, (sw.elapsed() / 1000) % 60);
    }
    else
    {
      printTime(((sw.elapsed() / 1000) / 60) % 60, (sw.elapsed() / 1000) % 60, (sw.elapsed() % 1000) / 10);
    }
  }
  else
  {
    int diff = total - sw.elapsed() / 1000;
    printTime((diff % 86400) / 3600, (diff / 60) % 60, diff % 60);
    if (diff <= 0)
    {
      sw.stop();
      while (digitalRead(8) == HIGH || digitalRead(10) == HIGH || digitalRead(11) == HIGH)
      {
        digitalWrite(13, rtc.now().second() % 2 == 0 ? HIGH : LOW);
        digitalWrite(6, rtc.now().second() % 2 == 0 ? HIGH : LOW);
        if (timer)
          printTime(0, 0, 0);
        else
          printTime(now.hour(), now.minute(), now.second());
        if (digitalRead(8) == LOW || digitalRead(10) == LOW || digitalRead(11) == LOW)
        {
          sw.reset();
          while (digitalRead(8) == LOW || digitalRead(10) == LOW || digitalRead(11) == LOW)
          {
          }
          break;
        }
      }
    }
  }
}

bool changed;

void settings()
{
  float temp;
  //Datetime(YYYY, MM, DD, HH, mm, ss)
  changed = false;
  DateTime newTime(settings2("Year", now.year(), 1, 2100, 1980),
                   settings2("Month", now.month(), 1, 12, 1),
                   settings2("Day", now.day(), 1, 1, 7),
                   settings2("Hour", now.hour(), 1, 23, 0),
                   settings2("Minute", now.minute(), 1, 59, 0),
                   settings2("Second", now.second(), 1, 59, 0));
  if (changed)
    rtc.adjust(newTime);
  changed = false;
  temp = settings2("X Axis", axis, 0.1, 10, -10);
  if (changed)
    axis = temp;
  changed = false;
  temp = settings2("Sensibility", axisRange, 0.1, 10, 0);
  if (changed)
    axisRange = temp;
  temp = settings2("Calibrated compass", mag.isCalibrated() ? 1 : 0, 1, 1, 0);
  changed = false;
  temp = settings2("Temperature", tmp, 0.5, 20, -20);
  if (changed)
    tmp = temp;
  changed = false;
  temp = settings2("Timeout", timeout, 1, 10, 0);
  if (changed)
    timeout = temp;
  settings2("Dim", dim ? 1 : 0, 1, 1, 0);
  count = millis();
}
//IZQUIERDA 8, ABAJO 10, ARRIBA 11
/*VALORES CONFIGURABLES:-------------------------------------------------------
  float axis = 6.5;
  float axisRange = 2;
  float seaPressure = 1231.75; 28254
  int timeout = 5;
  bool dim = false;
  -----------------------------------------------------------------------------*/
bool goFast = false;
short del = 0;
float settings2(char text[], float val, float step, short mx, short mn)
{
  while (true)
  {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print(text);
    display.println(": ");
    if ((text == "Sensibility" || text == "Temperature") && val >= 0)
      display.print('+');
    display.println(val, step == 1 ? 0 : 1);
    if (text == "X Axis")
    {
      display.print('X');
      display.print('=');
      display.print(mpu.readNormalizeAccel().XAxis);
    }
    if (text == "Dim")
    {
      if (val == 1)
      {
        display.dim(true);
        dim = true;
      }
      else
      {
        display.dim(false);
        dim = false;
      }
    }
    display.display();

    if (digitalRead(11) == HIGH && digitalRead(10) == HIGH)
      goFast = false;

    if (text == "Calibrated compass")
    {
      if (digitalRead(11) == LOW || digitalRead(10) == LOW)
      {
        mag.enterCalMode();
        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("Calibrating...");
        display.display();
        while (mag.isCalibrating())
        {
          mag.calibrate();
          if (digitalRead(8) == LOW)
            break;
        }
        //while (digitalRead(11) == HIGH && digitalRead(10) == HIGH && digitalRead(8) == HIGH) {}
        //while (digitalRead(11) == LOW || digitalRead(10) == LOW || digitalRead(8) == LOW) {}
      }
      val = mag.isCalibrated() ? 1 : 0;
    }
    else
    {
      if (digitalRead(11) == LOW)
      {
        changed = true;
        val = val + step;
        if (val > mx)
          val = mn;
        //delay(200);
        if (goFast)
        {
          delay(10);
        }
        else
        {
          del = 0;
          while (digitalRead(11) == LOW)
          {
            delay(1);
            del++;
            if (del == 700)
            {
              goFast = true;
              break;
            }
          }
        }
      }
      if (digitalRead(10) == LOW)
      {
        changed = true;
        val = val - step;
        if (val < mn)
          val = mx;
        //delay(200);
        if (goFast)
        {
          delay(10);
        }
        else
        {
          del = 0;
          while (digitalRead(10) == LOW)
          {
            delay(1);
            del++;
            if (del == 700)
            {
              goFast = true;
              break;
            }
          }
        }
      }
    }
    if (digitalRead(8) == LOW)
    {
      while (digitalRead(8) == LOW)
      {
      }
      return val;
    }
  }
}

void printTime(short h, short m, short s)
{
  //readImage(0, 0, WHITE, BLACK);
  display.setCursor(6, 22);
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.clearDisplay();
  display.print(h < 10 ? '0' : '\r');
  display.print(h);
  display.print(":");
  display.print(m < 10 ? '0' : '\r');
  display.print(m);
  display.setCursor(96, 29);
  display.setTextSize(2);
  display.print(s < 10 ? '0' : '\r');
  display.print(s);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println();
  if ((sw.isRunning() && !timer && now.second() % 2 == 0) || timer)
    display.print(total > 0 ? 'C' : 'T');
  display.setTextSize(1);
  display.setCursor(13, 56);
  display.print(months[now.month() - 1][0]);
  display.print(months[now.month() - 1][1]);
  display.print(months[now.month() - 1][2]);
  display.print(" ");
  display.print(now.day() < 10 ? '0' : '\0');
  display.print(now.day());
  display.print(", ");
  display.print(days[now.dayOfTheWeek()][0]);
  display.print(days[now.dayOfTheWeek()][1]);
  display.print(days[now.dayOfTheWeek()][2]);
  display.print(", ");
  display.print(now.year());
  //display.display();
  //}
  digitalWrite(4, HIGH);
  delay(50);
  float voltage = analogRead(A11);
  voltage = (voltage / 1024) * 3.35;
  voltage = voltage / 0.5;
  delay(50);
  digitalWrite(4, LOW);
  display.setCursor(0, 0);
  short batt = map(voltage * 100, 340, 420, 0, 100);
  if (batt > 100)
    batt = 100;
  if (batt < 0)
    batt = 0;
  display.print(batt);
  display.println('%');
  short len = 4;
  float temp = bme.readTemperature() + tmp;
  if (temp >= 10 || temp <= -10)
    len += 1;
  if (temp < 0)
    len += 1;
  display.setCursor(128 - (len + 6) * 6, 0);
  if (compass)
  {
    if (mag.isCalibrated())
    {
      int heading = (int)mag.readHeading();
      heading = map(heading, -180, 180, 0, 360);
      //if (heading > 180) heading = -180 + heading % 180;
      //if (heading < 180) heading = 180 + heading % 180;
      //heading = map(heading, -90, 270, 0, 360);
      char facing = ' ';
      if (heading >= 360 - 45 || heading < 0 + 45)
        facing = 'N';
      if (heading >= 180 - 45 && heading < 180 + 45)
        facing = 'S';
      if (heading >= 90 - 45 && heading < 90 + 45)
        facing = 'E';
      if (heading >= 270 - 45 && heading < 270 + 45)
        facing = 'W';
      display.print(facing);
      display.print(' ');
      display.print(heading < 10 ? '0' : '\r');
      display.print(heading < 100 ? '0' : '\r');
      display.print(heading);
    }
    else
    { //28028
      display.print("  CAL");
    }
    display.print(' ');
  }
  else
  {
    display.print("      ");
  }
  display.print(temp, 1);
  display.print('C');
  display.display();
}
