const express = require('express');
const app = express();
const mariadb = require('mariadb');

const pool = mariadb.createPool({
  host: '3.86.16.180',
  user: 'bz1521',
  password: '12345678',
  database: 'esp32_test',
  connectionLimit: 10
});

// Middleware to parse JSON request body
app.use(express.json());

// API endpoint to receive data
app.post('/data', async (req, res) => {
  const data = req.body;
  console.log('Received data:', data);

  // Assuming the data object has the expected structure
  const angleX = data.angleX;
  const angleY = data.angleY;
  const angleZ = data.angleZ;
  const timestamp = new Date();

  try {
    const connection = await getConnectionWithRetry();

    const query = `INSERT INTO Angles (angleX, angleY, angleZ, timestamp) VALUES (?, ?, ?, ?)`;
    const values = [angleX, angleY, angleZ, timestamp];

    await connection.query(query, values);

    connection.release();

    res.status(200).send('Data inserted successfully');
  } catch (error) {
    console.error(error);
    res.status(500).send('Error inserting data into MariaDB');
  }
});

// Start the server
const port = 3001;
app.listen(port, () => {
  console.log(`Server running on port ${port}`);
});

async function getConnectionWithRetry() {
  let retries = 5; // Number of connection retry attempts
  let connection;

  while (retries > 0) {
    try {
      connection = await pool.getConnection();
      return connection;
    } catch (error) {
      console.error('Failed to get connection:', error);
      retries--;
      await sleep(1000); // Wait for 1 second before retrying
    }
  }

  throw new Error('Unable to establish database connection');
}

function sleep(ms) {
  return new Promise((resolve) => setTimeout(resolve, ms));
}
