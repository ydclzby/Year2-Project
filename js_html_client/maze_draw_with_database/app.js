const express = require('express');
const path = require('path');
const mariadb = require('mariadb');

const app = express();
const port = 3000;

const pool = mariadb.createPool({
  host: '3.86.16.180',
  user: 'bz1521',
  password: '12345678',
  database: 'maze_db',
});

app.use(express.static(path.join(__dirname, 'public')));

app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

app.get('/maze', async (req, res) => {
  try {
    const conn = await pool.getConnection();
    const rows = await conn.query('SELECT matrix FROM maze_table WHERE id = 1'); // Assuming maze_table has a single row for the maze
    conn.release();
    
    const mazeMatrix = JSON.parse(rows[0].matrix);
    res.json(mazeMatrix);
  } catch (error) {
    console.error('Error retrieving maze matrix:', error);
    res.status(500).send('Error retrieving maze matrix');
  }
});

app.listen(port, () => {
  console.log(`Server running on http://localhost:${port}`);
});
