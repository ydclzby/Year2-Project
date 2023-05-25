var express = require('express'); 
var server = express();
server.get('/', function(req, res) {
    res.sendFile('/home/ubuntu/index.html'); 
});

server.get('/primeOrNot.js', function(req, res) { 
    res.sendFile('/home/ubuntu/primeOrNot.js');
});
console.log('Server is running on port 3000'); 
server.listen(3000,'0.0.0.0');