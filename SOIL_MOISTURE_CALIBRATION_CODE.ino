// Define the analog pin connected to the soil moisture sensor
const int soilMoisturePin = A0;

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(9600);

  // Set up the analog pin as input (optional)
  pinMode(soilMoisturePin, INPUT);

  // Print a startup message
  Serial.println("Reading Analog Values from Soil Moisture Sensor...");
}

void loop() {
  // Read the analog value from the sensor
  int analogValue = analogRead(soilMoisturePin);

  // Print the raw analog value
  Serial.print("Analog Value: ");
  Serial.println(analogValue);

  // Add a delay for readability in the Serial Monitor
  delay(500);
}
