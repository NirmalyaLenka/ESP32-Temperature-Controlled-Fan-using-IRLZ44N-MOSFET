# ESP32 Temperature-Controlled Fan
 
A fan speed controller built with an ESP32, an IRLZ44N MOSFET, and a DS18B20 temperature sensor.
 
- Fan is fully **OFF at 10°C and below**
- Fan runs at **full speed at 35°C and above**
- Between 10°C and 35°C, fan speed increases **linearly** with temperature
Files in this project:
 
| File                  | Purpose                                      |
|------------------------|------------------------------------------------|
| `fan_control.ino`      | Firmware to upload to the ESP32                |
| `README.md`            | This file — build and setup instructions       |
| `COMPONENT_SPECS.md`   | Electrical specs for every part used            |
| `wiring_diagram.svg`   | Full connection diagram (open in a browser or image viewer) |
 
---
 
## 1. Parts you need
 
- 1x ESP32 DevKit v1 (30-pin board)
- 1x IRLZ44N MOSFET (TO-220 package)
- 1x DS18B20 waterproof temperature probe
- 1x 12V DC fan (2-wire type)
- 1x 12V DC power supply, current rating higher than the fan's rated current
- 1x 4.7 kΩ resistor
- 1x 220 Ω resistor
- 1x 10 kΩ resistor
- 1x MBR3045PT Schottky diode (TO-220AB, dual diode — only one leg used)
- Breadboard and jumper wires
- USB cable to program the ESP32
Full electrical specs for each part are in `COMPONENT_SPECS.md`.
 
---
 
## 2. Understanding the circuit (in plain language)
 
The fan needs 12V and more current than the ESP32 can safely supply, so the ESP32 never powers the fan directly. Instead:
 
1. The fan's **positive (+)** wire connects straight to the 12V supply's **+12V**.
2. The fan's **negative (-)** wire connects to the MOSFET's **Drain** pin, instead of straight to ground.
3. The MOSFET's **Source** pin connects to ground (shared by the ESP32 and the 12V supply).
4. The ESP32 sends a PWM signal to the MOSFET's **Gate**. When the gate is "high," the MOSFET connects the fan's ground path and current flows. When it switches rapidly on/off (PWM), the fan effectively sees a lower average voltage, so it spins slower.
5. The DS18B20 sensor reads temperature and reports it to the ESP32 over a single data wire (OneWire protocol).
6. The ESP32 reads the temperature every second and adjusts the PWM duty cycle sent to the MOSFET gate accordingly.
Open `wiring_diagram.svg` in any web browser for the full visual diagram with every wire and pin labeled.
 
---
 
## 3. Step-by-step wiring
 
### Step 1 — Wire the DS18B20 sensor
1. Connect DS18B20 **red (VCC)** wire to ESP32 **3V3**.
2. Connect DS18B20 **black (GND)** wire to ESP32 **GND**.
3. Connect DS18B20 **yellow/white (DATA)** wire to ESP32 **GPIO4**.
4. Place a **4.7 kΩ resistor** between the DATA wire and the 3V3 wire (this is the required OneWire pull-up — without it, readings will fail or be unreliable).
### Step 2 — Wire the MOSFET
1. Identify the IRLZ44N pins: holding the part with the flat metal face toward you and pins pointing down, the order left-to-right is **Gate, Drain, Source**.
2. Connect **Gate** to ESP32 **GPIO18** through a **220 Ω resistor** (this protects the GPIO pin).
3. Add a **10 kΩ resistor** between **Gate** and **GND** (this keeps the MOSFET reliably OFF if the ESP32 resets or the pin floats before setup runs).
4. Connect **Source** to the common ground rail (same ground as ESP32 and the 12V supply).
5. Leave **Drain** for the fan connection (next step).
### Step 3 — Wire the fan and power supply
1. Connect the 12V supply's **+12V** to the fan's **(+)** wire.
2. Connect the fan's **(-)** wire to the MOSFET's **Drain** pin.
3. Connect the 12V supply's **GND** to the same common ground rail as the ESP32 and MOSFET Source.
4. Add the **MBR3045PT diode** directly across the fan's two wires. This part has 3 legs (it's a dual diode in one package) — holding it with the flat/metal face toward you and legs down, the legs are **Anode 1 — Common Cathode — Anode 2**:
   - Connect the **center pin (Common Cathode)** to the **+12V** side (same node as fan (+)).
   - Connect **either outer pin (Anode)** to the fan **(-)** / MOSFET Drain node.
   - Leave the remaining outer pin (the second, unused anode) disconnected.
   - This protects the MOSFET from voltage spikes generated when the fan's motor coil switches off, same job as a standard 1N4007 but with a lower voltage drop and much higher current headroom than this fan needs.
### Step 4 — Double-check grounds
Confirm with a multimeter (continuity mode) that ESP32 GND, MOSFET Source, and 12V supply GND are all electrically connected. This is the single most common mistake in this type of circuit — if the grounds aren't shared, the fan simply won't respond to PWM.
 
---
 
## 4. Software setup
 
1. Install the [Arduino IDE](https://www.arduino.cc/en/software) (or use PlatformIO).
2. Add ESP32 board support: **File > Preferences > Additional Board URLs**, add:
   `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   Then go to **Tools > Board > Boards Manager**, search "esp32", install it.
3. Install two libraries via **Sketch > Include Library > Manage Libraries**:
   - `OneWire`
   - `DallasTemperature`
4. Open `fan_control.ino` in the Arduino IDE.
5. Select **Tools > Board > ESP32 Dev Module**.
6. Select the correct COM port under **Tools > Port**.
7. Click **Upload**.
8. Open **Tools > Serial Monitor** at **115200 baud** to watch live temperature and fan speed readings.
---
 
## 5. Testing
 
1. With everything wired and powered, open the Serial Monitor.
2. At room temperature (usually below 25°C), the fan should be spinning at a partial speed proportional to the temperature — completely off at 10°C or below.
3. Warm the DS18B20 tip gently (cup it in your hand, or use a hair dryer at a safe distance) and watch the reported temperature rise and the fan speed increase in the Serial Monitor.
4. At 35°C or above, the fan should be running at full speed.
5. Let it cool back down and confirm the fan slows and eventually turns off again below 10°C.
---
 
## 6. Troubleshooting
 
| Symptom                                  | Likely cause                                                        |
|--------------------------------------------|------------------------------------------------------------------------|
| Serial Monitor shows "DS18B20 not detected" | Missing 4.7kΩ pull-up resistor, or DATA/VCC/GND wires swapped/loose  |
| Fan never turns on                        | Grounds not shared between ESP32, MOSFET, and 12V supply               |
| Fan runs at full speed only (no throttling) | MOSFET wired as high-side switch instead of low-side, or gate resistor missing causing erratic switching |
| Fan speed is jumpy/flickers                | Loose breadboard connections, or PWM frequency too low (audible buzzing can also indicate this) |
| ESP32 resets when fan turns on             | 12V supply and ESP32 sharing power incorrectly — power the ESP32 from USB, not from the 12V rail |
 
---
 
## 7. Customizing
 
All thresholds are defined near the top of `fan_control.ino`:
 
```cpp
#define TEMP_MIN_C   10.0   // fan OFF at/below this
#define TEMP_MAX_C   35.0   // fan FULL SPEED at/above this
```
 
Change these two values and re-upload to adjust the operating range.
 
