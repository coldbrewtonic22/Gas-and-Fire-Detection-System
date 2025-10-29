# 🔥 Gas & Fire Detection System

![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)
![Platform](https://img.shields.io/badge/platform-ESP32-green.svg)

An intelligent IoT-based Gas and Fire Detection System built with ESP32, featuring real-time monitoring, automated response, and remote control via Blynk IoT platform.

## 📋 Table of Contents

- [Features](#features)
- [System Architecture](#system-architecture)
- [Hardware Requirements](#hardware-requirements)
- [Pin Configuration](#pin-configuration)
- [Software Requirements](#software-requirements)
- [Installation](#installation)
- [Configuration](#configuration)
- [Usage](#usage)
- [System Workflow](#system-workflow)
- [Blynk Dashboard](#blynk-dashboard)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)

---

## ✨ Features

### 🔍 Detection & Monitoring
- **Gas Detection**: MQ2 sensor with Kalman filter for noise reduction
- **Fire Detection**: Infrared flame sensor (MH sensor)
- **Real-time Monitoring**: Continuous sensor data reading every 2 seconds
- **LCD Display**: 16x2 I2C LCD showing system status and gas levels

### 🚨 Automated Response
- **Multi-level Alerts**:
  - Gas only: Activates ventilation fan + opens door
  - Fire only: Activates water pump + opens door
  - Gas & Fire: Activates both fan and pump + opens door
- **Visual Alerts**: LED indicator
- **Audio Alerts**: Non-blocking buzzer pattern
- **Smart Door Control**: Servo-controlled emergency exit

### 🌐 IoT & Remote Control
- **WiFi Connectivity**: Auto-reconnection on connection loss
- **Blynk Integration**: Remote monitoring and control
- **Web Configuration**: User-friendly web interface for WiFi setup
- **Push Notifications**: Real-time alerts via Blynk app
- **Remote Control**: Adjust threshold, control relays and door remotely

### 🎛️ Advanced Features
- **FreeRTOS Multitasking**: Dual-core task management for responsive performance
- **Adjustable Threshold**: Customize gas detection sensitivity (200-9999 ppm)
- **EEPROM Storage**: Persistent configuration across reboots
- **Manual Override**: Emergency stop button
- **60-Seconds Warmup**: Sensor stabilization period

---

## 🏗️ System Architecture

### FreeRTOS Task Structure

The system uses **5 concurrent tasks** running on ESP32's dual-core architecture:

| Task | Core | Priority | Stack | Frequency | Description |
|------|------|----------|-------|-----------|-------------|
| **TaskWebServer** | 0 | 5 | 8KB | 10ms | HTTP server for configuration |
| **TaskBlynk** | 0 | 5 | 8KB | 100ms | Blynk communication & reconnection |
| **TaskMainDisplay** | 1 | 5 | 4KB | 2000ms | Sensor reading & alert logic |
| **TaskBuzzer** | 1 | 5 | 2KB | 10ms | Non-blocking buzzer control |
| **TaskButton** | 1 | 5 | 2KB | 10ms | Button debouncing & handling |

### System States

```
┌──────────────────────────────────────┐
│        SYSTEM INITIALIZATION         │
│  - WiFi Connection                   │
│  - Blynk Authentication              │
│  - 60-Second Sensor Warmup           │
└─────────────────┬────────────────────┘
                  │
                  ▼
┌──────────────────────────────────────┐
│          NORMAL OPERATION            │
│  - Read sensors every 2s             │
│  - Display gas level on LCD          │
│  - Monitor for threshold breach      │
└─────────────────┬────────────────────┘
                  │
                  ▼
        ┌─────────┴──────────┐
        │     Threshold      │
        │     Exceeded?      │
        └─────────┬──────────┘
                  │
        ┌─────────┴─────────┐
        │                   │
       NO                  YES
        │                   │
        │                   ▼
        │         ┌─────────────────────┐
        │         │   ALERT MODE        │
        │         │ - Activate buzzer   │
        │         │ - Turn on LED       │
        │         │ - Open door         │
        │         │ - Control relays    │
        │         │ - Send notification │
        │         └─────────┬───────────┘
        │                   │
        │                   ▼
        │         ┌─────────────────────┐
        │         │   Button Pressed?   │
        │         └─────────┬───────────┘
        │                   │
        │                  YES
        │                   │
        └───────────────────┴──────────►
                                       │
                                       ▼
                            ┌──────────────────┐
                            │  RESET TO SAFE   │
                            │  - Stop buzzer   │
                            │  - Turn off LED  │
                            │  - Close door    │
                            │  - Reset flags   │
                            └──────────────────┘
```

---

## 🛠️ Hardware Requirements

### Main Components

| Component | Specification | Quantity | Notes |
|-----------|--------------|----------|-------|
| **Microcontroller** | [ESP32](https://banlinhkien.com/kit-wifi-esp32-espwroom32s-p6649289.html) | 1 | Any ESP32 board with WiFi |
| **Gas Sensor** | [MQ2 Sensor](https://banlinhkien.com/module-cam-bien-khi-gas-mq2-p6646888.html) | 1 | Detects LPG, propane, methane, hydrogen |
| **Fire Sensor** | [Flame Sensor](https://banlinhkien.com/module-cam-bien-phat-hien-lua-flame-sensor-p6646877.html) | 1 | IR-based flame detection |
| **Display** | [LCD I2C 1602](https://www.linhkienx.com/man-hinh-lcd-1602-nen-xanh-la-chu-den-5vdc-kem-i2c-driver) | 1 | With I2C driver |
| **Servo Motor** | [SG90](https://banlinhkien.com/dong-co-servo-sg90-goc-quay-180-p6648774.html) | 1 | 180° rotation for door control |
| **Relay Module** | [Relay 2 channel 5V](https://banlinhkien.com/module-relay-mini-2-kenh-5v10a-blk-p17935548.html) | 1 | For fan and pump control |
| **Buzzer** | [Buzzer](https://www.linhkienx.com/1209-buzzer-coi-chip-12x9mm-93db-xuyen-lo) | 1 | Alert sound generation |
| **LED** | [LED Red 5mm](https://www.linhkienx.com/led-do-3mm-sieu-sang-dau-tron-trong-suot-chan-dai) | 1 | Visual alert indicator |
| **Push Button** | [Button](https://www.linhkienx.com/nut-nhan-6x6mm-cao-5mm-4-chan-xuyen-lo) | 1 | Emergency stop/reset |
| **Resistor** | [220Ω](https://www.linhkienx.com/dien-tro-220-ohm-1-4w-5-4-vong-mau) | 1 | For LED current limiting |

### Demo Components (Not for Production)
- Small DC Fan (controlled by Relay 1) - represents ventilation system
- Small Water Pump (controlled by Relay 2) - represents fire suppression
- Cardboard/3D printed door mechanism
- Breadboard or PCB for connections

### Optional Components
- Enclosure/Housing for demo presentation
- Jumper wires
- Perfboard for permanent connections

---

## 📌 Pin Configuration

### ESP32 Pin Mapping

```cpp
// Sensors
#define MQ2_SENSOR    35    // Gas sensor (Analog)
#define MH_SENSOR     34    // Fire sensor (Digital)

// Outputs
#define BUZZER        23    // Alert buzzer
#define LED           19    // Status LED
#define SERVO         33    // Door servo motor
#define RELAY_FAN     18    // Relay 1 - Ventilation fan
#define RELAY_PUMP     5    // Relay 2 - Water pump

// Input
#define BUTTON         4    // Emergency stop button

// I2C (LCD) - Default ESP32 I2C pins
// SDA: GPIO 21
// SCL: GPIO 22
```

### Wiring Diagram

```
ESP32                          Components
─────────────────────────────────────────
GPIO 35 ────────────────────► MQ2 Sensor (AO)
GPIO 34 ────────────────────► MH Sensor (DO)
GPIO 23 ────────────────────► Buzzer (+)
GPIO 19 ──┬─ 220Ω ──────────► LED (+)
          └─────────────────► GND
GPIO 33 ────────────────────► Servo (Signal)
GPIO 18 ────────────────────► Relay 1 (IN1)
GPIO 5  ────────────────────► Relay 2 (IN2)
GPIO 4  ────────────────────► Button
GPIO 21 ────────────────────► LCD (SDA)
GPIO 22 ────────────────────► LCD (SCL)
5V  ────────────────────────► VCC (All components)
GND ────────────────────────► GND (All components)
```

---

## 💻 Software Requirements

### Development Environment
- **PlatformIO** (recommended) or Arduino IDE
- **ESP32 Board Package** v2.0.0 or higher

### Required Libraries

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps = 
    blynkkk/Blynk@^1.3.2
    marcoschwartz/LiquidCrystal_I2C@^1.1.4
    madhephaestus/ESP32Servo@^0.13.0
    denyssene/SimpleKalmanFilter@^0.1.0
```

### Library Versions
- **Blynk**: ^1.3.2
- **LiquidCrystal_I2C**: ^1.1.4
- **ESP32Servo**: ^0.13.0
- **SimpleKalmanFilter**: ^0.1.0

---

## 📁 Project Structure

```
├── 📁 include
│   └── 📄 README                 # Header files documentation
├── 📁 lib
│   └── 📄 README                 # Custom libraries directory
├── 📁 src
│   ├── 📝 README.md              # Source code documentation
│   ├── ⚡ config.h                   # WiFi & Blynk configuration 
│   ├── ⚡ def.h                      # Pin definitions & constants
│   └── ⚡ main.cpp                   # Main program
├── 📁 test
│   └── 📄 README                 # Unit tests directory
├── ⚙️ .gitignore                 # Git ignore rules
└── ⚙️ platformio.ini             # Platform IO initialization
```

### File Description

| File | Purpose |
|------|--------|
| config.h | WiFi credentials, Blynk tokens, EEPROM strings |
| def.h | Hardware pin mappings, constants, threshold values |
| main.cpp | Main program with 5 FreeRTOS tasks implementation |
| platformio.ini | Build configuration, board settings, library dependencies |

## 🚀 Installation

### 1. Clone Repository

```bash
git clone https://github.com/yourusername/gas-fire-detection-system.git
cd gas-fire-detection-system
```

### 2. Install PlatformIO

**VS Code Extension:**
```bash
# Install VS Code
# Install PlatformIO IDE extension from marketplace
```

**Or CLI:**
```bash
pip install platformio
```

### 3. Build & Upload

```bash
# Open project in PlatformIO
pio run

# Upload to ESP32
pio run --target upload

# Monitor serial output
pio device monitor --baud 9600
```

---

## ⚙️ Configuration

### Initial Setup

1. **Power on the ESP32**
   - System will enter Access Point (AP) mode if no WiFi credentials found
   - LCD displays: `No WiFi Config` / `Setup Required`

2. **Connect to ESP32 WiFi**
   - Network Name: `ESP32`
   - Password: None (Open network)

3. **Open Configuration Page**
   - Browser: `http://192.168.4.1`
   - Fill in:
     - WiFi SSID
     - WiFi Password
     - Blynk Authentication Token

4. **Save Configuration**
   - ESP32 will restart and connect to your WiFi
   - Wait 60 seconds for sensor warmup

### Blynk Setup

1. Download **Blynk IoT** app (iOS/Android) or use web console
2. Create a new project (Free plan)
3. Create a **Template** with:
   - Device: ESP32
   - Connection Type: WiFi
4. Get **Template ID** and **Auth Token**
5. Set up **Datastreams**:
   - V0: Gas Level (Integer, 0-10000, Read only)
   - V1: Relay Control (Integer, 0-3, Read/Write)
   - V2: Door Control (Integer, 0-1, Read/Write)
   - V3: Gas Threshold (Integer, 200-9999, Read/Write)
6. Create **Event** for notifications:
   - Event name: `gas_fire_detection`
   - Enable notifications
7. Build your dashboard with widgets (see Blynk Dashboard section)

### Adjusting Gas Threshold

**Method 1: Blynk App**
- Use slider widget connected to Virtual Pin V3
- Range: 200 - 9999 ppm
- Default: 2200 ppm

**Method 2: Via Code**
```cpp
#define GAS_THRESHOLD 2200  // Modify in def.h
```

---

## 📱 Blynk Dashboard

### Datastream Configuration

Create these datastreams in Blynk Template:

| Virtual Pin | Name | Data Type | Min | Max | Default | Mode |
|------------|------|-----------|-----|-----|---------|------|
| **V0** | Gas Level | Integer | 200 | 10000 | 0 | Read |
| **V1** | Relay Control | Integer | 0 | 3 | 0 | Read/Write |
| **V2** | Door Control | Integer | 0 | 1 | 0 | Read/Write |
| **V3** | Gas Threshold | Integer | 200 | 9999 | 2200 | Read/Write |

### Dashboard Widgets (Free Plan)

#### V0 - Gas Level Monitoring
**Widget 1: Label**
```
Title: Gas Level
Datastream: V0
Unit: ppm
Refresh: 2 seconds
```

**Widget 2: SuperChart**
```
Title: Gas History
Datastream: V0
Time Range: 1 hour / 6 hours / 1 day
Update Interval: 2 seconds
Chart Type: Line
Color: Green (< threshold), Red (> threshold)
```

#### V1 - Relay Control
**Widget: Segmented Switch**
```
Title: System Control
Datastream: V1
Options:
  0: "All OFF"
  1: "Fan ON"
  2: "Pump ON"
  3: "Both ON"
Color: Blue
```

#### V2 - Door Control
**Widget: Switch**
```
Title: Emergency Door
Datastream: V2
Mode: Switch
Labels:
  OFF (0): "Closed 🔒"
  ON (1): "Open 🚪"
Color: Orange
```

#### V3 - Threshold Adjustment
**Widget: Slider**
```
Title: Gas Threshold
Datastream: V3
Range: 200 - 9999
Step: 50
Unit: ppm
Send on Release: Yes
Color: Red
```

### Event Configuration

**Event Name**: `gas_fire_detection`

**Notification Templates**:
```
Gas Only:
  Title: ⚠️ GAS ALERT
  Body: High gas concentration detected! ({{V0}} ppm)

Fire Only:
  Title: 🔥 FIRE ALERT
  Body: Fire detected! Emergency response activated.

Both:
  Title: 🚨 CRITICAL ALERT
  Body: GAS & FIRE detected! All systems activated!
```

**Setup Steps**:
1. Go to Template → Events → Create Event
2. Name: `gas_fire_detection`
3. Enable "Push Notification"
4. Test notification from dashboard

---

## 🎮 Usage

### Normal Operation

1. **System Startup**
   ```
   Display: "Gas and Fire"
            "Detection System"
   Wait: 3 seconds
   ```

2. **Sensor Warmup**
   ```
   Display: "Warming Up"
            "Sensors..."
   Then:    "Wait: 60 (s)"
   Countdown to 0
   ```

3. **Ready State**
   ```
   Display: "System running"
            "Gas: XXXX ppm"
   ```

### Alert Response

#### Gas Detection (Only)
```
Trigger: Gas > Threshold
Actions:
  ✓ Buzzer: ON (beep pattern)
  ✓ LED: ON
  ✓ Relay 1 (Fan): ON
  ✓ Relay 2 (Pump): OFF
  ✓ Door: OPEN
  ✓ LCD: "WARNING" / "GAS DETECTED"
  ✓ Blynk: Push notification
```

#### Fire Detection (Only)
```
Trigger: Flame sensor activated
Actions:
  ✓ Buzzer: ON (beep pattern)
  ✓ LED: ON
  ✓ Relay 1 (Fan): OFF
  ✓ Relay 2 (Pump): ON
  ✓ Door: OPEN
  ✓ LCD: "WARNING" / "FIRE DETECTED"
  ✓ Blynk: Push notification
```

#### Gas & Fire Detection (Both)
```
Trigger: Gas > Threshold AND Flame detected
Actions:
  ✓ Buzzer: ON (beep pattern)
  ✓ LED: ON
  ✓ Relay 1 (Fan): ON
  ✓ Relay 2 (Pump): ON
  ✓ Door: OPEN
  ✓ LCD: "WARNING!" / "GAS & FIRE"
  ✓ Blynk: Critical push notification
```

### Manual Override

**Emergency Stop Button:**
- Press the button to:
  - Stop buzzer
  - Turn off LED
  - Close door
  - Stop all relays
  - Reset alert flags
  - Display: `Alert Stopped by User`

---

## 🔄 System Workflow

### Detection Logic

```cpp
// Hysteresis for gas detection (prevents flickering)
if (gasValue > threshold)
    gasDetected = true;
else if (gasValue < threshold - 100)
    gasDetected = false;

// Fire detection (digital signal)
fireDetected = (fireValue == LOW);  // Active LOW sensor
```

### Alert Priority

1. **Both Gas & Fire**: Highest priority - activate all safety measures
2. **Gas Only**: Medium priority - ventilation only
3. **Fire Only**: Medium priority - suppression only
4. **Safe**: Normal operation - all safety measures off

### Data Flow

```
Sensors → Kalman Filter → Threshold Check → Alert Logic → Actuators
                                                             ↓
                        Blynk Cloud ◄────────────────────────┘
                             ↓
                        Mobile App
```

---

## 🐛 Troubleshooting

### WiFi Connection Issues

**Problem**: Cannot connect to WiFi
```
Solutions:
1. Check SSID and password are correct
2. Ensure WiFi is 2.4GHz (ESP32 doesn't support 5GHz)
3. Move ESP32 closer to router
4. Check serial monitor for connection status
5. Reset configuration: Connect to ESP32 AP and reconfigure
```

**Problem**: WiFi keeps disconnecting
```
Solutions:
1. Check router signal strength
2. Disable router power saving mode
3. Update ESP32 WiFi library
4. Check for interference from other devices
```

### Blynk Connection Issues

**Problem**: "Disconnect Blynk" on LCD
```
Solutions:
1. Verify Blynk token is exactly 32 characters
2. Check internet connection
3. Verify Blynk template ID matches
4. Check Blynk server status
5. Try creating a new Blynk project
```

### Sensor Issues

**Problem**: Gas sensor shows unstable readings
```
Solutions:
1. Wait for 60-second warmup period
2. MQ2 sensors need 24-48 hours for first-time calibration
3. Ensure proper power supply (5V stable)
4. Check wiring connections
5. Increase Kalman filter parameters if needed
```

**Problem**: Fire sensor too sensitive / not sensitive enough
```
Solutions:
1. Adjust sensor potentiometer (if available)
2. Check sensor viewing angle
3. Test with actual flame (safely!)
4. Verify sensor is not facing direct sunlight
```

### Hardware Issues

**Problem**: LCD not displaying
```
Solutions:
1. Check I2C address (default: 0x27)
   Run I2C scanner sketch to find address
2. Verify SDA (GPIO 21) and SCL (GPIO 22) connections
3. Check LCD contrast potentiometer
4. Verify 5V power supply
```

**Problem**: Servo not moving
```
Solutions:
1. Check servo power supply (5V, sufficient current)
2. Verify GPIO 33 connection
3. Test servo independently
4. Check if servo is mechanically stuck
```

**Problem**: Relays not activating
```
Solutions:
1. Verify relay is 5V type (not 12V)
2. Check GPIO 18 and GPIO 5 connections
3. Ensure relay module has separate power if needed
4. Test relay with direct 5V signal
```

### Software Issues

**Problem**: System freezes / crashes
```
Solutions:
1. Check serial monitor for error messages
2. Verify all tasks are not blocked
3. Increase stack size of tasks if needed
4. Check for memory leaks
5. Disable tasks one by one to isolate issue
```

**Problem**: Compilation errors
```
Solutions:
1. Update PlatformIO core
2. Clean build files: pio run --target clean
3. Check library versions match
4. Verify ESP32 board package version
```

### Configuration Issues

**Problem**: Cannot access 192.168.4.1
```
Solutions:
1. Verify connected to "ESP32" WiFi network
2. Try http://192.168.4.1 (not https)
3. Clear browser cache
4. Try different browser
5. Check ESP32 serial monitor for AP status
```

---

## 📊 Performance Metrics

### Response Times
- **Gas Detection**: < 2 seconds
- **Fire Detection**: < 2 seconds
- **Button Response**: < 50ms
- **Blynk Update**: 2 seconds interval
- **Web Server Response**: < 100ms

### Resource Usage
- **Flash**: ~1.2 MB / 4 MB
- **RAM**: ~180 KB / 320 KB
- **CPU**: ~40% average load

### Power Consumption
- **Idle**: ~150 mA
- **Active (no alert)**: ~200 mA
- **Alert mode**: ~800 mA (with all actuators)

---

## 🔐 Security Considerations

1. **WiFi**: Use WPA2 encryption, strong password
2. **Blynk**: Keep auth token private, never commit to public repos
3. **Web Interface**: Consider adding authentication for production
4. **OTA Updates**: Implement secure OTA for remote updates
5. **Data Privacy**: Gas level data is only sent to Blynk cloud

---

## 🚧 Future Enhancements

- [ ] **Data Logging**: SD card storage for historical data
- [ ] **SMS Alerts**: GSM module integration
- [ ] **Email Notifications**: SMTP integration
- [ ] **Machine Learning**: Predictive leak detection
- [ ] **Multi-sensor Support**: Multiple gas sensors
- [ ] **Battery Backup**: UPS integration
- [ ] **Web Dashboard**: ESP32 web server dashboard
- [ ] **Voice Alerts**: Audio message playback
- [ ] **Camera Integration**: ESP32-CAM for visual confirmation

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