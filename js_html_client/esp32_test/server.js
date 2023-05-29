const express = require('express');
const app = express();
const port = 3000;

// Middleware to parse JSON requests
app.use(express.json());

// In-memory variable to store the latest received data
let receivedData = null;

// Serve the HTML file
app.get('/', (req, res) => {
  res.sendFile(__dirname + '/index.html');
});

// GET endpoint to retrieve the latest received data
app.get('/esp32data', (req, res) => {
  res.json({ data: receivedData });
});

// POST endpoint to receive data from ESP32
app.post('/esp32data', (req, res) => {
  const newData = req.body.data;
  console.log('Data received from ESP32:', newData);

  // Store the latest received data
  receivedData = newData;

  res.send('Data received successfully');
});

// Start the server
app.listen(port, () => {
  console.log(`Server is running on port ${port}`);
});
