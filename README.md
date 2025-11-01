# 🔥 Gas & Fire Detection System with ESP32

<div align="center">

![Project Status](https://img.shields.io/badge/Status-Active-success?style=for-the-badge)
![Platform](https://img.shields.io/badge/Platform-ESP32-blue?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)

**An intelligent IoT-based safety system that detects gas leaks and fire, with real-time monitoring via Blynk app and automated response mechanisms.**

[Features](#-features) • [Hardware](#-hardware-requirements) • [Installation](#-installation) • [Usage](#-usage) • [Troubleshooting](#-troubleshooting)

</div>

---

## 📋 Table of Contents

- [Overview](#-overview)
- [Features](#-features)
- [System Architecture](#-system-architecture)
- [Hardware Requirements](#-hardware-requirements)
- [Software Requirements](#-software-requirements)
- [Pin Configuration](#-pin-configuration)
- [Installation](#-installation)
- [Configuration](#-configuration)
- [Usage](#-usage)
- [Blynk Setup](#-blynk-setup)
- [Web Interface](#-web-interface)
- [How It Works](#-how-it-works)
- [Troubleshooting](#-troubleshooting)
- [FAQ](#-faq)
- [License](#-license)

---

## 🌟 Overview

This project implements a **comprehensive safety monitoring system** using ESP32 microcontroller that can:

- 🔍 **Detect gas leaks** (LPG, propane, methane, etc.) using MQ-2 sensor
- 🔥 **Detect fire** using infrared flame sensor
- 📱 **Send real-time alerts** to your smartphone via Blynk
- 🚨 **Activate automated responses** (buzzer, ventilation fan, water pump)
- 🚪 **Open emergency ventilation door** automatically
- 💻 **Monitor and control** remotely through Blynk mobile app
- 🌐 **Easy WiFi configuration** through captive web portal (supports open networks!)
- 🔕 **Smart buzzer control** - Silence buzzer while keeping safety systems active

Perfect for homes, kitchens, laboratories, industrial facilities, or anywhere gas and fire safety is critical!

---

## ✨ Features

### 🛡️ Safety Features
- ✅ **Dual Hazard Detection**: Gas concentration & flame detection
- ✅ **Instant Alerts**: Buzzer alarm + Blynk push notifications
- ✅ **Automatic Response**: Fan activation, water pump, door opening
- ✅ **Smart Buzzer Control**: Silence buzzer without stopping safety systems
- ✅ **Auto-Reactivation**: Buzzer reactivates on new alert after being silenced
- ✅ **Sensor Warm-up**: 60-second calibration period on startup

### 📱 Smart Connectivity
- ✅ **Blynk IoT Integration**: Monitor from anywhere in the world
- ✅ **Real-time Data**: Gas levels updated every 2 seconds
- ✅ **Remote Control**: Control relays and door from app
- ✅ **Customizable Threshold**: Adjust gas sensitivity remotely
- ✅ **Event Logging**: Historical alerts stored in Blynk

### 💻 User Experience
- ✅ **16x2 LCD Display**: Shows system status and gas readings
- ✅ **Beautiful Web Portal**: Animated, professional interface
- ✅ **Open WiFi Support**: Configure with or without password
- ✅ **WiFi Manager**: Easy network setup without code changes
- ✅ **Auto-Reconnect**: Handles WiFi/Blynk disconnections
- ✅ **Visual Feedback**: LED indicators for system status

### ⚙️ Technical Features
- ✅ **FreeRTOS Multitasking**: Efficient parallel processing
- ✅ **Kalman Filtering**: Noise reduction for stable readings
- ✅ **Hysteresis Logic**: Prevents sensor fluctuation issues
- ✅ **EEPROM Storage**: Remembers settings after power loss
- ✅ **Dual-Core Processing**: Tasks distributed across ESP32 cores

---

## 🏗️ System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                         ESP32                               │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐       │
│  │   Core 0     │  │   Core 1     │  │   Sensors    │       │
│  ├──────────────┤  ├──────────────┤  ├──────────────┤       │
│  │ • WebServer  │  │ • Main Disp  │  │ • MQ-2 (Gas) │       │
│  │ • Blynk      │  │ • Buzzer     │  │ • MH (Fire)  │       │
│  │              │  │ • Button     │  │              │       │
│  └──────────────┘  └──────────────┘  └──────────────┘       │
│                                                             │
│  ┌──────────────────────────────────────────────────────┐   │
│  │              Output Devices                          │   │
│  ├──────────────────────────────────────────────────────┤   │
│  │ • LCD 16x2 (I2C)    • Servo Door (1x)                │   │
│  │ • Relay Fan         • Relay Pump                     │   │
│  │ • Buzzer            • LED Indicator                  │   │
│  └──────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
                            │
                            ├── WiFi ──► Router ──► Internet
                            │                          │
                            └──────────────────────────┴──► Blynk Cloud
                                                            
```

---

## 🔧 Hardware Requirements

### Main Components

| Component | Specification | Quantity | Purpose |
|-----------|---------------|----------|---------|
| **ESP32 Dev Board** | ESP32-WROOM-32 | 1 | Main microcontroller |
| **MQ-2 Gas Sensor** | Analog output | 1 | Detects LPG, propane, methane |
| **Flame Sensor** | IR-based (MH-Sensor) | 1 | Detects fire/flame |
| **LCD Display** | 16x2 I2C (0x27) | 1 | Status display |
| **Servo Motor** | SG90 or similar | 1 | Emergency door control |
| **Relay Module** | 5V 2-Channel | 1 | Controls fan & pump |
| **Buzzer** | Active 5V | 1 | Audio alarm |
| **Push Button** | Momentary switch | 1 | Silence buzzer |
| **LED** | 5mm any color | 1 | Visual indicator |

### Supporting Components

- Resistors: 10kΩ (for button pull-up - optional, using internal pull-up)
- Jumper wires (Male-Female, Male-Male)
- Breadboard or PCB
- Power supply: 5V 2A minimum
- Water pump (12V, for fire suppression)
- Ventilation fan (12V, for gas dispersal)

### Optional Enhancements

- External antenna for better WiFi range
- Battery backup (18650 Li-ion + TP4056 module)
- Weatherproof enclosure for outdoor use
- Additional sensors (CO, smoke, temperature)

---

## 💾 Software Requirements

### Development Environment

- **PlatformIO IDE** (recommended) or Arduino IDE
- **ESP32 Board Support** (version 2.0.0 or higher)
- **USB Drivers**: CP210x or CH340 (depending on your board)

### Required Libraries

```ini
[env:esp32]
platform = espressif32
board = esp32dev
framework = arduino

lib_deps = 
    blynkkk/Blynk@^1.3.2
    marcoschwartz/LiquidCrystal_I2C@^1.1.4
    denyssene/SimpleKalmanFilter@^0.1.0
    madhephaestus/ESP32Servo@^3.0.5
```

### Mobile App

- **Blynk IoT** (iOS/Android)
  - Download: [App Store](https://apps.apple.com) | [Google Play](https://play.google.com)
  - Free account with limited energy
  - Premium subscription for advanced features

---

## 📍 Pin Configuration

### ESP32 Pin Mapping

```cpp
// Sensors
#define MQ2_SENSOR    35    // Analog input (ADC1_CH7)
#define MH_SENSOR     34    // Digital input (ADC1_CH6)

// Actuators
#define SERVO         33    // PWM output (single door servo)
#define RELAY_FAN     18    // Fan control
#define RELAY_PUMP    5     // Pump control
#define BUZZER        23    // Buzzer control
#define LED           19    // Status LED

// User Interface
#define BUTTON        4     // Silence buzzer button (INPUT_PULLUP)

// I2C (LCD)
#define SDA           21    // Default I2C SDA
#define SCL           22    // Default I2C SCL
```

### Wiring Diagram

```
ESP32          MQ-2 Sensor
-----          -----------
 3V3    ────►  VCC
 GND    ────►  GND
 GPIO35 ────►  AO (Analog Out)

ESP32          Flame Sensor
-----          ------------
 3V3    ────►  VCC
 GND    ────►  GND
 GPIO34 ────►  DO (Digital Out)

ESP32          LCD I2C
-----          --------
 3V3    ────►  VCC
 GND    ────►  GND
 GPIO21 ────►  SDA
 GPIO22 ────►  SCL

ESP32          Servo Motor
-----          -----------
 5V     ────►  VCC (Red)
 GND    ────►  GND (Brown)
 GPIO33 ────►  Signal (Orange)

ESP32          Relay Module
-----          ------------
 5V     ────►  VCC
 GND    ────►  GND
 GPIO18 ────►  IN1 (Fan)
 GPIO5  ────►  IN2 (Pump)

ESP32          Buzzer
-----          ------
 GPIO23 ────►  Positive (+)
 GND    ────►  Negative (-)

ESP32          Button
-----          ------
 GPIO4  ────►  One side
 GND    ────►  Other side
 (Uses internal pull-up resistor)

ESP32          LED
-----          ---
 GPIO19 ────►  Anode (+) ──┬── 220Ω
 GND    ───────────────────┘
```

---

## 🚀 Installation

### Step 1: Hardware Assembly

1. **Connect the ESP32 to breadboard**
2. **Wire sensors** according to pin configuration
3. **Connect actuators** (servo, relays, buzzer, LED)
4. **Install LCD display** with I2C adapter
5. **Add push button** (uses internal pull-up)
6. **Double-check connections** before powering on

⚠️ **Important**: 
- MQ-2 sensor needs 24-48 hours of burn-in time for accurate readings
- Use separate power supply for relays if controlling high-current devices
- Ensure proper grounding to avoid sensor noise
- Button uses internal pull-up - no external resistor needed

### Step 2: Software Installation

#### Using PlatformIO (Recommended)

```bash
# 1. Install PlatformIO IDE extension in VSCode

# 2. Clone the repository
git clone https://github.com/yourusername/gas-fire-detection-esp32.git
cd gas-fire-detection-esp32

# 3. Open project in VSCode
code .

# 4. PlatformIO will auto-install dependencies

# 5. Edit configuration files
# - Update Blynk template ID in main.cpp (line 2)
# - Adjust pin numbers if needed in def.h

# 6. Build and upload
pio run --target upload

# 7. Monitor serial output
pio device monitor --baud 9600
```

#### Using Arduino IDE

```bash
# 1. Install Arduino IDE

# 2. Add ESP32 board support
# File → Preferences → Additional Board URLs:
# https://dl.espressif.com/dl/package_esp32_index.json

# 3. Install required libraries
# Tools → Manage Libraries → Search and install:
#   - Blynk (by Volodymyr Shymanskyy)
#   - LiquidCrystal I2C
#   - SimpleKalmanFilter
#   - ESP32Servo

# 4. Open main.cpp and configure Blynk credentials (lines 1-3)

# 5. Select board: ESP32 Dev Module

# 6. Upload to ESP32
```

### Step 3: File Structure

```
gas-fire-detection-esp32/
│
├── src/
│   ├── main.cpp              # Main program file
│   ├── def.h                 # Pin definitions
│   └── config.h              # Configuration variables
│
├── include/
│   └── README                # Include directory info
│
├── lib/
│   └── README                # Library directory info
│
├── platformio.ini            # PlatformIO configuration
├── README.md                 # This file
└── LICENSE                   # License file
```

---

## ⚙️ Configuration

### 1. Blynk Configuration

Edit `main.cpp` (lines 1-3):

```cpp
// Replace with your Blynk credentials
char BLYNK_AUTH_TOKEN[32] = "";                         // Leave empty
#define BLYNK_TEMPLATE_ID       "TMPL6pfVLC6bj"        // Your Template ID
#define BLYNK_TEMPLATE_NAME     "Gas and Fire Detection"
```

### 2. Pin Configuration (Optional)

Edit `def.h` if you want to change pins:

```cpp
// Modify these values to match your wiring
#define MQ2_SENSOR    35    // Change to your MQ-2 pin
#define MH_SENSOR     34    // Change to your flame sensor pin
#define SERVO         33    // Single servo for door
// ... etc
```

### 3. Gas Threshold (Optional)

Edit in `main.cpp` or change via Blynk app:

```cpp
#define GAS_THRESHOLD 2200  // Default threshold (0-10000 ppm scale)
```

### 4. WiFi Access Point (Optional)

Edit `config.h`:

```cpp
#define APssid      "ESP32"        // Change AP name
#define APpassword  ""             // Leave empty for open AP
```

---

## 📱 Blynk Setup

### Step 1: Create Blynk Account

1. Download **Blynk IoT** app ([iOS](https://apps.apple.com/app/blynk-iot/id1559317868) | [Android](https://play.google.com/store/apps/details?id=cloud.blynk))
2. Sign up for free account
3. Create new template

### Step 2: Configure Blynk Template

1. **Go to Templates** → **Create New Template**
2. **Name**: Gas and Fire Detection
3. **Hardware**: ESP32
4. **Connection**: WiFi

### Step 3: Add Datastreams

Create the following datastreams:

| Pin | Name | Type | Min | Max | Default |
|-----|------|------|-----|-----|---------|
| V0 | Gas Value | Virtual | 0 | 10000 | 0 |
| V1 | Relay Control | Virtual | 0 | 3 | 0 |
| V2 | Door Control | Virtual | 0 | 1 | 0 |
| V3 | Gas Threshold | Virtual | 200 | 10000 | 2200 |

### Step 4: Design Dashboard

Add these widgets to your dashboard:

1. **Gauge** (V0) - Gas concentration display
   - Units: ppm
   - Range: 0-10000
   - Color zones: Green (0-2000), Yellow (2000-3000), Red (3000+)

2. **Slider** (V1) - Relay control (0-3)
   - 0 = All OFF
   - 1 = Fan ON, Pump OFF
   - 2 = Fan OFF, Pump ON
   - 3 = Both ON

3. **Switch** (V2) - Door control
   - OFF = Closed (0°)
   - ON = Open (90°)

4. **Slider** (V3) - Threshold adjustment
   - Range: 200-5000 ppm
   - Default: 2200

5. **Chart** - Historical gas data (V0)
   - Time range: 6 hours
   - Y-axis: 0-10000

6. **Terminal** - Event log
   - Shows alerts and system messages

### Step 5: Configure Events

1. **Go to Events** → **Create Event**
2. **Event Code**: `gas_fire_detection`
3. **Name**: Gas/Fire Alert
4. **Notification**: "⚠️ {DEVICE_NAME} Alert: {EVENT_DESCRIPTION}"
5. **Enable Push Notifications**

### Step 6: Get Auth Token

1. Go to **Device** → **Create Device**
2. Select your template
3. Name your device (e.g., "Kitchen Safety Monitor")
4. **Copy Auth Token**
5. Enter via web configuration portal at 192.168.4.1

---

## 🌐 Web Interface

### Accessing Configuration Portal

#### First Time Setup (No WiFi Configured)

1. **Power on ESP32**
2. **Wait 10 seconds** for LCD to show "Connect ESP32"
3. **On your phone/laptop**:
   - Open WiFi settings
   - Connect to network: **"ESP32"**
   - Password: *(none - open network)*
4. **Browser will auto-open** to configuration page
   - If not, manually navigate to: **http://192.168.4.1**

### Configuration Page Features

The web interface includes:

- 🎨 **Beautiful animated gradient background**
- 🔥 **Pulsing fire icon** with glow effects
- 📝 **Three input fields**:
  - **WiFi SSID** (network name) - Required
  - **WiFi Password** - **OPTIONAL** (leave empty for open networks!)
  - **Blynk Token** (32 characters) - Required
- ✅ **Real-time validation**
- 💾 **Automatic save and restart**

### Filling Out the Form

1. **WiFi SSID** (Required):
   - Enter your WiFi network name
   - Case-sensitive
   - Max 32 characters

2. **WiFi Password** (Optional):
   - Enter your WiFi password
   - **Leave EMPTY for open/public WiFi networks**
   - Click 👁️ icon to show/hide
   - Max 64 characters
   - Spaces allowed

3. **Blynk Token** (Required):
   - Paste 32-character token from Blynk app
   - Counter shows character count (must be exactly 32)
   - Green = valid, Red = invalid length

4. **Click "💾 SAVE CONFIGURATION"**
   - Page will show "⏳ Saving..."
   - ESP32 will restart automatically
   - Wait 30-60 seconds for connection

### Success Page

After saving, you'll see:
- ✅ **Success checkmark animation**
- 📡 **Connecting status** with spinner
- ⏳ **Wait time** (30-60 seconds)
- 📋 **Connected network** name

---

## 🎮 Usage

### System Startup Sequence

```
1. Power On
   └── LCD: "Gas and Fire Detection System"
   
2. Hardware Initialization
   └── LCD: "Configuring WiFi..."
   
3. WiFi Connection
   ├── Success: "WiFi Connected" → Shows SSID
   └── Fail: "Connect ESP32" → AP mode
   
4. Blynk Connection
   ├── Success: "Blynk Connected"
   └── Fail: "Disconnect Blynk" → Retry or AP mode
   
5. Sensor Warm-up
   └── LCD: "Warming Up Sensors... Wait: 60 (s)"
       (Counts down from 60 to 0)
   
6. System Ready
   └── LCD: "System running"
           "Gas: XXXX ppm"
```

### Normal Operation

**LCD Display** (2 rows x 16 columns):
```
Row 1: "System running  "
Row 2: "Gas:2150ppm    "
```

**Every 2 seconds**:
- Reads gas sensor (with Kalman filtering)
- Reads fire sensor
- Updates LCD display
- Sends data to Blynk
- Evaluates hazard conditions

### Alert Scenarios

#### 🟢 Scenario 1: Normal (All Safe)

**Conditions:**
- Gas < Threshold
- No Fire

**Actions:**
- ✅ LCD: "System running" + gas value
- ✅ Buzzer: OFF
- ✅ LED: OFF
- ✅ Door: Closed (0°)
- ✅ Fan: OFF
- ✅ Pump: OFF
- ✅ User silence flag: Reset

---

#### 🟡 Scenario 2: Gas Detected Only

**Conditions:**
- Gas > Threshold
- No Fire

**Actions:**
- 🚨 LCD: "WARNING! GAS DETECTED"
- 🚨 Buzzer: Beeping (1s ON, 0.1s OFF) - unless silenced
- 🚨 LED: ON (solid)
- 🚨 Door: Open (90°)
- 🚨 Fan: ON (ventilation)
- ✅ Pump: OFF
- 📱 Blynk: "GAS CONCENTRATION HIGH!"

**Purpose**: Ventilate area to disperse gas

---

#### 🟠 Scenario 3: Fire Detected Only

**Conditions:**
- Gas < Threshold
- Fire Detected

**Actions:**
- 🚨 LCD: "WARNING! FIRE DETECTED"
- 🚨 Buzzer: Beeping - unless silenced
- 🚨 LED: ON
- 🚨 Door: Open
- ✅ Fan: OFF
- 🚨 Pump: ON (water spray)
- 📱 Blynk: "FIRE DETECTED!"

**Purpose**: Suppress fire with water

---

#### 🔴 Scenario 4: Gas AND Fire (Critical!)

**Conditions:**
- Gas > Threshold
- Fire Detected

**Actions:**
- 🚨 LCD: "WARNING! GAS & FIRE"
- 🚨 Buzzer: Continuous beeping - unless silenced
- 🚨 LED: ON
- 🚨 Door: Open
- 🚨 Fan: ON
- 🚨 Pump: ON
- 📱 Blynk: "WARNING: FIRE & GAS DETECTED!"

**Purpose**: Maximum response - ventilate AND suppress

---

### Manual Control

#### Physical Button - Smart Silence Feature 🔕

**Button Behavior** (Short Press):

```
1. ✅ Silences buzzer immediately
2. ✅ Keeps all safety systems active (fan, pump, door)
3. ✅ Sets "userSilencedBuzzer" flag
4. ✅ LCD shows "Buzzer Silenced by User" for 2 seconds
5. ✅ System continues monitoring
```

**Auto-Reactivation Logic:**

```
Scenario A: Alert persists (gas/fire still present)
  └── Buzzer remains OFF (respects user's silence)
  └── Safety systems stay active
  └── LCD continues showing warnings

Scenario B: Alert clears, then new alert detected
  └── Buzzer automatically REACTIVATES
  └── User must press button again to silence new alert
  └── Ensures user is aware of new hazard
```

**Usage Examples:**

1. **False Alarm (cooking smoke)**:
   - Press button → Buzzer stops
   - Wait for alert to clear
   - System resets automatically

2. **Real Emergency**:
   - Alert triggered → Buzzer sounds
   - Press button → Silence buzzer to think/communicate
   - Safety systems continue working
   - Leave area safely

3. **Multiple Alerts**:
   - Alert 1 → Press button → Silenced
   - Alert clears → System safe
   - Alert 2 (new) → Buzzer reactivates automatically
   - Must press button again to silence

---

#### Blynk App Control

**Gas Value (V0) - Read Only**
- Displays current gas concentration (0-10000 ppm)
- Updates every 2 seconds
- Use Chart widget to see trends

**Relay Control (V1) - Manual Override**
- Slider: 0 to 3
  - `0` = Both OFF
  - `1` = Fan ON, Pump OFF
  - `2` = Fan OFF, Pump ON
  - `3` = Both ON
- Useful for testing or manual ventilation

**Door Control (V2) - Emergency Override**
- Switch: ON/OFF
  - `OFF` = Door closed (0°)
  - `ON` = Door open (90°)
- Useful for testing servo or emergency access

**Gas Threshold (V3) - Sensitivity Adjustment**
- Slider: 200 to 10000 ppm
- Default: 2200 ppm
- Lower = More sensitive (earlier warnings)
- Higher = Less sensitive (fewer false alarms)
- Saved to EEPROM (persists after restart)

---

### FreeRTOS Tasks

The system runs 5 parallel tasks:

| Task | Core | Priority | Stack | Function |
|------|------|----------|-------|----------|
| **TaskWebServer** | 0 | 5 | 8192 | Handles HTTP requests (AP mode) |
| **TaskBlynk** | 0 | 5 | 8192 | Blynk communication & reconnection |
| **TaskMainDisplay** | 1 | 5 | 4096 | Sensor monitoring & alerts |
| **TaskBuzzer** | 1 | 5 | 2048 | Buzzer control with silence logic |
| **TaskButton** | 1 | 5 | 2048 | Button input handling (debounced) |

**Why FreeRTOS?**
- ✅ **Non-blocking**: All tasks run simultaneously
- ✅ **Responsive**: Button press detected instantly
- ✅ **Efficient**: ESP32 dual-core fully utilized
- ✅ **Stable**: Each task has its own stack
- ✅ **Task Suspension**: Can pause MainDisplay during config

---

## 🔍 How It Works

### Gas Detection (MQ-2 Sensor)

**Sensor Technology**: SnO2 semiconductor
- Detects: LPG, propane, methane, hydrogen, alcohol, smoke
- Preheat time: 24-48 hours (for accuracy)
- Operating voltage: 5V
- Output: Analog voltage (0-5V)

**Signal Processing Pipeline:**
```
1. analogRead(MQ2_SENSOR)           → Raw ADC value (0-4095)
   ↓
2. Kalman Filter                    → Noise reduction
   ↓
3. map(0-4095 → 0-10000)           → Convert to ppm scale
   ↓
4. Compare with threshold           → Decision
   ↓
5. Hysteresis check                 → Prevent flickering
   ↓
6. gasDetected = true/false         → Final state
```

**Hysteresis Logic:**
```cpp
If gas > threshold:
    gasDetected = TRUE
    
If gas < threshold - 100:
    gasDetected = FALSE
    
If threshold-100 < gas < threshold:
    gasDetected = (keep previous state)
```

**Why hysteresis?** Prevents rapid ON/OFF cycling when gas level hovers near threshold.

---

### Fire Detection (Infrared Flame Sensor)

**Sensor Technology**: IR photodiode
- Detects: Infrared radiation from flames (760-1100nm)
- Response time: < 1ms
- Detection range: 60-80cm (adjustable via potentiometer)
- Output: Digital (HIGH/LOW)

**Detection Logic:**
```cpp
fireDetected = (digitalRead(MH_SENSOR) == MH_SENSOR_ON);

// Where:
// MH_SENSOR_ON  = 0 (Active LOW - fire detected)
// MH_SENSOR_OFF = 1 (No fire)
```

---

### Smart Buzzer Control Logic

**State Machine:**
```cpp
bool buzzerActive = false;          // Current buzzer state
bool userSilencedBuzzer = false;   // User silence flag
bool lastAlertState = false;        // Previous alert state

// In handleAlerts():
bool currentAlertState = (gasDetected || fireDetected);

// NEW ALERT DETECTED
if (currentAlertState && !lastAlertState) {
    buzzerActive = true;              // Activate buzzer
    userSilencedBuzzer = false;       // Reset silence flag
}

// ALERT ACTIVE
if (currentAlertState) {
    if (!userSilencedBuzzer) {
        buzzerActive = true;           // Buzzer ON
    } else {
        buzzerActive = false;          // Buzzer OFF (silenced)
    }
    // Safety systems stay active regardless
}

// ALERT CLEARED
if (!currentAlertState) {
    buzzerActive = false;
    userSilencedBuzzer = false;        // Reset for next alert
}
```

**Key Features:**
- ✅ Button only affects buzzer, not safety systems
- ✅ Silence persists during same alert
- ✅ Auto-reactivates on new alert
- ✅ Safety systems always active during hazard

---

### WiFi Manager

**Access Point (AP) Mode:**
- Activates when: No credentials stored OR connection fails
- SSID: "ESP32"
- Password: None (open network)
- IP: 192.168.4.1
- Serves: Configuration web page

**Station (STA) Mode:**
- Activates when: Valid credentials in EEPROM
- Connects to: User's WiFi router
- IP: Assigned by DHCP (e.g., 192.168.1.100)
- Serves: Blynk connection

**Open WiFi Support:**
- Password field is optional
- Can leave empty for public/open networks
- JavaScript validation updated to allow empty password
- Backend accepts empty password string

---

### EEPROM Memory Map

ESP32 EEPROM (512 bytes) usage:

| Address | Size | Content | Default |
|---------|------|---------|---------|
| 0-31 | 32 bytes | WiFi SSID | Empty |
| 32-95 | 64 bytes | WiFi Password | Empty (open network support) |
| 96-127 | 32 bytes | Blynk Token | Empty |
| 128-201 | 74 bytes | *Reserved* | - |
| 202 | 1 byte | Threshold MSB (×100) | 22 |
| 203 | 1 byte | Threshold LSB | 0 |
| 204-511 | 308 bytes | *Free* | - |

**Example: Storing threshold 2200**
```
2200 ÷ 100 = 22 → Address 202
2200 % 100 = 0  → Address 203

Read back: (22 × 100) + 0 = 2200
```

---

## 📊 Performance Metrics

| Metric | Value | Notes |
|--------|-------|-------|
| **Sensor Read Rate** | 2 seconds | Configurable via `sensorCheckInterval` |
| **Blynk Update Rate** | 2 seconds | Via `timer.setInterval(2000L, Timer)` |
| **Button Response Time** | <50ms | Hardware debounce + 50ms software |
| **WiFi Reconnect** | 30 seconds | Auto-retry every 30s if disconnected |
| **Memory Usage** | ~45% | ~135KB RAM used of 320KB |
| **Flash Usage** | ~60% | ~800KB of 1.3MB partition |
| **Power Consumption** | ~250mA | ESP32 + sensors (without relays) |
| **Boot Time** | ~5 seconds | From power-on to WiFi connected |
| **Warmup Time** | 60 seconds | Sensor stabilization countdown |

---

## 🔐 Security Considerations

### WiFi Security
- ✅ Supports WPA/WPA2 encryption
- ⚠️ AP mode is OPEN by default (no password)
- 💡 Consider adding AP password in production
- 🔒 Credentials stored in EEPROM (not encrypted)

### Blynk Security
- ✅ Token-based authentication
- ✅ HTTPS communication with Blynk cloud
- ⚠️ Token visible in Serial Monitor during debug
- 💡 Disable Serial output in production

---

### Community Contributions
Pull requests welcome! Areas for contribution:
- Additional sensor support (CO, CO2, smoke)
- Alternative IoT platforms (ThingSpeak, Home Assistant)
- Mobile app (React Native/Flutter)
- PCB design for production
- 3D printable enclosure designs

---

## 📄 License

This project is licensed under the **MIT License**.

```
MIT License

Copyright (c) 2024 [Your Name]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

---

## 🙏 Acknowledgments

- **Blynk** - For excellent IoT platform
- **ESP32 Community** - For comprehensive documentation
- **Arduino Community** - For extensive library support
- **FreeRTOS** - For real-time operating system
- **All contributors** - For improvements and bug reports

---

## 📞 Support & Contact

### Get Help
- 📖 **Documentation**: Read this README thoroughly
- 🐛 **Bug Reports**: [GitHub Issues](https://github.com/coldbrewtonic22/Gas-and-Fire-Detection-System/issues)
- 💬 **Discussions**: [GitHub Discussions](https://github.com/coldbrewtonic22/Gas-and-Fire-Detection-System/discussions)
- 📧 **Email**: vmquan.dev@gmail.com

---

## ⚠️ Safety Disclaimer

**IMPORTANT**: This project is for educational and experimental purposes. 

- ⚠️ **NOT certified** for commercial safety applications
- ⚠️ **NOT a replacement** for professional fire/gas detection systems
- ⚠️ **Use at your own risk** - creator assumes no liability
- ⚠️ **Always have backup** safety measures (smoke alarms, CO detectors)
- ⚠️ **Regular maintenance** required - test sensors monthly
- ⚠️ **Follow local codes** and regulations for safety equipment

**For life-critical applications, use certified commercial systems.**

---

## 👨‍💻 Author

**[Quan Vu]**
- GitHub: [@coldbrewtonic22](https://github.com/coldbrewonic22)
- Email: vmquan.dev@gmail.com

---

## 🙏 Acknowledgments

- Blynk Team
- ESP32 Community
- FreeRTOS Project
- Arduino Community
- All open-source library contributors

---

## 📞 Support

If you encounter any issues or have questions:
1. Check [Troubleshooting](#troubleshooting) section
2. Open an issue on [GitHub](https://github.com/coldbrewtonic22/gas-fire-detection-system/issues)
3. Contact via email: vmquan.dev@gmail.com

---

**⚠️ Safety Warning**: This system is designed for educational purposes. For critical safety applications, use certified commercial systems and consult with safety professionals.