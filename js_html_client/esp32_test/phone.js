const express = require('express');
const app = express();
const port = 3000;

// Middleware to parse JSON requests
app.use(express.json());

// POST endpoint to receive data
app.post('/data', (req, res) => {
  const receivedData = req.body;
  console.log('Received data:', receivedData);
  // Perform any necessary processing or storage of the received data here
  res.send('Data received successfully');
});

// Start the server
app.listen(port, () => {
  console.log(`Server is running on port ${port}`);
});