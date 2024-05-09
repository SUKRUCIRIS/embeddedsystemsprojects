//you need to install WiFiEsp and PubSubClient from library manager
#include <WiFiEsp.h>
#include <WiFiEspClient.h>
#include <WiFiEspUdp.h>
#include <PubSubClient.h>
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include "dht11.h"

#define DHT11PIN 2
LiquidCrystal lcd(12, 11, 5, 4, 3, 7);

dht11 DHT11;
int temp = 0;
int nem = 0;
const int fan_control_pin = 10;
int count = 0;

IPAddress server(192, 168, 1, 100); //mqtt server ip
int port=8080; //mqtt server port
char ssid[] = "wifiname";
char pass[] = "wifipassword"; 
int status = WL_IDLE_STATUS;   // the Wifi radio's status

//Arayuzden cihaza gonderilecek veriler, her topicde string olarak sayilar gelicek
char devicetopic[]="printercooler/device";
char templimit1topic[]="printercooler/device/temp1";
char templimit2topic[]="printercooler/device/temp2";
char fanspeed1topic[]="printercooler/device/speed1";
char fanspeed2topic[]="printercooler/device/speed2";
char fanspeed3topic[]="printercooler/device/speed3";

//Arayuze gonderilecek veriler, bunlarla grafik cizilcek, her topicde string olarak sayilar gidicek
char guitopic[]="printercooler/gui";
char tempdatatopic[]="printercooler/gui/temp";
char humdatatopic[]="printercooler/gui/hum";
char speeddatatopic[]="printercooler/gui/speed";

// Initialize the Ethernet client object
WiFiEspClient espClient;

PubSubClient client(espClient);

SoftwareSerial soft(9,13); // RX, TX

int templimit1=25,templimit2=30;
int fanspeed1=63,fanspeed2=127,fanspeed3=255;
int currspeed=0;

void setup()
{
  Serial.begin(9600);
  lcd.begin(16, 2);
  
  pinMode(fan_control_pin, OUTPUT);
  analogWrite(fan_control_pin, 0);
  attachInterrupt(digitalPinToInterrupt(8), counter, RISING);

  soft.begin(9600);
  // initialize ESP module
  WiFi.init(&soft);

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  // you're connected now, so print out the data
  Serial.println("You're connected to the network");

  //connect to MQTT server
  client.setServer(server, port);
  client.setCallback(callback);
}

void loop()
{

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  DHT11.read(DHT11PIN);
  temp = DHT11.temperature;
  nem = DHT11.humidity;

  count = 0;
  
  lcd.setCursor(0, 1);
  lcd.print("fan hizi %:");

  if(temp < templimit1)
  {
    analogWrite(fan_control_pin, fanspeed1);
    lcd.setCursor(12, 1);
    lcd.print(fanspeed1);
    currspeed=fanspeed1;
  }
  else if(temp >= templimit1 && temp <= templimit2)
  {
    analogWrite(fan_control_pin, fanspeed2);
    lcd.setCursor(12, 1);
    lcd.print(fanspeed2);
    currspeed=fanspeed2;
  }
  else if(temp > templimit2)
  {
    analogWrite(fan_control_pin, fanspeed3);
    lcd.setCursor(12, 1);
    lcd.print(fanspeed3);
    currspeed=fanspeed3;
  }

  lcd.setCursor(0, 0);
  lcd.print("temp:");
  lcd.setCursor(5, 0);
  lcd.print(temp);

  lcd.setCursor(8, 0);
  lcd.print("nem %:");
  lcd.setCursor(14, 0);
  lcd.print(nem);

  char payload[20];

  itoa(temp, payload, 20); 
  client.publish(tempdatatopic,payload);

  itoa(nem, payload, 20); 
  client.publish(humdatatopic,payload);

  itoa(currspeed, payload, 20); 
  client.publish(speeddatatopic,payload);

  delay(1000);
  
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';
  if (strcmp(topic, templimit1topic) == 0){
    templimit1=atoi(message);
  }
  else if(strcmp(topic, templimit2topic) == 0){
    templimit2=atoi(message);
  }
  else if(strcmp(topic, fanspeed1topic) == 0){
    fanspeed1=atoi(message);
  }
  else if(strcmp(topic, fanspeed2topic) == 0){
    fanspeed2=atoi(message);
  }
  else if(strcmp(topic, fanspeed3topic) == 0){
    fanspeed3=atoi(message);
  }
  else{
    Serial.println("Unknown topic!!!");
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("Device")) {
      Serial.println("Device connected...");
      client.subscribe(devicetopic);
    } else {
      Serial.print("Device connection failed, state=");
      Serial.print(client.state());
      Serial.println(" trying again in 1 seconds");
      delay(1000);
    }
  }
}

void counter(){
  count++;
}
