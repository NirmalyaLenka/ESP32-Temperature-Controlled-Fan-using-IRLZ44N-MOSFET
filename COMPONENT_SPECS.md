# Component Specifications

Bill of materials and key electrical specs for the ESP32 temperature-controlled fan project.

## 1. ESP32 DevKit v1 (30-pin, ESP32-WROOM-32 module)

| Parameter               | Value                                   |
|--------------------------|------------------------------------------|
| MCU                      | Xtensa dual-core 32-bit LX6, up to 240 MHz |
| Operating voltage        | 3.3 V (logic level)                     |
| Input supply (USB/VIN)   | 5 V via USB or VIN pin                  |
| GPIO output voltage      | 3.3 V (NOT 5V tolerant on most pins)    |
| GPIO max source/sink current | ~40 mA per pin (12 mA recommended continuous) |
| PWM (LEDC peripheral)    | Up to 16 channels, up to 40 MHz, configurable resolution (used here: 25 kHz, 8-bit) |
| Onboard 3V3 regulator output | Typically 500 mA-600 mA (varies by board) |
| Flash                    | Typically 4 MB                          |
| Connectivity             | Wi-Fi 802.11 b/g/n, Bluetooth Classic + BLE |

**Relevant pins used in this project:**
- GPIO4 — OneWire data bus (DS18B20)
- GPIO18 — PWM gate drive signal to MOSFET
- 3V3 — sensor and pull-up supply
- GND — common ground

## 2. IRLZ44N — Logic-Level N-Channel Power MOSFET

| Parameter                        | Value                          |
|-----------------------------------|---------------------------------|
| Type                              | N-channel, logic-level gate     |
| Drain-source voltage, V(DS)       | 55 V max                        |
| Continuous drain current, I(D)    | 47 A (at 25°C, derate with heat)|
| Gate threshold voltage, V(GS,th)  | 1.0 V - 2.0 V (fully logic-level, switches reliably from 3.3 V) |
| R(DS,on) at V(GS) = 5V            | ~0.022 Ω (typ.)                 |
| R(DS,on) at V(GS) = 4V            | ~0.028 Ω (typ.)                 |
| Package                           | TO-220                          |
| Pinout (facing flat side, pins down) | Gate — Drain — Source (left to right) |

**Why this MOSFET works directly with ESP32 (3.3 V logic):**
IRLZ44N is a *logic-level* MOSFET, meaning it fully turns on with a 3.3 V-5 V gate signal. A standard (non-logic-level) MOSFET like an IRF540 would not fully saturate at 3.3 V and would overheat — do not substitute without checking V(GS,th).

**Role in circuit:** Low-side switch. The fan's negative terminal connects to the MOSFET drain; the MOSFET source goes to ground. The ESP32 PWM signal drives the gate, chopping the fan's ground connection on and off to control average power delivered (and therefore speed).

## 3. DS18B20 — Digital Temperature Sensor

| Parameter                | Value                                   |
|----------------------------|------------------------------------------|
| Interface                 | 1-Wire (OneWire) digital bus            |
| Supply voltage             | 3.0 V - 5.5 V                           |
| Measurement range          | -55°C to +125°C                         |
| Accuracy                   | ±0.5°C over -10°C to +85°C              |
| Resolution                 | Configurable 9-12 bit (default 12-bit, 0.0625°C steps) |
| Conversion time (12-bit)   | ~750 ms                                 |
| Package used here          | Waterproof probe (stainless steel tip, 3 wires: red = VCC, black = GND, yellow/white = DATA) |
| Required external part     | 4.7 kΩ pull-up resistor between DATA and VCC |

## 4. Fan (12 V DC brushless fan, 2-wire type)

| Parameter                | Typical Value                           |
|----------------------------|------------------------------------------|
| Rated voltage              | 12 V DC                                 |
| Rated current               | 0.1 A - 0.3 A (check your specific fan's label) |
| Wires                       | 2 (VCC / GND) — this project controls speed by chopping the ground return with the MOSFET |

> If your fan is a 4-pin PC fan (with a dedicated PWM control wire and tachometer), you do not need the MOSFET at all — connect the fan's PWM wire directly to GPIO18 (with a 100Ω series resistor) and power VCC/GND directly from 12V. This project assumes a simple 2-wire fan.

## 5. MBR3045PT — Dual Schottky Barrier Rectifier (flyback diode)

| Parameter                        | Value                                    |
|------------------------------------|---------------------------------------------|
| Type                                | Dual Schottky diode, common cathode          |
| Reverse voltage, V(RRM)            | 45 V                                         |
| Forward current, I(F)              | 30 A total (15 A per diode leg)              |
| Forward voltage drop, V(F)         | ~0.57 V typ. at rated current (much lower than a silicon diode like 1N4007's ~0.9-1.1 V) |
| Reverse leakage current            | Higher than silicon diodes (normal for Schottky), negligible at 12 V |
| Package                            | TO-220AB, 3-lead                             |
| Pinout (facing flat side, pins down) | Pin 1 = Anode 1 — Pin 2 = Common Cathode (tied to metal tab) — Pin 3 = Anode 2 |

**This part is a dual-diode package** — two independent diodes sharing one cathode. This project only needs **one** diode, so only one anode pin is used; the other anode pin (and its diode) is left unconnected.

**Role in circuit:** Same flyback/freewheeling function as a standard diode across the fan terminals, just with a lower forward voltage drop and far higher current headroom than this small fan needs — useful if you later drive a larger fan, pump, or relay coil from the same MOSFET.

**Wiring for this project:**
- **Cathode (Pin 2, center pin)** → +12V rail (same node as fan (+) and 12V supply (+))
- **Anode (Pin 1 or Pin 3, either one)** → MOSFET Drain / fan (−) node
- The unused anode pin is left disconnected (not grounded, not tied to anything)

## 6. Supporting components (BOM)

| Part                                  | Qty | Purpose                                      |
|-----------------------------------------|-----|-----------------------------------------------|
| ESP32 DevKit v1                          | 1   | Controller                                    |
| IRLZ44N MOSFET (TO-220)                  | 1   | Fan speed switching                           |
| DS18B20 waterproof probe                 | 1   | Temperature sensing                           |
| 4.7 kΩ resistor (1/4W)                   | 1   | OneWire pull-up (DS18B20 DATA to VCC)         |
| 220 Ω resistor (1/4W)                    | 1   | Gate series resistor (protects GPIO18)        |
| 10 kΩ resistor (1/4W)                    | 1   | Gate pull-down (ensures MOSFET off if ESP32 resets/floats) |
| MBR3045PT Schottky diode (TO-220AB)      | 1   | Flyback/freewheeling diode across fan terminals (only one of the two internal diodes used) |
| 12 V DC power supply                     | 1   | Powers the fan (sized to fan current rating)  |
| Breadboard + jumper wires                | -   | Prototyping                                   |
| Common ground rail                       | -   | ESP32 GND must connect to 12V supply GND      |

