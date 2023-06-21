// Server code
const express = require('express');
const app = express();
const port = 3000;

app.use(express.static('public'));
app.use(express.json());

app.post('/command', (req, res) => {
  const command = req.body.command;

  if (command === 'stop') {
    console.log('Stopping the rover');
  } else if (command === 'forward') {
    console.log('Moving the rover forward');
  } else if (command === 'left') {
    console.log('Turning the rover left');
  } else if (command === 'right') {
    console.log('Turning the rover right');
  }

  res.sendStatus(200);
});

app.listen(port, () => {
  console.log(`Server running at http://localhost:${port}`);
});
