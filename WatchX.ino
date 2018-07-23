#include <avr/sleep.h>
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

//#define TAP 1
#define DOT

#ifdef TAP
#define SIGNAL_PATH_RESET  0x68
#define I2C_SLV0_ADDR      0x37
#define ACCEL_CONFIG       0x1C
#define MOT_THR            0x1F  // Motion detection threshold bits [7:0]
#define MOT_DUR            0x20  // Duration counter threshold for motion interrupt generation, 1 kHz rate, LSB = 1 ms
#define MOT_DETECT_CTRL    0x69
#define INT_ENABLE         0x38
#define WHO_AM_I_MPU6050   0x75 // Should return 0x68
#define INT_STATUS 0x3A
#define MPU6050_ADDRESS 0x69
#endif

#define USBDEVICE UDADDR & _BV(ADDEN)
unsigned char DEVICESTATE;

RTC_DS3231 rtc;
MPU6050 mpu;
StopWatch sw;
Adafruit_BMP280 bme;
MAG3110 mag = MAG3110();

#define OLED_DC A3
#define OLED_CS A5
#define OLED_RESET A4
Adafruit_SSD1306 display(OLED_DC, OLED_RESET, OLED_CS);

//char classes[10][3] = {"LEN", "MAT", "SOC", "NAT", "MUS", "FyQ", "DIB", "TUT", "EF", "VE"};
//char aulas[5][2] = {"E3", "E4", "E2", "D1", "D3"};

/*char Lunes[8][6] = {"ING-E3", "LEN-E3", "MUSICA", "1º REC" , "NAT-E2", "EF", "2 REC" , "FyQ-E3"};
  char Martes[8][6] = {"MAT-E3", "SOC-E3", "VE-E2", "1º REC", "FyQ-E3", "ING-E3", "2º REC", "LEN-E3"};
  char Miercoles[8][6] = {"MAT-E3", "SOC-E3", "LEN-E3", "1º REC", "DIB-D3", "ING-E3", "2º REC", "FyQ-E3"};
  char Jueves[8][6] = {"LEN-E3", "ING-E3", "NAT-E4", "1º REC", "SOC-E3", "MUSICA", "2º REC", "MAT-D1"};
  char Viernes[7][6] = {"EF", "NAT-E3", "MAT-E3", "1º REC", "ING-E3", "DIB-D3", "TUT-E2"};
*/
/*String getClass(short cls, short aula) {
  return String(classes[cls]) + " - " + String(aulas[aula]);
  }*/



char days[7][3] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
char months[12][3] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
long count = 0;
bool compass = false;
bool timer = false;
bool timerStarted = false;
bool updateBandT = true;
char Ch, Cm, Cs;
int total = Ch * 3600 + Cm * 60 + Cs;
DateTime now;

//VALORES CONFIGURABLES:-------------------------------------------------------
float axis = 6.5;
float axisRange = 2;
//float seaPressure = 1231.75;
char timeout = 5;
bool dim = false;
float tmp = -7;
#ifdef TAP
char MotionTreshold = 20;
//-----------------------------------------------------------------------------

void mpu_int() {
  detachInterrupt(digitalPinToInterrupt(7));
  //digitalWrite(13, HIGH);
  //Sleep();
}
#endif

void setup()
{
  pinMode(8, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
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
#ifndef TAP
  mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G);
#endif
  bme.begin();
  mag.initialize();
  mag.start();
  //Keyboard.begin();
  display.begin(SSD1306_SWITCHCAPVCC);
  count = millis();
  //digitalWrite(13, HIGH);
  //digitalWrite(6, HIGH);
#ifdef TAP
  mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_16G);
  delay(500);
  writeByte( MPU6050_ADDRESS, 0x6B, 0x00);
  writeByte( MPU6050_ADDRESS, SIGNAL_PATH_RESET, 0x07);//Reset all internal signal paths in the MPU-6050 by writing 0x07 to register 0x68;
  writeByte( MPU6050_ADDRESS, I2C_SLV0_ADDR, 0x20);//write register 0x37 to select how to use the interrupt pin. For an active high, push-pull signal that stays until register (decimal) 58 is read, write 0x20.
  writeByte( MPU6050_ADDRESS, ACCEL_CONFIG, 0x01);//Write register 28 (==0x1C) to set the Digital High Pass Filter, bits 3:0. For example set it to 0x01 for 5Hz. (These 3 bits are grey in the data sheet, but they are used! Leaving them 0 means the filter always outputs 0.)
  writeByte( MPU6050_ADDRESS, MOT_THR, MotionTreshold);  //Write the desired Motion threshold to register 0x1F (For example, write decimal 20).
  writeByte( MPU6050_ADDRESS, MOT_DUR, 1);  //Set motion detect duration to 1  ms; LSB is 1 ms @ 1 kHz rate
  writeByte( MPU6050_ADDRESS, MOT_DETECT_CTRL, 0x15); //to register 0x69, write the motion detection decrement and a few other settings (for example write 0x15 to set both free-fall and motion decrements to 1 and accelerometer start-up delay to 5ms total by adding 1ms. )
  writeByte( MPU6050_ADDRESS, INT_ENABLE, 0x40 ); //write register 0x38, bit 6 (0x40), to enable motion detection interrupt.
  writeByte( MPU6050_ADDRESS, 0x37, 160 );
  //digitalWrite(13, HIGH);
  //digitalWrite(6, HIGH);
#endif
}

void loop()
{
  //digitalWrite(13, digitalRead(7));
  //if (digitalRead(7) == LOW) tone(9, 1000, 100);
#ifndef TAP
  if (digitalRead(8) == LOW || digitalRead(10) == LOW || digitalRead(11) == LOW)
  { //IZQUIERDA, ABAJO, ARRIBA
    if (millis() - count > timeout * 1000)
      while (digitalRead(8) == LOW || digitalRead(10) == LOW || digitalRead(11) == LOW)
      {
      }
    count = millis();
    updateBandT = true;
  }

  if (axisRange > 0)
  {
    //mpu.readActivites();
    //mpu.readNormalizeAccel().ZAxis;
    float x = mpu.readNormalizeAccel().ZAxis;
    if (x >= axis - axisRange && x <= axis + axisRange)
    { // && z >= 3.4 - 2 && z <= 3.4 + 2) {
#ifdef DOT
      display.drawPixel(127, 63, WHITE);
#endif
      count = millis();
      //updateBandT = true;
    }
    else
    {
      if (!compass)
        delay(200);
    }
  }
#endif

  if (millis() - count > timeout * 1000 && timeout > 0 && !timer && !compass)
  {
    display.clearDisplay();
    display.display();
    //attachInterrupt(/*digitalPinToInterrupt(7)*/7, mpu_int, CHANGE);
#ifdef TAP
    Sleep();
#else
    delay(100);
    return;
#endif
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
      Ch = settings2("Hours", Ch, 1, 99, 0, 0);
      Cm = settings2("Minutes", Cm, 1, 59, 0, 0);
      Cs = settings2("Seconds", Cs, 1, 59, 0, 0);
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
      count = millis();
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
  DateTime newTime(settings2("Year", now.year(), 1, 2100, 1980, 0),
                   settings2("Month", now.month(), 1, 12, 1, 0),
                   settings2("Day", now.day(), 1, 31, 1, 0),
                   settings2("Hour", now.hour(), 1, 23, 0, 0),
                   settings2("Minute", now.minute(), 1, 59, 0, 0),
                   settings2("Second", now.second(), 1, 59, 0, 0));
  if (changed) rtc.adjust(newTime);
  axis = settings2("X Axis", axis, 0.1, 10, -10, 2);
  axisRange = settings2("X Sensivity", axisRange, 0.1, 10, 0, 1);
#ifdef TAP
  changed = false;
  MotionTreshold = settings2("Tap sensivity", MotionTreshold, 1, 30, 1, 0);
  if (changed) writeByte( MPU6050_ADDRESS, MOT_THR, MotionTreshold);
#endif
  settings2("Calibrated compass", mag.isCalibrated() ? 1 : 0, 1, 1, 0, 4);
  tmp = settings2("Temperature", tmp, 0.5, 20, -20, 1);
  timeout = settings2("Timeout", timeout, 1, 10, 0, 0);
  settings2("Dim", dim ? 1 : 0, 1, 1, 0, 3);
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
unsigned short del = 0;
float settings2(char text[], float val, float step, char mx, char mn, char mode)
{
  while (true)
  {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print(text);
    display.println(": ");
    if (mode == 1 && val >= 0)
      display.print('+');
    display.println(val, step == 1 ? 0 : 1);
    if (mode == 2)
    {
      display.print('X');
      display.print('=');
#ifndef TAP
      display.print(mpu.readNormalizeAccel().ZAxis);
#else
      display.print(mpu.readNormalizeAccel().ZAxis / 10);
#endif
    }
    if (mode == 3)
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

    if (mode == 4)
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

char last;
char batt;
float temp;

void printTime(char h, char m, char s)
{
  if (s == last) {
    delay(10);
    return;
  }
  display.setTextColor(WHITE);
  display.clearDisplay();
  display.setCursor(6, 22);
  display.setTextSize(3);
  display.print(h < 10 ? '0' : '\r');
  display.print(h, DEC);
  display.print(":");
  display.print(m < 10 ? '0' : '\r');
  display.print(m, DEC);
  display.setCursor(96, 29);
  display.setTextSize(2);
  display.print(s < 10 ? '0' : '\r');
  display.print(s, DEC);
  display.setTextSize(1);
  display.setCursor(0, 2);
  display.println();
  if ((sw.isRunning() && !timer && now.second() % 2 == 0) || timer) display.print(total > 0 ? 'C' : 'T');
  display.setTextSize(1);
  display.setCursor(8, 56);
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
  if (now.second() % 10 == 0 || updateBandT) {
    digitalWrite(4, HIGH);
    delay(50);
    float voltage = analogRead(A11);
    voltage = (voltage / 1024) * 3.35;
    voltage = voltage / 0.5;
    delay(50);
    digitalWrite(4, LOW);
    batt = map(voltage * 100, 340, 420, 0, 100);
    if (batt > 100)
      batt = 100;
    if (batt < 0)
      batt = 0;
  }
  display.setCursor(0, 2);
  display.print(batt, DEC);
  DEVICESTATE = (USBDEVICE) | ((((~PINB) & B11010000) + B00010000) >> 5);
  if (DEVICESTATE <= 5) {
    display.print('%');
  } else {
    if (batt < 10) drawThunder(7, 2); else if (batt < 100) drawThunder(13, 2); else drawThunder(19, 2);
  }
  char len = 4;
  if (now.second() % 5 == 0 || updateBandT) temp = bme.readTemperature() + tmp;
  updateBandT = false;
  if (temp >= 10 || temp <= -10)
    len += 1;
  if (temp < 0)
    len += 1;
  display.setCursor(128 - (len + 6) * 6, 2);
  if (compass)
  {
    if (mag.isCalibrated())
    {
      char heading = (short)mag.readHeading();
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
      display.print(heading, DEC);
    }
    else
    {
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
#ifdef TAP
  if (axisRange > 0)
  {
    //mpu.readActivites();
    //mpu.readNormalizeAccel().ZAxis;
    float x = mpu.readNormalizeAccel().ZAxis;
    x = x / 10;
    if (x >= axis - axisRange && x <= axis + axisRange)
    { // && z >= 3.4 - 2 && z <= 3.4 + 2) {
#ifdef DOT
      display.drawPixel(127, 63, WHITE);
#endif
      count = millis();
      //updateBandT = true;
    }
    else
    {
      if (!compass)
        delay(200);
    }
  }
#endif
  display.display();
  last = s;
}

#ifdef TAP
void Sleep()
{
  mpu.readActivites();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);   // sleep mode is set here
  sleep_enable();                        // enables the sleep bit in the mcucr register
  attachInterrupt(digitalPinToInterrupt(7), mpu_int, LOW); // use interrupt 0 (pin 2) and run function
  sleep_mode();     // here the device is actually put to sleep...!!


  // THE PROGRAM CONTINUES FROM HERE AFTER INTERRUPT IS CLOSED
  sleep_disable();         // first thing after waking from sleep: disable sleep...
  /*detachInterrupt(0);     We detach the interrupt to stop it from
                                  continuously firing while the interrupt pin
                                  is low.
  */
  //detachInterrupt(digitalPinToInterrupt(7));
#if (TAP == 1)
  count = millis();
  updateBandT = true;
#endif

#if (TAP == 2)
  long k = millis();
  delay(100);
  while (millis() - k < 500) {
    if (axisRange > 0)
    {
      float x = mpu.readNormalizeAccel().ZAxis / 10;
      if (x >= axis - axisRange && x <= axis + axisRange)
      {
        count = millis();
        updateBandT = true;
        //digitalWrite(13, HIGH);
        //delay(500);
        //digitalWrite(13, LOW);
        return;
      }
    }
  }
  Sleep();
#endif
}

void writeByte(uint8_t address, uint8_t subAddress, uint8_t data)
{
  Wire.begin();
  Wire.beginTransmission(address);  // Initialize the Tx buffer
  Wire.write(subAddress);           // Put slave register address in Tx buffer
  Wire.write(data);                 // Put data in Tx buffer
  Wire.endTransmission();           // Send the Tx buffer
  //  Serial.println("mnnj");

}
#endif

void drawThunder(char x, char y) {
  display.drawLine(x, y + 3, x + 5, y + 3, WHITE);
  display.drawLine(x + 1, y + 2, x + 2, y + 2, WHITE);
  display.drawLine(x + 3, y + 4, x + 4, y + 4, WHITE);
  display.drawPixel(x + 3, y, WHITE);
  display.drawPixel(x + 2, y + 6, WHITE);
  display.drawPixel(x + 2, y + 1, WHITE);
  display.drawPixel(x + 3, y + 5, WHITE);
}
