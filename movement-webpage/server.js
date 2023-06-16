const express = require('express');
const bodyParser = require('body-parser');
const app = express();
const port = 3000;

let roverState = 'STOP';
let kpValue = 500;
let kdValue = 15;

app.use(bodyParser.urlencoded({ extended: false }));
app.use(bodyParser.json());

app.use(express.static('public'));

app.post('/control', (req, res) => {
  const { state } = req.body;
  roverState = state;
  console.log('Rover state:', roverState);
  res.sendStatus(200);
});

app.post('/adjust', (req, res) => {
  const { kp, kd } = req.body;
  kpValue = kp;
  kdValue = kd;
  console.log('Kp:', kpValue);
  console.log('Kd:', kdValue);
  res.sendStatus(200);
});

app.get('/state', (req, res) => {
  res.json({ state: roverState });
});

app.get('/kp', (req, res) => {
  res.json({ kp: kpValue });
});

app.get('/kd', (req, res) => {
  res.json({ kd: kdValue });
});

app.listen(port, () => {
  console.log(`Server listening on port ${port}`);
});
