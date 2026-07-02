#include <EEPROM.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

long duration, inches;
int set_val, percentage;
bool state, pump;

void setup() {
  lcd.begin(16, 2);
  lcd.print("WATER LEVEL:");
  lcd.setCursor(0, 1);
  lcd.print("PUMP:OFF MANUAL");

  pinMode(8, OUTPUT);   // Ultrasonic Trigger
  pinMode(9, INPUT);    // Ultrasonic Echo
  pinMode(10, INPUT_PULLUP);  // Button (Set/Manual)
  pinMode(11, INPUT_PULLUP);  // Mode Switch (Manual/Auto)
  pinMode(12, OUTPUT);  // Pump Relay

  set_val = EEPROM.read(0);
  if (set_val > 150) set_val = 150;
}

void loop() {
  // ── Ultrasonic Trigger ──
  digitalWrite(8, LOW);        // FIXED: was pin 3 (wrong pin), should be pin 8
  delayMicroseconds(2);
  digitalWrite(8, HIGH);
  delayMicroseconds(10);
  digitalWrite(8, LOW);

  // ── Read Echo ──
  duration = pulseIn(9, HIGH);
  inches = microsecondsToInches(duration);

  // ── Calculate Percentage ──
  percentage = (set_val - inches) * 100 / set_val;

  // ── Display Percentage ──
  lcd.setCursor(12, 0);
  if (percentage < 0) percentage = 0;
  lcd.print(percentage);
  lcd.print("%  ");

  // ── Pump Control (AUTO mode only) ──
  if (percentage < 30 && digitalRead(11)) pump = 1;   // FIXED: & → &&
  if (percentage > 99) pump = 0;
  digitalWrite(12, !pump);    // Active-low relay

  // ── Display Pump Status ──
  lcd.setCursor(5, 1);
  if (pump == 1) lcd.print("ON ");
  else if (pump == 0) lcd.print("OFF");

  // ── Display Mode (MANUAL / AUTO) ──
  lcd.setCursor(9, 1);
  if (!digitalRead(11)) lcd.print("MANUAL");
  else lcd.print("AUTO  ");

  // ── Button 10: Set max level (in AUTO mode) ──
  if (!digitalRead(10) && !state && digitalRead(11)) {
    state = 1;
    set_val = inches;
    EEPROM.write(0, set_val);
  }

  // ── Button 10: Toggle pump (in MANUAL mode) ──
  // FIXED: was checking same condition as above (both AUTO), should be !digitalRead(11) for MANUAL
  if (!digitalRead(10) && !state && !digitalRead(11)) {
    state = 1;
    pump = !pump;
  }

  if (digitalRead(10)) state = 0;

  delay(500);
}

long microsecondsToInches(long microseconds) {
  return microseconds / 74 / 2;
}