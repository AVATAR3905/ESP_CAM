# ESP32-CAM Streaming System with Enclosure

A compact ESP32-CAM-based video streaming system integrated into a protective enclosure for real-world deployment.

---

## Project Overview

This project demonstrates how to build a low-cost surveillance system using the ESP32-CAM module, combined with a 3D printable enclosure adapted to house and protect the hardware.

---

## Enclosure Details

Based on your model:

🔹 Front Panel
Circular cutout for camera lens
Secondary hole for mounting / sensor access
Clean minimal design for unobstructed vision
🔹 Main Housing
Snug fit for ESP32-CAM board
Internal guiding rails for placement
Compact rectangular geometry
🔹 Ventilation System
Angled slotted vents (as seen in your design)
Improves airflow → prevents overheating
🔹 Bottom Cutout
Access for:
Power cable
Programming pins

### 🔹 Features Visible in Design

* Circular opening for camera lens
* Additional side hole for mounting / access
* Ventilation slots for airflow
* Bottom opening for cable routing

---

## Features

* 📡 Live video streaming over WiFi
* 🌐 Web-based camera interface
* 📦 Compact and portable setup
* 🧱 Protective housing for hardware

---

## Hardware Required

* ESP32-CAM (AI Thinker)
* FTDI Programmer
* 5V Power Supply
* Jumper wires

---

## Setup

### Wiring

| FTDI | ESP32-CAM        |
| ---- | ---------------- |
| 5V   | 5V               |
| GND  | GND              |
| TX   | U0R              |
| RX   | U0T              |
| IO0  | GND (for upload) |

---

### Upload Steps

1. Connect IO0 → GND
2. Upload code using Arduino IDE
3. Remove IO0 → GND
4. Reset the board

---

### WiFi Configuration

Edit in `esp_cam_imp.ino`:

```cpp
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
```

---

## 📂 Repository Structure

```id="9k3w5z"
ESP32-CAM-System/
│
├── code/
│   └── esp_cam_imp.ino
│
├── case/
│   ├── enclosure.stl
│   ├── case_front.png
│   ├── case_inside.png
│
└── README.md
```

---

## Applications

* Home surveillance
* Robotics vision
* Embedded camera systems
* IoT monitoring

---

## Future Work

* Motion detection integration
* Cloud-based video streaming
* Outdoor/weatherproof casing

---

## 📜 License

MIT License
