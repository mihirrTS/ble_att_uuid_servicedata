const express = require('express');
const app = express();
const PORT = 3000;

// Middleware
app.use(express.json());
app.use(express.static('public'));

// In-memory storage for logs (in production, use a database)
const presenceLogs = [];
const deviceStatus = new Map();

// Endpoint to receive presence events from ESP32
app.post('/api/presence', (req, res) => {
    const { token, esp_id, action, timestamp } = req.body;
    
    // Validate required fields
    if (!token || !esp_id || !action) {
        return res.status(400).json({ 
            error: 'Missing required fields: token, esp_id, action' 
        });
    }
    
    // Create log entry
    const logEntry = {
        token,
        esp_id,
        action,
        timestamp: timestamp || Date.now(),
        received_at: new Date().toISOString()
    };
    
    // Store log
    presenceLogs.push(logEntry);
    
    // Update device status
    if (action === 'IN') {
        deviceStatus.set(token, {
            present: true,
            lastSeen: logEntry.received_at,
            esp_id
        });
    } else if (action === 'OUT') {
        deviceStatus.set(token, {
            present: false,
            lastSeen: logEntry.received_at,
            esp_id
        });
    }
    
    // Log to console
    console.log(`[${logEntry.received_at}] ${action} - Token: ${token} | ESP: ${esp_id}`);
    
    // Keep only last 1000 logs in memory
    if (presenceLogs.length > 1000) {
        presenceLogs.shift();
    }
    
    res.status(201).json({ 
        success: true, 
        message: 'Presence event logged',
        data: logEntry
    });
});

// Endpoint to get all logs
app.get('/api/logs', (req, res) => {
    const limit = parseInt(req.query.limit) || 100;
    const logs = presenceLogs.slice(-limit).reverse();
    res.json({ 
        total: presenceLogs.length,
        logs 
    });
});

// Endpoint to get device status
app.get('/api/status', (req, res) => {
    const status = Array.from(deviceStatus.entries()).map(([token, info]) => ({
        token,
        ...info
    }));
    res.json({ devices: status });
});

// Endpoint to get logs for a specific token
app.get('/api/logs/:token', (req, res) => {
    const { token } = req.params;
    const logs = presenceLogs.filter(log => log.token === token);
    res.json({ 
        token,
        count: logs.length,
        logs 
    });
});

// Endpoint to clear all logs
app.post('/api/clear', (req, res) => {
    const previousCount = presenceLogs.length;
    const previousDevices = deviceStatus.size;
    
    // Clear logs array
    presenceLogs.length = 0;
    
    // Clear device status
    deviceStatus.clear();
    
    console.log(`🗑️  Logs cleared! Removed ${previousCount} logs and ${previousDevices} device statuses`);
    
    res.json({ 
        success: true,
        message: 'All logs cleared',
        cleared: {
            logs: previousCount,
            devices: previousDevices
        }
    });
});

// Health check endpoint
app.get('/health', (req, res) => {
    res.json({ 
        status: 'OK', 
        uptime: process.uptime(),
        logs_count: presenceLogs.length,
        active_devices: deviceStatus.size
    });
});

// Start server
app.listen(PORT, () => {
    console.log('='.repeat(50));
    console.log(`🚀 BLE Attendance Server Running`);
    console.log(`📡 Listening on http://localhost:${PORT}`);
    console.log(`📊 Dashboard: http://localhost:${PORT}`);
    console.log(`🔍 API Endpoints:`);
    console.log(`   POST /api/presence - Receive presence events`);
    console.log(`   GET  /api/logs     - Get all logs`);
    console.log(`   GET  /api/status   - Get device status`);
    console.log(`   GET  /health       - Health check`);
    console.log('='.repeat(50));
});
