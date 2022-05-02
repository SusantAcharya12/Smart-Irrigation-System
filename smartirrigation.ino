#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

LiquidCrystal_I2C lcd(0x3F, 20, 4); // set the LCD address to 0x3F for a 16 chars and 2 line display
SoftwareSerial mySerial(9, 10);
int SMSSent;

const int trigPin = 7; //D7
const int echoPin = 6; //D6

// defines variables
long duration;
float distance;
float pdistance;
float pdistance2;
float pdistance3;
int tankDepth = 15; //depth of the tank to be monitored
float alertDepth = 10;

const int pumpPin = 13;

int rainPin = A0;
// you can adjust the threshold value
int thresholdValue = 500;

void setup()
{

  pinMode(12, OUTPUT);
  pinMode(pumpPin, OUTPUT);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);  // Sets the echoPin as an Input
  Serial.begin(9600);       // Starts the serial communication
  mySerial.begin(9600);     // Setting the baud rate of GSM Module
  pinMode(rainPin, INPUT);
  lcd.init();
  lcd.backlight();
  lcd.print("SMART IRRIGATION");
  delay(3000);
}

void SendMessage()
{
  mySerial.println("AT+CMGF=1");                   //Sets the GSM Module in Text Mode
  delay(1000);                                     // Delay of 1000 milli seconds or 1 second
  mySerial.println("AT+CMGS=\"9779815337491\"\r"); // Replace x with mobile number
  delay(1000);
  mySerial.println("Smart Irrigation: Time to refill your tank!"); // The SMS text you want to send
  delay(100);
  mySerial.println((char)26); // ASCII code of CTRL+Z
  delay(1000);
}
void waterLevel()
{

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println("cm");

  if (distance > alertDepth)
  {
    Serial.println("ALERT! Please Refill your tank!");
    Serial.println("Buzzer on");
    lcd.setCursor(0,1);
    lcd.print("WL:Low");
    digitalWrite(12, HIGH);

    if (SMSSent == 0)
    {
      SendMessage();
      SMSSent = 1;
    }
  }
  else if (distance > 100)
  {
    digitalWrite(12, 0);
  }

  else
  {
    Serial.println("Tank full");
    lcd.setCursor(0,1);
    lcd.print("WL:High");
    digitalWrite(12, 0);
  }

  Serial.print("Water Level:");
  Serial.print(distance);
  delay(50);
}
void soilMoisture()
{

  // read the input on analog pin 0:
  int sensorValue = analogRead(rainPin);
  Serial.println("A0:");
  Serial.print(sensorValue);
  float s = 100- sensorValue/7;
  lcd.setCursor(11,1);
  lcd.print(s);
  lcd.setCursor(15,1);
  lcd.print('%');

  if (sensorValue < thresholdValue)
  {
    Serial.println(" - Doesn't need watering");
    lcd.init(); // initialize the lcd
    lcd.setCursor(0, 0);
    lcd.print("No Need to Water");
    digitalWrite(pumpPin, LOW);
    Serial.println("Pump is Off");
  }
  else
  {
    Serial.println(" - Time to water your plant");
    lcd.init(); // initialize the lcd
    lcd.init();
    lcd.setCursor(0, 0);
    lcd.print("Soil Dry:Pump on");
    digitalWrite(pumpPin, HIGH);
    Serial.println("Pump is On");
  }
  delay(50);
}

void loop()
{
  if (mySerial.available() > 0)
    Serial.write(mySerial.read());
  soilMoisture();
  waterLevel();
  Serial.print("\n");
}

