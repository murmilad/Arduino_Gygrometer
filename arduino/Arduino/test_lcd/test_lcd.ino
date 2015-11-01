#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x38,16,2);  // set the LCD address to 0x20 for a 16 chars and 2 line display

void setup()
{
  lcd.init();
  lcd.print("Hello, world!");
}

void loop()
{
  lcd.setCursor(0, 1);
  lcd.print(millis()/1000);
}
