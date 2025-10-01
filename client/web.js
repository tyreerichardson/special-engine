const WebSocket = require('ws');
const readline = require('readline');

const rl = readline.createInterface({
  input: process.stdin,
  output: process.stdout
});

const ws = new WebSocket("ws://localhost:1234");

ws.onopen = () => {
  console.log("Connected");
  promptMessage();
  //ws.send('{"username":"tyree"}');
};

ws.onmessage = (event) => {
  console.log("Received:", event.data);
};

function promptMessage() {
    rl.question('-> ', input => {
        try {
            const json = JSON.parse(input); // Validates input is JSON
            //client.write(JSON.stringify(json) + '\n'); // Add newline
            ws.send(JSON.stringify(json) + '\n');
        } catch (e) {
            console.log("Invalid JSON. Try again.");
        }
        promptMessage();
    });
}

// const net = require('net');

// const client = new net.Socket();

// client.connect(1234, '127.0.0.1', function() {
//     console.log('Connected to server');
// })
