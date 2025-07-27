const net = require('net');
const readline = require('readline');

const rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout
});

const client = new net.Socket();

client.connect(1234, '127.0.0.1', function() {
    console.log('Connected to server');
    rl.question('-> ', input => {
        client.write(input);
    })
});

client.on('data', function(data) {
    console.log('Received: ' + data);
    rl.question('-> ', input => {
        client.write(input);
    })
});

client.on('close', function() {
    console.log('Connection closed');
});

