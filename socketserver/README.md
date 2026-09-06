# Socket server — Phase 0

A C++11 WebSocket server with optional Cassandra persistence. This baseline has
been exercised on macOS Apple Silicon and Ubuntu 24.04 ARM64. Ubuntu x86_64 and
Windows are not validated. Run commands from the `special-engine` application root.

## Mac setup from a fresh checkout

Prerequisites: Apple Command Line Tools (Clang), CMake, Python 3.6+, curl, tar, and
a SHA-256 tool. Install missing CMake/Python with Homebrew if necessary:

```sh
xcode-select --install
brew install cmake python
```

Skip the installer command if Command Line Tools are already installed. Provision
pinned Boost 1.83.0 locally. This downloads an official release, checks its published
SHA-256, and installs only the needed Boost libraries into the ignored build tree:

```sh
sh scripts/bootstrap-boost.sh
```

Build without any Cassandra dependency:

```sh
cmake -S socketserver -B build/mac-off \
  -DBoost_DIR="$PWD/build/deps/boost-1.83.0/lib/cmake/Boost-1.83.0" \
  -DBoost_USE_STATIC_LIBS=ON -DENABLE_CASSANDRA=OFF -DBUILD_TESTING=ON
cmake --build build/mac-off -j4
ctest --test-dir build/mac-off --output-on-failure
./build/mac-off/boost_websocket
```

The existing untracked Boost 1.79 staging tree remains usable on this Mac, but is
not required by these fresh-checkout instructions. CMake prefers an explicit
Boost_DIR; otherwise its local-staging fallback applies only on macOS. A scoped
Clang compatibility flag handles older Boost enum templates. Dependency warnings
may remain; they are not hidden globally.

## Cassandra-enabled Mac build

ON is the default; select OFF explicitly for database-free development. OFF keeps
live username registration and routing, but saves no users or message history. It
is a build configuration, not automatic database failover.

```sh
brew install cassandra-cpp-driver
cmake -S socketserver -B build/mac-on \
  -DBoost_DIR="$PWD/build/deps/boost-1.83.0/lib/cmake/Boost-1.83.0" \
  -DBoost_USE_STATIC_LIBS=ON -DENABLE_CASSANDRA=ON -DBUILD_TESTING=ON
cmake --build build/mac-on -j4
ctest --test-dir build/mac-on --output-on-failure
```

Start Cassandra and apply the schema as described in [the database setup](../cassandra/README.md).
The executable currently connects to 127.0.0.1:9042 and queries startup history for
user123. Missing Cassandra produces a nonzero startup exit.

```sh
./build/mac-on/boost_websocket
```

## Ubuntu 24.04

Use Linux libraries, not copied macOS binaries:

```sh
sudo apt-get update
sudo apt-get install build-essential cmake python3 git ca-certificates \
  libboost-thread-dev libboost-system-dev libboost-json-dev libuv1-dev libssl-dev zlib1g-dev
cmake -S socketserver -B build/ubuntu-off -DENABLE_CASSANDRA=OFF
cmake --build build/ubuntu-off -j4
ctest --test-dir build/ubuntu-off --output-on-failure
```

For ON, install the pinned Cassandra C/C++ driver and then configure separately:

```sh
git clone --depth 1 --branch 2.17.1 https://github.com/apache/cassandra-cpp-driver.git build/cassandra-cpp-driver
cmake -S build/cassandra-cpp-driver -B build/cassandra-driver-build -DCMAKE_BUILD_TYPE=Release
cmake --build build/cassandra-driver-build -j4
sudo cmake --install build/cassandra-driver-build
sudo ldconfig
cmake -S socketserver -B build/ubuntu-on -DENABLE_CASSANDRA=ON
cmake --build build/ubuntu-on -j4
ctest --test-dir build/ubuntu-on --output-on-failure
./build/ubuntu-on/boost_websocket
```

Ubuntu 24.04's Boost 1.83 satisfies the minimum 1.79. Use CMAKE_PREFIX_PATH for a
custom driver install and Boost_DIR for an explicit Boost package location.

### Reproduce Linux validation from the Mac

Docker Desktop must be open. The image builds OFF before installing the driver,
then builds ON and runs both CTest suites with native Linux dependencies. It copies
only source and test files, never the Mac build directories.

```sh
docker build --platform linux/arm64 --progress=plain \
  -f socketserver/Dockerfile.ubuntu -t special-engine-phase0-ubuntu:24.04 socketserver
```

## Tests and client protocol

BUILD_TESTING defaults ON and requires Python. CTest compiles a dedicated
persistence-disabled test server even in an ON build: it never writes test data to
Cassandra. The smoke test covers login, self-message readiness, bidirectional
routing, offline recipients, client close, and orderly process exit. Shutdown tests
cover SIGINT/SIGTERM with idle, raw TCP, partial-handshake, and logged-in clients.
Tests require port 1234 free; CTest serializes them with a resource lock.

```sh
ctest --test-dir build/mac-off --output-on-failure --repeat until-fail:10
```

To omit tests and the Python prerequisite, configure with BUILD_TESTING=OFF.
For interactive use, the existing Node client uses WebSockets (main.js is the older
raw TCP client):

```sh
cd client
npm install
node web.js
```

Log in in each terminal, then send to the other username:

```json
{"username":"alice"}
```

```json
{"message_from":"alice","message_to":"bob","content":"Hello"}
```

Duplicate active usernames are rejected. Empty/invalid login input closes that
client. This is identity selection, not password authentication.

## Shutdown contract and Phase 1 limits

Control-C (SIGINT) or SIGTERM stops acceptance, interrupts blocked TCP operations,
joins every owned worker, releases sessions, and closes the Cassandra driver after
workers finish. Success prints `Server stopped cleanly.` and exits zero. Tests
require exit within eight seconds; driver connect/request timeouts are three seconds.
This is orderly process/resource teardown. Active WebSocket connections can receive
a transport disconnect, not a WebSocket close handshake; pending messages are not
promised delivery during shutdown. A hard kill does not execute cleanup.

The server still uses blocking I/O and one thread per client. Native shutdown uses
POSIX APIs supported by macOS and Ubuntu. Per-session asynchronous serialization,
bounded outbound queues, protocol validation, query parameterization, load tests,
and behavior during database outages remain later work. In particular, shared
ownership does not make concurrent stream operations safe. The server binds all
IPv4 interfaces on port 1234; use it as a local development baseline.
