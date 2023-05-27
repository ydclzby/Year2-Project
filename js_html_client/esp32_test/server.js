const express = require('express');
const WebSocket = require('ws');
const app = express();
const port = 3000;

// Serve the HTML file
app.get('/', (req, res) => {
  res.sendFile(__dirname + '/index.html');
});

// Create a WebSocket server
const wss = new WebSocket.Server({ noServer: true });

// Handle WebSocket connections
wss.on('connection', (ws) => {
  ws.on('message', (data) => {
    console.log('Received data from ESP32:', data);
    // Send the data to all connected clients
    wss.clients.forEach((client) => {
      if (client.readyState === WebSocket.OPEN) {
        client.send(data);
      }
    });
  });
});

// Upgrade HTTP requests to WebSocket
app.server = app.listen(port, () => {
  console.log(`Server is running on port ${port}`);
});

app.server.on('upgrade', (request, socket, head) => {
  wss.handleUpgrade(request, socket, head, (ws) => {
    wss.emit('connection', ws, request);
  });
});
