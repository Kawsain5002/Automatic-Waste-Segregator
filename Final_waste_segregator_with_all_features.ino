#include <CheapStepper.h>
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Initialize components
Servo servo1;
LiquidCrystal_I2C lcd(0x27, 16, 2); // Adjust address if needed
CheapStepper stepper(8, 9, 10, 11);

// Pin Definitions
#define ir 5       // Upper bin IR sensor
#define proxi 6    // Proximity sensor
#define buzzer 12  // Buzzer
#define dryIR A1   // Dry bin full detection IR sensor
#define wetIR A2   // Wet bin full detection IR sensor
#define metalIR A3 // Metal bin full detection IR sensor
int potPin = A0;   // Soil moisture sensor

int fsoil; // Final soil moisture value

void setup() {
  Serial.begin(9600);
  pinMode(proxi, INPUT_PULLUP);
  pinMode(ir, INPUT);
  pinMode(buzzer, OUTPUT);
  
  pinMode(dryIR, INPUT);
  pinMode(wetIR, INPUT);
  pinMode(metalIR, INPUT);

  servo1.attach(7);
  lcd.begin();
  lcd.backlight();
  stepper.setRpm(17);

  // Welcome Message
  lcd.setCursor(0, 0);
  lcd.print("Welcome");
  lcd.setCursor(0, 1);
  lcd.print("to the BIN");
  delay(5000);

  // Bin is Ready
  lcd.clear();
  lcd.print("Bin is ON");
  servo1.write(70); // Default position
  delay(1000);
}

void loop() {
  checkBinStatus(); // Check if bins are full
  
  fsoil = 0; // Reset soil moisture reading
  int L = digitalRead(proxi);

  if (L == 0) { // Metal detection
    tone(buzzer, 1000, 500); // Alert
    lcd.clear();
    lcd.print("Metal Waste");
    stepper.moveDegreesCCW(120); // Rotate stepper to metal bin
    delay(1000);
    openFlap(); // Open and close the flap
    stepper.moveDegreesCW(120); // Return stepper to initial position
    lcd.clear();
    lcd.print("Bin is ON");
  }

  if (digitalRead(ir) == 0) { // Waste detected in the upper bin
    tone(buzzer, 1000, 500); // Alert
    delay(1000);
    fsoil = readSoilMoisture(); // Read soil moisture to determine waste type

    if (fsoil > 20) { // Wet waste
      lcd.clear();
      lcd.print("Wet Waste");
      stepper.moveDegreesCW(120); // Rotate stepper to wet bin
      delay(1000);
      openFlap(); // Open and close the flap
      stepper.moveDegreesCCW(120); // Return stepper to initial position
    } else { // Dry waste
      lcd.clear();
      lcd.print("Dry Waste");
      openFlap(); // Open and close the flap for dry waste
    }
    lcd.clear();
    lcd.print("Bin is ON");
  }
}

// Check if bins are full
void checkBinStatus() {
  if (digitalRead(dryIR) == LOW) { // Dry bin full
    delay(3000); // Confirm with delay
    if (digitalRead(dryIR) == LOW) {
      buzzerAlert("Dry Bin is Full");
    }
  }
  if (digitalRead(wetIR) == LOW) { // Wet bin full
    delay(3000); // Confirm with delay
    if (digitalRead(wetIR) == LOW) {
      buzzerAlert("Wet Bin is Full");
    }
  }
  if (digitalRead(metalIR) == LOW) { // Metal bin full
    delay(3000); // Confirm with delay
    if (digitalRead(metalIR) == LOW) {
      buzzerAlert("Metal Bin is Full");
    }
  }
}

// Show full bin alert on LCD and sound buzzer
void buzzerAlert(String message) {
  while (true) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(message);
    lcd.setCursor(0, 1);
    lcd.print("Empty the Bin");
    tone(buzzer, 1000, 500);
    delay(1000);
    noTone(buzzer);

    // Check if bins have been emptied
    if (digitalRead(dryIR) == HIGH && digitalRead(wetIR) == HIGH && digitalRead(metalIR) == HIGH) {
      lcd.clear();
      lcd.print("Bin is ON");
      return;
    }
  }
}

// Read soil moisture to determine waste type
int readSoilMoisture() {
  int soil = 0;
  for (int i = 0; i < 3; i++) {
    soil = analogRead(potPin);
    soil = constrain(soil, 485, 1023);
    fsoil = (map(soil, 485, 1023, 100, 0)) + fsoil;
    delay(75);
  }
  return fsoil / 3; // Average of three readings
}

// Open and close flap for waste disposal
void openFlap() {
  servo1.write(180); // Open flap
  delay(1000);
  servo1.write(70);  // Close flap
  delay(1000);
}
