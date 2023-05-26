var express = require('express');
var server = express();
var path = require('path');

// Serve static files from the "public" directory
server.use(express.static(path.join(__dirname, 'public')));

// Route handler for the root URL
server.get('/', function(req, res) {
  res.sendFile(path.join(__dirname, 'index.html'));
});

// Start the server
server.listen(3000, function() {
  console.log('Server is running on port 3000');
});