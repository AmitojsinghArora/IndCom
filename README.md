# Industrial Communication Projects

This repository contains three projects developed as part of my Industrial Communication course. Each project demonstrates a different communication protocol using embedded systems and industrial sensors.

## Projects Overview

### 1. Modbus RTU & TCP via HTTP Server with ESP32

**Description**  
This project involves reading temperature and humidity values from a Modbus sensor using an ESP32. The values are served via an HTTP server and support both Modbus RTU and TCP protocols.

**Key Components**
- ESP32
- Modbus Temperature & Humidity Sensor
- HTTP Server on ESP32

**Features**
- Read sensor data over Modbus RTU 
- Serve readings over a local HTTP server and TCP server
- Observe measured data over NodeRed dashboard

**Folder**: `src/Modbus_TCP`

---

### 2. CAN Bus Protocol with CR1301 Joystick and LEDs

**Description**  
This project implements a CAN bus communication system using an MCP2515 and ESP32. It detects joystick button presses and LED color changes based on the joystick’s direction.

**Key Components**
- ESP32
- CR1301 Joystick
- MCP2515 CAN Module
- RGB LEDs

**Features**
- Detect rotary direction (clockwise/anticlockwise)
- Change LED colors based on a color wheel when in `DETECT_DIRECTION` state
- Communication handled via CAN bus

**Folder**: `src/CAN_Joystick/`

---

### 3. IO-Link Communication with IFM AL1330 and Arduino Shield

**Description**  
This project demonstrates IO-Link communication between an IFM AL1330 master and a joystick sensor. The joystick data is processed through an Arduino IO-Link shield and visualized on Node-RED.

**Key Components**
- IFM AL1330 IO-Link Master
- Arduino with IO-Link Shield
- Joystick Sensor
- Node-RED Dashboard

**Features**
- Joystick data acquisition via IO-Link
- Real-time visualization in Node-RED

**Folder**: `iolink_nodered/`

---

## 🛠️ Installation and Setup

Each project folder contains its own code and dependencies. To run any of the projects:

1. Clone the repository:
   ```bash
   git clone https://github.com/AmitojsinghArora/IndCom.git
   cd src
   ```
   Enter the folder Modbus_TCP or CAN_Joystick or IOlink to run the respective protocols
