# 🧵 special-engine 
## Boost Multithreaded TCP Socket Server

A lightweight multithreaded TCP server built in C++ using [Boost.Asio](https://www.boost.org/doc/libs/release/doc/html/boost_asio.html) and `boost::thread`. This server listens for incoming client connections, spawns a dedicated thread for each client, and echoes back any message received.

---

## 📌 Features

- ✅ Accepts multiple clients concurrently using `boost::thread`
- ✅ Handles basic message exchange with read/write functionality
- ✅ Simple logging of incoming client messages
- ✅ Robust error handling with clean client disconnection detection
- ✅ Fully commented and beginner-friendly C++/Boost project structure

---

## 🧰 Requirements

- C++ compiler supporting C++11 or higher (e.g., `g++`, `clang++`, `MSVC`)
- [Boost C++ Libraries (v1.78+ recommended)](https://www.boost.org/)
  - Required modules: `asio`, `thread`, `system`
- [CMake (3.14+ recommended)](https://cmake.org/)
- Windows/Linux/Mac OS

---

## 🚀 Getting Started

### 1. Clone the Repository

```bash
git clone https://github.com/yourusername/boost-tcp-server.git
cd socketserver
```

### 2. Build with CMake

> 🔧 Make sure Boost is properly installed and discoverable by CMake.

```bash
mkdir build
cd build
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release .. // MinGW Makefiles is for the Windows version
make . //to build or use the command below
cmake --build .
```

To specify custom Boost paths:

```bash
cmake -DBOOST_ROOT="C:/local/boost_1_78_0" ..
```

### 3. Run the Server

```bash
./boost_websocket_server
```

The server will start and listen on port **1234**.

---

## 🖧 How It Works

### Main Flow

```text
Main thread:
┌─────────────────────────────┐
│ Create io_context           │
│ Bind to TCP port 1234       │
│ Loop:                       │
│   Accept client connection  │
│   Launch thread to handle   │
└─────────────────────────────┘

Worker thread (per client):
┌──────────────────────────────┐
│ Read data from socket        │
│ Log message                  │
│ Send confirmation response   │
│ Repeat until client exits    │
└──────────────────────────────┘
```

### Threaded Client Handling

- Each client connection is handed off to a new thread via `boost::thread`.
- Communication continues until the client disconnects or sends EOF.
- Threads are detached to run independently.

---

## 📁 Project Structure

```bash
boost-tcp-server/
├── client/
├── └── main.js
├── └── package.json     
├── socketserver/
├── └── server.cpp       # Main server logic
├── └── CMakeLists.txt   # CMake build script   
├── include/             # (optional) Header files
└── README.md            # Project documentation
```

---

## 🧪 Testing

You can test the server using `telnet`, `netcat`, or a simple Node.js/WebSocket client:

```bash
telnet localhost 1234
```

Or use:

```bash
nc localhost 1234
```

---

## 🛠️ Future Enhancements

- Replace `boost::thread` with a thread pool
- Add SSL/TLS encryption
- Implement client authentication
- Support structured messaging (e.g., JSON, Protocol Buffers)
- Add WebSocket support
- Extend to a full chat application

---

## 📄 License

This project is open-source and available under the [MIT License](LICENSE).

---

## 👤 Author

**Tyree Richardson**  
🔗 [LinkedIn](https://www.linkedin.com/in/tyree-richardson-5b564118a/)  
📧 tyree.yourname@example.com *(Replace with your actual email)*  

---

> 💬 *"Built to learn. Built to scale. Built for the Hive."*
