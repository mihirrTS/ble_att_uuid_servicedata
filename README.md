# 🎯 BLE-Based Attendance System

A complete IoT attendance tracking system using ESP32 devices to detect BLE beacons and log presence events to a backend server.

## 📋 Table of Contents

- [Overview](#overview)
- [System Architecture](#system-architecture)
- [Files Structure](#files-structure)
- [Prerequisites](#prerequisites)
- [Quick Start Guide](#quick-start-guide)
- [Detailed File Documentation](#detailed-file-documentation)
- [How It Works](#how-it-works)
- [Configuration](#configuration)
- [API Reference](#api-reference)
- [Troubleshooting](#troubleshooting)
- [Future Enhancements](#future-enhancements)

---

## 🎯 Overview

This system automatically tracks employee attendance using:
- **Mobile devices** broadcasting BLE beacons (employee tokens)
- **ESP32 devices** scanning for BLE beacons and detecting presence
- **Backend server** receiving, processing, and storing attendance logs
- **Web dashboard** for real-time monitoring and reporting

**Key Features:**
- ✅ Automatic check-in/check-out (no manual action required)
- ✅ Real-time attendance tracking
- ✅ Visual dashboard with live updates
- ✅ Support for multiple employees and ESP devices
- ✅ Timeout-based absence detection
- ✅ RESTful API for integration

---

## 🏗️ System Architecture

```
┌─────────────────┐         ┌──────────────────┐         ┌─────────────────┐
│  Mobile Device  │         │   ESP32 Device   │         │  Backend Server │
│  (BLE Beacon)   │────────>│  (BLE Scanner)   │────────>│   (Node.js)     │
│                 │  BLE    │                  │  HTTP   │                 │
│  Broadcasts:    │ Signal  │  Detects:        │  POST   │  Processes:     │
│  - Token ID     │         │  - Service UUID  │         │  - Check-in     │
│  - Service UUID │         │  - Token Data    │         │  - Check-out    │
└─────────────────┘         │  - Timestamp     │         │  - Status       │
                            └──────────────────┘         └─────────────────┘
                                                                   │
                                                                   │ HTTP
                                                                   ▼
                                                          ┌─────────────────┐
                                                          │  Web Dashboard  │
                                                          │   (Browser)     │
                                                          │                 │
                                                          │  Shows:         │
                                                          │  - Live logs    │
                                                          │  - Statistics   │
                                                          │  - Device status│
                                                          └─────────────────┘
```

---

## 📁 Files Structure

```
test/
├── README.md                           # This file - comprehensive documentation
├── BRD - Wireless Attendance .txt      # Business Requirements Document
│
├── Arduino Files (.ino)
│   ├── test.ino                        # Original code (Google Sheets integration)
│   ├── server_att.ino                  # Production code (Server integration)
│   └── mock_esp32_test.ino            # Mock/Test code (No BLE, for testing)
│
└── server/                             # Backend server directory
    ├── server.js                       # Main Node.js server application
    ├── package.json                    # Node.js dependencies
    ├── README.md                       # Server-specific documentation
    └── public/
        └── index.html                  # Web dashboard (frontend)
```

---

## 🔧 Prerequisites

### Hardware
- **ESP32 Development Board** (NodeMCU-32S, ESP32-WROOM, or similar)
- **USB Cable** (for programming ESP32)
- **Computer** (Windows, Mac, or Linux)

### Software
- **Arduino IDE** (v1.8.19 or higher) or **PlatformIO**
- **Node.js** (v14 or higher) and **npm**
- **Web Browser** (Chrome, Firefox, Edge, or Safari)

### Arduino Libraries (Install via Library Manager)
- **WiFi** (built-in with ESP32 board)
- **HTTPClient** (built-in with ESP32 board)
- **BLEDevice** (built-in with ESP32 board)
- **ArduinoJson** (v6.21.0 or higher) - **Must install manually!**

### Network Requirements
- WiFi network (2.4GHz - ESP32 compatible)
- ESP32 and server must be on the same network

---

## 🚀 Quick Start Guide

### Step 1: Install Arduino Libraries

1. Open Arduino IDE
2. Go to **Sketch → Include Library → Manage Libraries**
3. Search for "**ArduinoJson**"
4. Install version **6.21.0** or higher by Benoit Blanchon

### Step 2: Setup Backend Server

```powershell
# Navigate to server directory
cd server

# Install dependencies
npm install

# Start the server
npm start
```

Server will start at `http://localhost:3000`

### Step 3: Configure ESP32 Code

1. Open `server_att.ino` (for production) or `mock_esp32_test.ino` (for testing)
2. Update WiFi credentials:
   ```cpp
   const char* WIFI_SSID = "Your_WiFi_Name";
   const char* WIFI_PASS = "Your_WiFi_Password";
   ```
3. Find your computer's IP address:
   ```powershell
   ipconfig
   ```
   Look for "IPv4 Address" (e.g., 192.168.1.102)

4. Update server URL:
   ```cpp
   const char* SERVER_URL = "http://192.168.1.102:3000/api/presence";
   ```

### Step 4: Upload to ESP32

1. Connect ESP32 to computer via USB
2. Select **Tools → Board → ESP32 Dev Module**
3. Select correct **Port** (COM3, COM4, etc.)
4. Click **Upload** button
5. Open **Serial Monitor** (115200 baud)

### Step 5: View Dashboard

Open browser and go to: `http://localhost:3000`

You should see:
- Real-time logs of check-ins/check-outs
- Device status (present/absent)
- Statistics (total events, active devices)

---

## 📄 Detailed File Documentation

### 1. **test.ino** (Original - Google Sheets)

**Purpose:** Original proof-of-concept that logs attendance to Google Sheets

**Key Features:**
- Scans for BLE devices with Service UUID `0xD7E1A3F4`
- Detects walk-in when token first appears
- Detects walk-out after 15 seconds of absence
- Sends data to Google Apps Script via HTTP GET

**Use Case:** 
- Quick prototype without backend infrastructure
- Direct integration with Google Sheets for simple reporting

**Configuration:**
```cpp
const char* WIFI_SSID = "Your_WiFi";
const char* WIFI_PASS = "Your_Password";
const char* GOOGLE_SCRIPT_URL = "your_google_script_url";
```

---

### 2. **server_att.ino** (Production - Server Integration)

**Purpose:** Production-ready code that sends attendance data to backend server

**Key Features:**
- BLE scanning with Service UUID `0xD7E1A3F4` (32-bit)
- JSON payload formatting using ArduinoJson library
- ESP device identification for multi-device support
- Robust error handling and retry logic
- Maintains device presence state with timeout detection

**How It Works:**

1. **BLE Scanning:**
   ```cpp
   - Scans every 5 seconds for BLE advertisements
   - Filters by Service UUID (0xD7E1A3F4)
   - Extracts readable token from service data
   ```

2. **Presence Detection:**
   ```cpp
   - First detection → Walk-IN event
   - Continuous detection → Updates last seen timestamp
   - No detection for 15+ seconds → Walk-OUT event
   ```

3. **Data Transmission:**
   ```cpp
   - Creates JSON payload: {token, esp_id, action, timestamp}
   - Sends HTTP POST to server endpoint
   - Logs response/errors to Serial Monitor
   ```

**Configuration:**
```cpp
const char* WIFI_SSID = "Zoo_Studio_2.4";      // Your WiFi name
const char* WIFI_PASS = "Trh@1234";            // Your WiFi password
const char* SERVER_URL = "http://192.168.1.102:3000/api/presence";
const char* ESP_ID = "ESP32_001";              // Unique ID per device
```

**Serial Monitor Output Example:**
```
===== ESP32 BLE Attendance (Server Integration) =====
ESP Device ID: ESP32_001
✅ Wi-Fi connected! IP: 192.168.1.123
✅ BLE ready

🔍 Scanning...
👣 Walk-IN: EMP001 [aa:bb:cc:dd:ee:ff]
✅ Logged EMP001 (IN): 201
   Response: {"success":true,"message":"Presence event logged"}

🔍 Scanning...
🚪 Walk-OUT: EMP001 (timeout 15234 ms)
✅ Logged EMP001 (OUT): 201
```

**Dependencies:**
```cpp
#include <WiFi.h>          // ESP32 WiFi
#include <HTTPClient.h>    // HTTP requests
#include <BLEDevice.h>     // BLE functionality
#include <BLEUtils.h>      // BLE utilities
#include <BLEScan.h>       // BLE scanning
#include <ArduinoJson.h>   // JSON formatting
#include <map>             // STL map for tracking
```

---

### 3. **mock_esp32_test.ino** (Testing - No BLE Required)

**Purpose:** Testing tool to verify server connectivity without BLE hardware

**Key Features:**
- No BLE scanning (removes complexity)
- Generates mock employee tokens (EMP001, EMP002, EMP003)
- Simulates walk-in/walk-out events automatically
- Perfect for testing server integration

**How It Works:**
```cpp
1. Connects to WiFi
2. Every 10 seconds, sends mock data:
   - EMP001 IN → EMP001 OUT → EMP002 IN → EMP002 OUT → ...
3. Displays full HTTP response for debugging
4. Auto-cycles through all mock tokens
```

**When to Use:**
- ✅ Testing server setup before deploying ESP32
- ✅ Debugging network connectivity issues
- ✅ Verifying dashboard functionality
- ✅ Development without BLE beacons

**Serial Monitor Output Example:**
```
╔═══════════════════════════════════════╗
║  ESP32 Mock Server Test               ║
╚═══════════════════════════════════════╝

ESP Device ID: ESP32_MOCK_001
Server URL: http://192.168.1.102:3000/api/presence

✅ Wi-Fi Connected!
📡 IP Address: 192.168.1.123
📶 Signal Strength: -45 dBm

🎭 Mock Event #1: EMP001 - IN

📤 Sending mock data...
Payload: {"token":"EMP001","esp_id":"ESP32_MOCK_001","action":"IN","timestamp":12345}
✅ Response Code: 201
Response: {"success":true,"message":"Presence event logged"...}
─────────────────────────────────────
⏳ Waiting 10 seconds before next event...
```

**Customization:**
```cpp
// Add more mock employees
const char* mockTokens[] = {"EMP001", "EMP002", "EMP003", "EMP004"};
const int numTokens = 4;

// Change interval
delay(5000);  // 5 seconds instead of 10
```

---

### 4. **server/server.js** (Backend Server)

**Purpose:** Node.js/Express server that receives, processes, and stores attendance data

**Key Components:**

#### **Data Storage (In-Memory)**
```javascript
const presenceLogs = [];           // Array of all presence events
const deviceStatus = new Map();    // Current status of each device
```

#### **API Endpoints:**

| Method | Endpoint | Description |
|--------|----------|-------------|
| POST | `/api/presence` | Receive presence events from ESP32 |
| GET | `/api/logs` | Get recent logs (query: `?limit=50`) |
| GET | `/api/status` | Get current status of all devices |
| GET | `/api/logs/:token` | Get logs for specific employee token |
| POST | `/api/clear` | Clear all logs and device status |
| GET | `/health` | Server health check |

#### **Presence Event Processing:**

```javascript
POST /api/presence
Request Body:
{
  "token": "EMP001",
  "esp_id": "ESP32_001",
  "action": "IN",
  "timestamp": 1234567890
}

Response:
{
  "success": true,
  "message": "Presence event logged",
  "data": { ... }
}
```

**Features:**
- ✅ Input validation
- ✅ Automatic timestamp generation
- ✅ Device status tracking (present/absent)
- ✅ Log rotation (keeps last 1000 entries)
- ✅ Console logging for debugging
- ✅ CORS-friendly (serves static dashboard)

**Console Output Example:**
```
==================================================
🚀 BLE Attendance Server Running
📡 Listening on http://localhost:3000
📊 Dashboard: http://localhost:3000
==================================================
[2025-10-17T12:30:45.123Z] IN - Token: EMP001 | ESP: ESP32_001
[2025-10-17T12:45:12.456Z] OUT - Token: EMP001 | ESP: ESP32_001
🗑️  Logs cleared! Removed 25 logs and 3 device statuses
```

**Configuration:**
```javascript
const PORT = 3000;  // Server port
```

---

### 5. **server/public/index.html** (Web Dashboard)

**Purpose:** Real-time web dashboard for monitoring attendance

**Features:**

#### **Statistics Panel**
- 📊 Total Events (all check-ins/check-outs)
- 👥 Active Devices (currently present)
- ✅ Check-ins (IN events)
- ❌ Check-outs (OUT events)

#### **Recent Activity Panel**
- 📝 Last 50 events displayed
- 🟢 Green highlight for IN events
- 🔴 Red highlight for OUT events
- 🕐 Timestamp for each event
- 📡 ESP device ID shown
- 🔄 Refresh button (manual)
- 🗑️ Clear Logs button (with confirmation)
- ⏰ Auto-refresh every 5 seconds

#### **Device Status Panel**
- 👤 List of all detected employees
- 🟢 Green indicator = Present
- 🔴 Red indicator = Absent
- 📅 Last seen timestamp
- 📡 Associated ESP device

**Technology Stack:**
- Pure HTML/CSS/JavaScript (no frameworks)
- Fetch API for AJAX requests
- Responsive design (mobile-friendly)
- Modern gradient UI design

**Auto-Refresh Logic:**
```javascript
// Loads data every 5 seconds automatically
setInterval(loadData, 5000);
```

**API Calls:**
```javascript
// Fetch logs
GET /api/logs?limit=50

// Fetch device status
GET /api/status

// Clear all logs
POST /api/clear
```

---

### 6. **BRD - Wireless Attendance .txt** (Business Requirements)

**Purpose:** Complete business requirements document for the system

**Contents:**
- System overview and objectives
- Functional scope by component (mobile, ESP32, backend, dashboard)
- Token and identification policy
- Attendance logic rules
- Security considerations
- Success criteria
- Future roadmap

**Key Specifications:**
- Service UUID: `74686d62-51ac-7e8b-4a60-b0c73fbb0fbd` (as per BRD)
- Note: Current implementation uses `0xD7E1A3F4` for POC
- Timeout logic: configurable (default 15 seconds)
- Multi-zone support ready

---

## ⚙️ How It Works

### Complete Flow Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                        STEP 1: BLE Broadcasting                  │
│  Mobile Device broadcasts BLE advertisement every 1-2 seconds    │
│  Payload: Service UUID + Employee Token                          │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│                        STEP 2: BLE Scanning                      │
│  ESP32 scans for BLE advertisements every 5 seconds              │
│  Filters by Service UUID (0xD7E1A3F4)                           │
│  Extracts employee token from service data                       │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│                     STEP 3: Presence Detection                   │
│  • First detection today → Mark as Walk-IN                       │
│  • Continuous detection → Update "last seen" timestamp           │
│  • No detection for 15s → Mark as Walk-OUT                       │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│                     STEP 4: Data Transmission                    │
│  ESP32 creates JSON payload:                                     │
│  {                                                                │
│    "token": "EMP001",                                            │
│    "esp_id": "ESP32_001",                                        │
│    "action": "IN",                                               │
│    "timestamp": 1234567890                                       │
│  }                                                                │
│  Sends via HTTP POST to server                                   │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│                     STEP 5: Server Processing                    │
│  • Validates payload                                             │
│  • Stores in presenceLogs array                                  │
│  • Updates deviceStatus map                                      │
│  • Logs to console                                               │
│  • Returns success response                                      │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│                     STEP 6: Dashboard Display                    │
│  • Fetches /api/logs every 5 seconds                            │
│  • Fetches /api/status every 5 seconds                          │
│  • Updates UI with color-coded events                            │
│  • Shows real-time statistics                                    │
│  • Displays present/absent indicators                            │
└─────────────────────────────────────────────────────────────────┘
```

### Timeout Logic

```
Employee Present:
─────────●─────●─────●─────●─────●─────●─────●─────
         ↑     ↑     ↑     ↑     ↑     ↑     ↑
      Detected every 5 seconds (BLE scan interval)
      Status: PRESENT

Employee Leaves:
─────────●─────●─────────────────────────────────────
         ↑     ↑     
      Last seen        15 seconds timeout
      Status: PRESENT → Walk-OUT triggered → ABSENT
```

---

## 🔧 Configuration

### ESP32 Configuration

**WiFi Settings:**
```cpp
const char* WIFI_SSID = "Your_Network_Name";
const char* WIFI_PASS = "Your_Network_Password";
```

**Server URL:**
```cpp
// Replace with your server's IP address
const char* SERVER_URL = "http://192.168.1.102:3000/api/presence";
```

**ESP Device ID:**
```cpp
// Unique identifier for each ESP32 device
const char* ESP_ID = "ESP32_001";  // Change for each device
```

**BLE Settings:**
```cpp
#define SERVICE_UUID32 0xD7E1A3F4  // Service UUID to scan for
int scanTime = 5;                   // Scan duration in seconds
const unsigned long TIMEOUT_MS = 15000;  // 15 seconds timeout
```

### Server Configuration

**Port:**
```javascript
const PORT = 3000;  // Change if port 3000 is in use
```

**Log Retention:**
```javascript
// Keeps last 1000 logs in memory
if (presenceLogs.length > 1000) {
    presenceLogs.shift();
}
```

**CORS (if needed):**
```javascript
// Add CORS middleware for cross-origin requests
app.use((req, res, next) => {
    res.header('Access-Control-Allow-Origin', '*');
    next();
});
```

---

## 🔌 API Reference

### POST /api/presence

**Description:** Receive presence event from ESP32

**Request:**
```json
{
  "token": "EMP001",
  "esp_id": "ESP32_001",
  "action": "IN",
  "timestamp": 1234567890
}
```

**Response (Success - 201):**
```json
{
  "success": true,
  "message": "Presence event logged",
  "data": {
    "token": "EMP001",
    "esp_id": "ESP32_001",
    "action": "IN",
    "timestamp": 1234567890,
    "received_at": "2025-10-17T12:30:45.123Z"
  }
}
```

**Response (Error - 400):**
```json
{
  "error": "Missing required fields: token, esp_id, action"
}
```

---

### GET /api/logs

**Description:** Get recent attendance logs

**Query Parameters:**
- `limit` (optional): Number of logs to return (default: 100, max: 1000)

**Request:**
```
GET /api/logs?limit=50
```

**Response:**
```json
{
  "total": 150,
  "logs": [
    {
      "token": "EMP001",
      "esp_id": "ESP32_001",
      "action": "IN",
      "timestamp": 1234567890,
      "received_at": "2025-10-17T12:30:45.123Z"
    },
    ...
  ]
}
```

---

### GET /api/status

**Description:** Get current status of all devices

**Response:**
```json
{
  "devices": [
    {
      "token": "EMP001",
      "present": true,
      "lastSeen": "2025-10-17T12:30:45.123Z",
      "esp_id": "ESP32_001"
    },
    {
      "token": "EMP002",
      "present": false,
      "lastSeen": "2025-10-17T11:15:30.456Z",
      "esp_id": "ESP32_001"
    }
  ]
}
```

---

### GET /api/logs/:token

**Description:** Get logs for a specific employee token

**Request:**
```
GET /api/logs/EMP001
```

**Response:**
```json
{
  "token": "EMP001",
  "count": 10,
  "logs": [ ... ]
}
```

---

### POST /api/clear

**Description:** Clear all logs and device status

**Response:**
```json
{
  "success": true,
  "message": "All logs cleared",
  "cleared": {
    "logs": 150,
    "devices": 5
  }
}
```

---

### GET /health

**Description:** Server health check

**Response:**
```json
{
  "status": "OK",
  "uptime": 3600.5,
  "logs_count": 150,
  "active_devices": 5
}
```

---

## 🐛 Troubleshooting

### ESP32 Issues

#### ❌ WiFi Connection Failed
**Problem:** ESP32 can't connect to WiFi

**Solutions:**
1. Verify SSID and password are correct
2. Ensure WiFi is 2.4GHz (ESP32 doesn't support 5GHz)
3. Check if WiFi has special characters (use `\"` for quotes in strings)
4. Move ESP32 closer to router
5. Check Serial Monitor for specific error messages

```cpp
// Enable WiFi debugging
WiFi.setDebugLevel(WIFI_DEBUG_VERBOSE);
```

#### ❌ BLE Devices Not Detected
**Problem:** ESP32 scans but finds no devices

**Solutions:**
1. Verify Service UUID matches mobile app
2. Ensure mobile device Bluetooth is ON
3. Check if mobile app is broadcasting in background
4. Reduce distance between ESP32 and mobile device
5. Try increasing scan time:
   ```cpp
   int scanTime = 10;  // Increase to 10 seconds
   ```

#### ❌ Connection Refused / Server Error
**Problem:** ESP32 can't connect to server

**Solutions:**
1. Verify server IP address is correct:
   ```powershell
   ipconfig  # Run on server computer
   ```
2. Ensure server is running:
   ```powershell
   netstat -an | findstr :3000
   ```
3. Check firewall isn't blocking port 3000
4. Verify ESP32 and server are on same network
5. Test with browser: `http://SERVER_IP:3000/health`

#### ❌ ArduinoJson Compile Error
**Problem:** Error during compilation

**Solution:**
Install ArduinoJson library:
1. Sketch → Include Library → Manage Libraries
2. Search "ArduinoJson"
3. Install version 6.21.0 or higher

---

### Server Issues

#### ❌ Cannot Start Server (Port in Use)
**Problem:** `Error: listen EADDRINUSE :::3000`

**Solutions:**
1. Find process using port 3000:
   ```powershell
   netstat -ano | findstr :3000
   ```
2. Kill the process:
   ```powershell
   taskkill /PID <PID> /F
   ```
3. Or change port in `server.js`:
   ```javascript
   const PORT = 3001;  // Use different port
   ```

#### ❌ Module Not Found Error
**Problem:** `Error: Cannot find module 'express'`

**Solution:**
```powershell
cd server
npm install
```

#### ❌ Dashboard Not Loading
**Problem:** Blank page or 404 error

**Solutions:**
1. Verify server is running
2. Check browser console for errors (F12)
3. Clear browser cache (Ctrl+Shift+Delete)
4. Try different browser
5. Check server console for errors

---

### Dashboard Issues

#### ❌ No Data Showing
**Problem:** Dashboard loads but shows "No activity yet"

**Solutions:**
1. Verify ESP32 is sending data (check Serial Monitor)
2. Check server console for incoming requests
3. Click "Refresh" button manually
4. Open browser DevTools (F12) → Network tab → Check API calls
5. Verify ESP32 has correct server URL

#### ❌ Clear Logs Button Not Working
**Problem:** "Failed to clear logs" error

**Solutions:**
1. Restart server (Ctrl+C, then `npm start`)
2. Hard refresh browser (Ctrl+Shift+R)
3. Check browser console for errors
4. Verify server has `/api/clear` endpoint

---

## 📊 Testing Checklist

### Before Deployment

- [ ] Arduino libraries installed (especially ArduinoJson)
- [ ] WiFi credentials configured correctly
- [ ] Server IP address is correct
- [ ] ESP32 can connect to WiFi
- [ ] Server is running and accessible
- [ ] Dashboard loads in browser
- [ ] Mock test successful (using `mock_esp32_test.ino`)

### After Deployment

- [ ] ESP32 detects BLE beacons
- [ ] Walk-IN events logged correctly
- [ ] Walk-OUT events triggered after timeout
- [ ] Dashboard updates automatically
- [ ] Multiple employees tracked simultaneously
- [ ] Clear logs functionality works
- [ ] Server remains stable over time

---

## 🚀 Future Enhancements

Based on the BRD, future enhancements include:

### Phase 1 (POC Complete) ✅
- [x] ESP32 BLE scanning
- [x] Server integration
- [x] Web dashboard
- [x] Basic attendance tracking

### Phase 2 (Planned)
- [ ] Database integration (MongoDB/PostgreSQL)
- [ ] Token rotation for security
- [ ] Multiple ESP32 zones
- [ ] NTP time synchronization
- [ ] HTTPS/TLS encryption
- [ ] API authentication (JWT)

### Phase 3 (Future)
- [ ] Mobile app with BLE broadcasting
- [ ] HR dashboard with reporting
- [ ] Break time tracking
- [ ] Export to Excel/CSV
- [ ] Email notifications
- [ ] Integration with payroll systems
- [ ] AI-based anomaly detection

---

## 📝 License

This project is for internal use and testing purposes.

---

## 🤝 Contributing

For questions or issues:
1. Check troubleshooting section
2. Review Serial Monitor output
3. Check server console logs
4. Verify network connectivity

---

## 📞 Support

**Common Commands:**

Find your IP:
```powershell
ipconfig
```

Start server:
```powershell
cd server
npm start
```

Check server status:
```powershell
netstat -an | findstr :3000
```

Monitor ESP32:
- Open Arduino IDE → Tools → Serial Monitor (115200 baud)

---

## 📚 Additional Resources

- [ESP32 BLE Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/esp_bt.html)
- [ArduinoJson Documentation](https://arduinojson.org/)
- [Express.js Guide](https://expressjs.com/en/guide/routing.html)
- [BLE Advertising Basics](https://learn.adafruit.com/introduction-to-bluetooth-low-energy/gap)

---

**Last Updated:** October 17, 2025  
**Version:** 1.0.0  
**Status:** Production Ready (POC Phase)
