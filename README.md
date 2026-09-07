# special-engine

A C++ WebSocket server being recovered as the future networking foundation for
HiveSim. Phase 0 focuses on a reproducible build, basic messaging, optional
Cassandra, automated checks, and orderly shutdown. Simulation features are not
implemented yet.

Start with **[the server build and test instructions](socketserver/README.md)**.
They cover fresh-checkout macOS setup and Ubuntu 24.04 ARM64, with and without
Cassandra. **[Database setup](cassandra/README.md)** is only needed for persistence.

| Directory | Role |
| --- | --- |
| socketserver | C++ server, CMake, CTest clients/harnesses, Ubuntu validation image |
| scripts | Pinned, checksum-verified local Boost bootstrap |
| client | Interactive Node WebSocket client; older raw TCP client also present |
| cassandra | Local database image and schema/seed script |
| special-engine-fe | Existing frontend, not validated in Phase 0 |

Current work remains a development baseline. Authentication, async session safety,
backpressure, and load handling are not established by the Phase 0 smoke tests.
