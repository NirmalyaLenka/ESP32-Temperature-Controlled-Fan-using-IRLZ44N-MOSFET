/*
  ESP32 Temperature-Controlled Fan
  --------------------------------
  Hardware : ESP32 DevKit v1, IRLZ44N logic-level MOSFET,
             DS18B20 digital temperature sensor, 12V DC fan.

  Behavior : Fan is OFF at or below 10 C.
             Fan runs at FULL SPEED at or above 35 C.
             Between 10 C and 35 C, speed increases linearly
             with temperature (PWM duty scales proportionally).

  Libraries required (install via Library Manager):
    - OneWire            by Jim Studt / Paul Stoffregen
    - DallasTemperature   by Miles Burton

  See README.md for wiring and COMPONENT_SPECS.md for part details.
*/

#include <OneWire.h>
#include <DallasTemperature.h>

// ---------------- Pin configuration ----------------
#define ONE_WIRE_BUS     4     // DS18B20 data line (through 4.7k pull-up to 3V3)
#define MOSFET_GATE_PIN  18    // Gate of IRLZ44N (through 220R series resistor)

// ---------------- PWM configuration ----------------
#define PWM_CHANNEL      0
#define PWM_FREQ         25000  // 25 kHz, above audible range, safe for MOSFET switching
#define PWM_RESOLUTION   8      // 8-bit -> duty range 0-255

// ---------------- Temperature thresholds ----------------
#define TEMP_MIN_C       10.0   // Fan OFF at/below this
#define TEMP_MAX_C       35.0   // Fan FULL SPEED at/above this

// ---------------- Sensor read timing ----------------
const unsigned long READ_INTERVAL_MS = 1000;
unsigned long lastReadTime = 0;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Convert a temperature reading into a 0-255 PWM duty value,
// linearly scaled between TEMP_MIN_C and TEMP_MAX_C.
int temperatureToDuty(float tempC) {
  if (tempC <= TEMP_MIN_C) return 0;
  if (tempC >= TEMP_MAX_C) return 255;

  float ratio = (tempC - TEMP_MIN_C) / (TEMP_MAX_C - TEMP_MIN_C);
  return (int)(ratio * 255.0 + 0.5); // round to nearest
}

void setup() {
  Serial.begin(115200);
  delay(200);

  sensors.begin();

  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(MOSFET_GATE_PIN, PWM_CHANNEL);
  ledcWrite(PWM_CHANNEL, 0); // fan off at boot

  Serial.println("ESP32 Fan Controller started.");
  Serial.println("Fan OFF <= 10C, FULL SPEED >= 35C, linear in between.");
}

void loop() {
  unsigned long now = millis();
  if (now - lastReadTime < READ_INTERVAL_MS) {
    return;
  }
  lastReadTime = now;

  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  if (tempC == DEVICE_DISCONNECTED_C) {
    Serial.println("ERROR: DS18B20 not detected. Check wiring. Fan forced OFF.");
    ledcWrite(PWM_CHANNEL, 0);
    return;
  }

  int duty = temperatureToDuty(tempC);
  ledcWrite(PWM_CHANNEL, duty);

  float percent = (duty / 255.0) * 100.0;
  Serial.print("Temp: ");
  Serial.print(tempC, 2);
  Serial.print(" C | PWM duty: ");
  Serial.print(duty);
  Serial.print("/255 | Fan speed: ");
  Serial.print(percent, 1);
  Serial.println(" %");
}
