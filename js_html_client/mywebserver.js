var express = require('express');
var server = express();

server.get('/', function(req,res){
    res.sendFile('');
});

server.get('/primeOrNot.js', function(req,res){
    res,senFile();
});

console.log('Server is running on port 3000');
server.listen(300,'127.0.0.1');