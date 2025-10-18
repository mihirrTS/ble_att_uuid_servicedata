# BLE Attendance Server

A simple Node.js server to receive and display attendance logs from ESP32 devices.

## Features

- ✅ RESTful API to receive presence events from ESP32
- 📊 Real-time dashboard to view logs and device status
- 💾 In-memory storage (can be extended to database)
- 🔄 Auto-refresh dashboard every 5 seconds
- 📱 Device status tracking (IN/OUT)

## Setup

### 1. Install Dependencies

```bash
cd server
npm install
```

### 2. Start the Server

```bash
npm start
```

Or for development with auto-reload:

```bash
npm run dev
```

### 3. Access the Dashboard

Open your browser and go to:
```
http://localhost:3000
```

## API Endpoints

### POST `/api/presence`
Receive presence events from ESP32.

**Request Body:**
```json
{
  "token": "employee_token",
  "esp_id": "ESP32_001",
  "action": "IN",
  "timestamp": 1234567890
}
```

**Response:**
```json
{
  "success": true,
  "message": "Presence event logged",
  "data": { ... }
}
```

### GET `/api/logs?limit=50`
Get recent logs (default: 100, max: 1000)

### GET `/api/status`
Get current status of all devices

### GET `/api/logs/:token`
Get logs for a specific token

### GET `/health`
Server health check

## ESP32 Configuration

Update the following in `server_att.ino`:

```cpp
const char* SERVER_URL = "http://YOUR_SERVER_IP:3000/api/presence";
const char* ESP_ID = "ESP32_001";  // Unique ID for each ESP device
```

**Important:** Replace `YOUR_SERVER_IP` with your actual server IP address. You can find it by running:
- Windows: `ipconfig`
- Linux/Mac: `ifconfig` or `ip addr`

## Arduino Library Dependencies

Make sure you have installed the following library in Arduino IDE:

- **ArduinoJson** by Benoit Blanchon (version 6.x or higher)

To install:
1. Open Arduino IDE
2. Go to Sketch → Include Library → Manage Libraries
3. Search for "ArduinoJson"
4. Install version 6.21.0 or higher

## Production Considerations

For production use, consider:

1. **Database Integration:** Replace in-memory storage with MongoDB, PostgreSQL, etc.
2. **Authentication:** Add API key or JWT authentication
3. **HTTPS:** Use SSL/TLS certificates
4. **Time Synchronization:** Use NTP on ESP32 for accurate timestamps
5. **Error Handling:** Implement retry logic on ESP32
6. **Logging:** Use proper logging framework (Winston, etc.)
7. **Monitoring:** Add health checks and alerting

## Troubleshooting

### ESP32 can't connect to server
- Verify the server IP address is correct
- Ensure both ESP32 and server are on the same network
- Check firewall settings
- Test the endpoint using Postman or curl

### Dashboard not updating
- Check browser console for errors
- Verify server is running on port 3000
- Clear browser cache

### "Module not found" error
- Run `npm install` in the server directory
- Ensure you're in the correct directory

## License

MIT
