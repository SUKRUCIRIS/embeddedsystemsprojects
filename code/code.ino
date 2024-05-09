#include "dht11.h"
#include <LiquidCrystal.h>
#define DHT11PIN 2
LiquidCrystal lcd(12, 11, 5, 4, 3, 7);

dht11 DHT11;
int temp = 0;
int nem = 0;
const int fan_control_pin = 10;
int count = 0;

void setup()
{
  Serial.begin(9600);
  lcd.begin(16, 2);
  
  pinMode(fan_control_pin, OUTPUT);
  analogWrite(fan_control_pin, 0);
  attachInterrupt(digitalPinToInterrupt(8), counter, RISING);
}

void loop()
{

  int chk = DHT11.read(DHT11PIN);
  temp = DHT11.temperature;
  nem = DHT11.humidity;

  count = 0;
  
  lcd.setCursor(0, 1);
  lcd.print("fan hizi %:");

  if(temp < 25)
  {
    analogWrite(fan_control_pin, 63);
      lcd.setCursor(12, 1);
      lcd.print(25);
  }
  if(temp >= 25 && temp <= 30)
  {
    analogWrite(fan_control_pin, 127);
      lcd.setCursor(12, 1);
      lcd.print(50);
  }
  if(temp > 30)
  {
    analogWrite(fan_control_pin, 255);
      lcd.setCursor(12, 1);
      lcd.print(100);
  }

  lcd.setCursor(0, 0);
  lcd.print("temp:");
  lcd.setCursor(5, 0);
  lcd.print(temp);

  lcd.setCursor(8, 0);
  lcd.print("nem %:");
  lcd.setCursor(14, 0);
  lcd.print(nem);

  delay(1000);
  
}

void counter(){
  count++;
}
