const net = require('net');
const readline = require('readline');

const rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout
});

const client = new net.Socket();

client.connect(1234, '127.0.0.1', function() {
    console.log('Connected to server');
    promptMessage();
});

function promptMessage() {
    rl.question('-> ', input => {
        try {
            const json = JSON.parse(input); // Validates input is JSON
            client.write(JSON.stringify(json) + '\n'); // Add newline
        } catch (e) {
            console.log("Invalid JSON. Try again.");
        }
        promptMessage();
    });
}

client.on('data', function(data) {
    console.log('Received: ' + data.toString());
});

client.on('close', function() {
    console.log('Connection closed');
});


/**
 * ** Sample send JSON message **
 * 
 * {"message_id":"2", "message_from":"tyree", "message_to":"alice", "content":"Hi back!", "created_at":"..."}
 *
 */