#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <DHT.h>

// Pins and constants
#define echo 8
#define trigger 9
#define tank_pump 12
#define watering_pump 5
#define moisture_sensor A0
#define DHTPIN 10         // DHT11 sensor connected to digital pin 10
#define DHTTYPE DHT11     // Define sensor type as DHT11

// Variables
long duration;
int distance;
int moisture_value;
int distance_percent;
int moist_percent;
float temperature;
bool prev_tank_pump_state = LOW;
bool prev_watering_pump_state = LOW;

// LCD, GSM module, and DHT sensor initialization
LiquidCrystal_I2C lcd(0x27, 20, 4); // 20x4 LCD with I2C address 0x27
SoftwareSerial SIM800L(3, 2);       // SIM800L TX on pin 2, RX on pin 3
DHT dht(DHTPIN, DHTTYPE);           // Initialize DHT11 sensor

void setup() {
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(1, 1);
  lcd.print("IRRIGATION PROJECT");
  lcd.setCursor(6, 2);
  lcd.print("GROUP ONE");
  lcd.setCursor(7, 3);
  lcd.print("WELCOME");
  delay(2000);
  lcd.clear();

  // Initialize GSM module
  SIM800L.begin(9600);
  Serial.begin(9600);

  // Set pin modes
  pinMode(echo, INPUT);
  pinMode(trigger, OUTPUT);
  digitalWrite(trigger, LOW);
  pinMode(moisture_sensor, INPUT);
  pinMode(watering_pump, OUTPUT);
  pinMode(tank_pump, OUTPUT);
  digitalWrite(watering_pump, LOW);
  digitalWrite(tank_pump, LOW);

  // Initialize DHT sensor
  dht.begin();

  // Display setup complete message
  Serial.println("System Initialized.");
}

void loop() {
  // Level Sensor
  digitalWrite(trigger, LOW);
  delayMicroseconds(2);
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);
  duration = pulseIn(echo, HIGH);
  distance = duration * 0.017;
  distance_percent = map(distance, 0, 1023, 0, 100);

  // Moisture Sensor
  moisture_value = analogRead(moisture_sensor);
  moist_percent = map(moisture_value, 0, 1023, 0, 100);

  // Temperature from DHT11
  temperature = dht.readTemperature();

  // Display readings on the serial monitor
  Serial.print("Tank Level: ");
  Serial.print(distance_percent);
  Serial.print("%, Moisture Content: ");
  Serial.print(moist_percent);
  Serial.print("%, Temperature: ");
  Serial.print(temperature);
  Serial.println("°C");

  // Update LCD and conditions
  condition();
}

void condition() {
  bool current_tank_pump_state = LOW;
  bool current_watering_pump_state = LOW;

  if (distance_percent > 65 && moist_percent < 85) {
    LCD_3();
    current_tank_pump_state = LOW;
    current_watering_pump_state = HIGH;
  } else if (distance_percent < 65 && moist_percent > 85) {
    LCD_2();
    current_tank_pump_state = HIGH;
    current_watering_pump_state = LOW;
  } else if (distance_percent > 65 && moist_percent > 85) {
    LCD_4();
    current_tank_pump_state = LOW;
    current_watering_pump_state = LOW;
  } else if (distance_percent < 65 && moist_percent < 85) {
    LCD_1();
    current_tank_pump_state = HIGH;
    current_watering_pump_state = HIGH;
  }

  // Update pump states
  digitalWrite(tank_pump, current_tank_pump_state);
  digitalWrite(watering_pump, current_watering_pump_state);

  // Send SMS only when the pump states change
  if (current_tank_pump_state != prev_tank_pump_state || current_watering_pump_state != prev_watering_pump_state) {
    sendSMS(current_tank_pump_state, current_watering_pump_state);
    prev_tank_pump_state = current_tank_pump_state;
    prev_watering_pump_state = current_watering_pump_state;
  }

  // Log pump states to the serial monitor
  Serial.print("Tank Pump: ");
  Serial.print(current_tank_pump_state ? "ON" : "OFF");
  Serial.print(", Watering Pump: ");
  Serial.println(current_watering_pump_state ? "ON" : "OFF");

  delay(1000);
}

void sendSMS(bool tank_pump_state, bool watering_pump_state) {
  String smsContent = "Tank Pump: ";
  smsContent += (tank_pump_state ? "ON" : "OFF");
  smsContent += ", Watering Pump: ";
  smsContent += (watering_pump_state ? "ON" : "OFF");
  smsContent += ", Temperature: ";
  smsContent += temperature;
  smsContent += "°C";

  Serial.println("Preparing to send SMS...");
  Serial.print("SMS: ");
  Serial.println(smsContent);

  SIM800L.print("AT+CMGF=1\r"); // Set SMS to text mode

  SIM800L.print("AT+CMGS=\"+256761005390\"\r"); // Replace with recipient number
  delay(100);
  Serial.println("Recipient Number: +256761005390");

  SIM800L.print(smsContent);
  delay(100);
  SIM800L.write(26); // End SMS with CTRL+Z
  delay(1000);

  Serial.println("SMS Sent.");
}

void LCD_1() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TANK LEVEL= ");
  lcd.print(distance_percent);
  lcd.print("%");
  lcd.setCursor(0, 1);
  lcd.print("MOIST CONTENT= ");
  lcd.print(moist_percent);
  lcd.print("%");
  lcd.setCursor(0, 2);
  lcd.print("TEMP= ");
  lcd.print(temperature);
  lcd.print("C");
  lcd.setCursor(0, 3);
  lcd.print("W-PMP:ON T-PMP:ON");
}

void LCD_2() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TANK LEVEL= ");
  lcd.print(distance_percent);
  lcd.print("%");
  lcd.setCursor(0, 1);
  lcd.print("MOIST CONTENT= ");
  lcd.print(moist_percent);
  lcd.print("%");
  lcd.setCursor(0, 2);
  lcd.print("TEMP= ");
  lcd.print(temperature);
  lcd.print("C");
  lcd.setCursor(0, 3);
  lcd.print("W-PMP:OFF T-PMP:ON");
}

void LCD_3() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TANK LEVEL= ");
  lcd.print(distance_percent);
  lcd.print("%");
  lcd.setCursor(0, 1);
  lcd.print("MOIST CONTENT= ");
  lcd.print(moist_percent);
  lcd.print("%");
  lcd.setCursor(0, 2);
  lcd.print("TEMP= ");
  lcd.print(temperature);
  lcd.print("C");
  lcd.setCursor(0, 3);
  lcd.print("W-PMP:ON T-PMP:OFF");
}

void LCD_4() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TANK LEVEL= ");
  lcd.print(distance_percent);
  lcd.print("%");
  lcd.setCursor(0, 1);
  lcd.print("SOIL MOIST= ");
  lcd.print(moist_percent);
  lcd.print("%");
  lcd.setCursor(0, 2);
  lcd.print("TEMP= ");
  lcd.print(temperature);
  lcd.print("C");
  lcd.setCursor(0, 3);
  lcd.print("W-PMP:OFF T-PMP:OFF");
}
