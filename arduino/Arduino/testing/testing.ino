// Temperature sensor
#include <dht.h>

// Only used for sprintf
#include <stdio.h>

// Time
#include <Wire.h>
#include <Time.h>
#include <DS1307RTC.h>

// LCD
#include <LiquidCrystal_I2C.h>

// SD Card

/*
  SD card read/write
 
 This example shows how to read and write data to and from an SD card file 	
 The circuit:
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4
 
 created   Nov 2010
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe
 
 This example code is in the public domain.
 	 
 */
 
#include <Fat16.h>
#include <Fat16util.h> // use functions to print strings from flash memory


// CO2 Sensor

 
/************************Hardware Related Macros************************************/
#define         MG_PIN                       1     //define which analog input channel you are going to use
#define         DC_GAIN                      8.5   //define the DC gain of amplifier
 
/***********************Software Related Macros************************************/
#define         READ_SAMPLE_INTERVAL         (50)    //define how many samples you are going to take in normal operation
#define         READ_SAMPLE_TIMES            (5)     //define the time interval(in milisecond) between each samples in 
                                                     //normal operation
 
/**********************Application Related Macros**********************************/
//These two values differ from sensor to sensor. user should derermine this value.
#define         ZERO_POINT_VOLTAGE           (0.400) //define the output of the sensor in volts when the concentration of CO2 is 400PPM
#define         REACTION_VOLTGAE             (0.0517) //define the voltage drop of the sensor when move the sensor from air into 1000ppm CO2
 
/*****************************Globals***********************************************/
float           CO2Curve[3]  =  {2.602,ZERO_POINT_VOLTAGE,(REACTION_VOLTGAE/(2.602-3))};   
                                                     //two points are taken from the curve. 
                                                     //with these two points, a line is formed which is
                                                     //"approximately equivalent" to the original curve.
                                                     //data format:{ x, y, slope}; point1: (lg400, 0.324), point2: (lg4000, 0.280) 
                                                     //slope = ( reaction voltage ) / (log400 –log1000) 

int percentage;
float volts;


// Temperature

// Setup a DHT22 instance
// Data wire is plugged into port 7 on the Arduino
// Connect a 4.7K resistor between VCC and the data pin (strong pullup)

dht DHT;

#define DHT22_PIN 7

struct
{
    uint32_t total;
    uint32_t ok;
    uint32_t crc_error;
    uint32_t time_out;
    uint32_t connect;
    uint32_t ack_l;
    uint32_t ack_h;
    uint32_t unknown;
} stat = { 0,0,0,0,0,0,0,0};

// SD Card
SdCard card;
Fat16 file;

// Time
tmElements_t tm;


// Free Memory request

extern unsigned long __bss_end;
extern void *__brkval;

// Read input data
char inputChar;

char iteration = 0;

// LCD
LiquidCrystal_I2C lcd(0x38,16,2);  // set the LCD address to 0x20 for a 16 chars and 2 line display

void setup(void)
{  
  // start serial port
  Serial.begin(9600);

  while (!Serial) ; // wait for serial

  delay(200);
  printFreeMemory("0");

  // SD Card

  printFreeMemory("1");

  // initialize the SD card
  if (!card.begin(SS)) Serial.println("error [SD card]: card.begin");
  // initialize a FAT16 volume
  if (!Fat16::init(&card)) Serial.println("error [SD card]: Fat16::init");

  printFreeMemory("2");
  
  // LCD
  lcd.init();

}

void loop(void)
{ 

  // The sensor can only be read from every 1-2s, and requires a minimum
  // 2s warm-up after power-on.
  delay(2000);
  iteration++;

  // Temperature sensor  

  printFreeMemory("3");

  uint32_t start = micros();
  int chk = DHT.read22(DHT22_PIN);
  uint32_t stop = micros();

  printFreeMemory("4");

   stat.total++;
    switch (chk)
    {
    case DHTLIB_OK:
        stat.ok++;
        break;
    case DHTLIB_ERROR_CHECKSUM:
        stat.crc_error++;
        Serial.print("Checksum error,\t");
        break;
    case DHTLIB_ERROR_TIMEOUT:
        stat.time_out++;
        Serial.print("Time out error,\t");
        break;
    case DHTLIB_ERROR_CONNECT:
        stat.connect++;
        Serial.print("Connect error,\t");
        break;
    case DHTLIB_ERROR_ACK_L:
        stat.ack_l++;
        Serial.print("Ack Low error,\t");
        break;
    case DHTLIB_ERROR_ACK_H:
        stat.ack_h++;
        Serial.print("Ack High error,\t");
        break;
    default:
        stat.unknown++;
        Serial.print("Unknown error,\t");
        break;
    }
  
  Serial.print(DHT.temperature); 
  Serial.print( "," );
  Serial.print(DHT.humidity); 
  Serial.print( "," );  printFreeMemory("5");

  // Time

  if (!RTC.read(tm)) {
    if (RTC.chipPresent()) {
      Serial.println("error [RTC]: stopped");
    } else {
      Serial.println("error [RTC]: read error");
    }
    delay(9000);
  }

  printFreeMemory("7");

  
  // CO2 Sensor
     
  volts = MGRead();
  
  printFreeMemory("7.1");

  Serial.print(volts); 
  Serial.print( "," );
     
  percentage = MGGetPercentage();

  printFreeMemory("7.2");

  if (percentage == -1) {
      Serial.println( "<400" );
  } else {
      Serial.println(percentage);
  }

  printFreeMemory("7.3");
     
  // SD Card

  if (iteration % 60 == 0) {
    // create a new file with default timestamps
    // clear write error
    file.writeError = false;
    if (!file.open("log.csv", O_CREAT | O_APPEND | O_WRITE)) Serial.println("error [SD card]: open");
  
  
    printFreeMemory("8");
  
    print2digits(tm.Hour);
    file.print(':');
    print2digits(tm.Minute);
    file.print(':');
    print2digits(tm.Second);
    file.print(" ");
    file.print(tm.Day);
    file.print('.');
    file.print(tm.Month);
    file.print('.');
    file.print(tmYearToCalendar(tm.Year));
    file.print(",");
    file.print(DHT.temperature);
    file.print(",");
    file.print(DHT.humidity);
    file.print(",");
    file.print(volts);
    file.print(",");
    file.println(percentage);
  
    if (file.writeError) Serial.println("error [SD card]: write");
    if (!file.close()) Serial.println("error [SD card]: close");
  }

  printFreeMemory("10");

  // LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("t:");
  lcd.setCursor(2, 0);
  lcd.print(DHT.temperature);
  lcd.setCursor(8, 0);
  lcd.print("h:");
  lcd.setCursor(10, 0);
  lcd.print(DHT.humidity);
  lcd.setCursor(0, 1);
  lcd.print("CO2:");
  lcd.setCursor(4, 1);
  lcd.print(percentage);
  lcd.setCursor(9, 1);
  lcd.print("V:");
  lcd.setCursor(11, 1);
  lcd.print(volts);


  // Read input commands
  
  if (Serial.available()){
    inputChar = Serial.read();
    if (inputChar == 'l') {

    
      // open a file
      if (!file.open("log.csv", O_READ)) {
        Serial.println("error [SD card]: open");
      }
  
      // copy file to serial port
      Serial.println("log begin");
      int16_t c;
      while ((c = file.read()) > 0) {
        Serial.write((char)c);
      }
      Serial.println("log end");
      if (!file.close()) Serial.println("error [SD card]: close");
    }
  }
}

// Time
void print2digits(int number) {
  if (number >= 0 && number < 10) {
    file.write('0');
  }
  file.print(number);
}

// CO2 Sensor

/*****************************  MGRead *********************************************
Input:   mg_pin - analog channel
Output:  output of SEN-000007
Remarks: This function reads the output of SEN-000007
************************************************************************************/
float MGRead()
{
    int i;
    float v=0;
 
    for (i=0;i<READ_SAMPLE_TIMES;i++) {
        v += analogRead(MG_PIN);
        delay(READ_SAMPLE_INTERVAL);
    }
    v = (v/READ_SAMPLE_TIMES) *5/1024 ;
    return v;  
}
 
/*****************************  MQGetPercentage **********************************
Input:   volts   - SEN-000007 output measured in volts
         pcurve  - pointer to the curve of the target gas
Output:  ppm of the target gas
Remarks: By using the slope and a point of the line. The x(logarithmic value of ppm) 
         of the line could be derived if y(MG-811 output) is provided. As it is a 
         logarithmic coordinate, power of 10 is used to convert the result to non-logarithmic 
         value.
************************************************************************************/
int  MGGetPercentage()
{
   if ((volts/DC_GAIN )>=ZERO_POINT_VOLTAGE) {
      return -1;
   } else { 
      return pow(10, ((volts/DC_GAIN)-CO2Curve[1])/CO2Curve[2]+CO2Curve[0]);
   }
}

// Free memory request
int memoryFree()
{
  //  long myValue;
  int freeValue;
  freeValue = 0;

  if ((unsigned long)__brkval == 0)
  { 
    freeValue = ((unsigned long)&freeValue) - ((unsigned long)&__bss_end);
    //       Serial.println("here and there");
  }
  else
  { 
    freeValue = ((unsigned long)&freeValue) - ((unsigned long)__brkval);
    //      Serial.println("here  2");
  }

  return freeValue;

}//end memoryFree()

void printFreeMemory(String message) {
//  Serial.println("  ");  
//  Serial.print("Free Memory:  ");
//  Serial.print(message); 
//  Serial.print(" ");
//  Serial.print(memoryFree()); 
//  Serial.println("  ");  
}
