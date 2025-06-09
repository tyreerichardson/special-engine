const net = require('net');

const client = new net.Socket();
client.connect(1234, '127.0.0.1', function() {
    console.log('Connected to server');
    client.write('Hello from Node.js TCP client!');
});

client.on('data', function(data) {
    console.log('Received: ' + data);
});

client.on('close', function() {
    console.log('Connection closed');
});
