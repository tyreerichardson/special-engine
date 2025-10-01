import React, { useEffect, useState } from 'react';

const ClientSocket = () => {
  const [socket, setSocket] = useState(null);
  const [message, setMessage] = useState('');
  const [receivedMessages, setReceivedMessages] = useState([]);

  useEffect(() => {
    // Establish connection on component mount
    const ws = new WebSocket('ws://localhost:1234'); // Replace with your server URL

    ws.onopen = () => {
      console.log('WebSocket connection established.');
      setSocket(ws);
    };

    ws.onmessage = (event) => {
      console.log('Message received:', event.data);
      setReceivedMessages((prevMessages) => [...prevMessages, event.data]);
    };

    ws.onclose = () => {
      console.log('WebSocket connection closed.');
      setSocket(null); // Clear socket on close
    };

    ws.onerror = (error) => {
      console.error('WebSocket error:', error);
    };

    // Clean up on component unmount
    return () => {
      if (ws.readyState === WebSocket.OPEN) {
        ws.close();
      }
    };
  }, []); // Empty dependency array ensures effect runs only once

  const sendMessage = () => {
    if (socket && socket.readyState === WebSocket.OPEN) {
      socket.send(message);
      setMessage('');
    } else {
      console.warn('WebSocket not connected.');
    }
  };

  return (
    <div>
      <input
        type="text"
        value={message}
        onChange={(e) => setMessage(e.target.value)}
      />
      <button onClick={sendMessage}>Send Message</button>
      <div>
        <h3>Received Messages:</h3>
        <ul>
          {receivedMessages.map((msg, index) => (
            <li key={index}>{msg}</li>
          ))}
        </ul>
      </div>
    </div>
  );
};

export default ClientSocket;
