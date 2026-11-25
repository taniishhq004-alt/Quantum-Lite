# Quantum Lite — Quantum Circuit Simulator

**Quantum Lite** is an interactive, browser-based Quantum Circuit Simulator built for beginners learning quantum computing. The core quantum simulation engine is implemented in **C++17** using **Object-Oriented Programming (OOP)** principles and exposed via a **Crow C++** JSON REST API, paired with a modern **HTML5 / CSS3 / Vanilla JavaScript / Chart.js** frontend.

> **Academic Context:**
> - **Institute:** Jaypee Institute of Information Technology (JIIT)
> - **Course:** Bachelor of Computer Applications (BCA)
> - **Subject:** C++ Using OOPS (`23B65CA215`) — ODD, III Semester (Session 2026–27)
> - **Team:** Tanishq Gupta (`992505170075`), Tanmay Thakur (`992505170082`), Abhinav Rathore (`992505170061`), Rajat Singh (`992505170083`)

---

## Key Features

- **Interactive Circuit Builder:** Add/remove qubits (up to 8 in UI, 10 in engine), toggle initial basis states (`|0⟩` / `|1⟩`), drag-and-drop or click-to-place gates, connect control and target qubits with CNOT, and reset circuits.
- **C++17 OOP Simulation Engine:** Full complex state-vector simulation using `std::complex<double>`, tensor-product state initialization, polymorphic gate application (`QuantumGate` hierarchy), and normalization verification (`∑|αᵢ|² = 1`).
- **Supported Quantum Gates:**
  - **Hadamard (`H`)** — Creates equal quantum superposition.
  - **Pauli-X (`X`)** — Quantum NOT / bit-flip gate.
  - **Pauli-Y (`Y`)** — Bit and phase rotation around the Y-axis.
  - **Pauli-Z (`Z`)** — Phase-flip gate on the `|1⟩` component.
  - **Controlled-NOT (`CNOT`)** — Two-qubit entangling gate (e.g., Bell & GHZ states).
- **Educational Visual Analytics:** Displays Dirac notation (`|ψ⟩`), complex amplitudes (`a + bi`), magnitudes, measurement probability bar charts (Chart.js), and a step-by-step gate impact trace.
- **Automated C++ Unit Tests:** 10 test suites verifying single-qubit gates, CNOT, Bell state, 3-qubit GHZ state, normalization, reset, and exception handling.

---

## Project Structure

```text
Quantum-Lite/
├── backend/
│   ├── include/
│   │   ├── ApiServer.h
│   │   ├── Gates.h
│   │   ├── MatrixUtils.h
│   │   ├── QuantumCircuit.h
│   │   ├── QuantumGate.h
│   │   ├── QuantumSimulator.h
│   │   └── Qubit.h
│   ├── src/
│   │   ├── ApiServer.cpp
│   │   ├── Gates.cpp
│   │   ├── MatrixUtils.cpp
│   │   ├── QuantumCircuit.cpp
│   │   ├── QuantumSimulator.cpp
│   │   ├── Qubit.cpp
│   │   └── main.cpp
│   ├── third_party/        # Header-only Crow & standalone Asio
│   └── CMakeLists.txt
├── frontend/
│   ├── index.html
│   ├── css/style.css
│   └── js/
│       ├── api.js
│       ├── app.js
│       └── circuit.js
├── tests/
│   └── simulation_tests.cpp
├── docs/
│   ├── Project_Report.docx
│   └── Project_Report.pdf
├── .gitignore
└── README.md
```

---

## Prerequisites (Windows)

1. **C++17 Compiler:**
   - **Recommended:** MinGW-w64 (GCC 11+ with POSIX threads, e.g., [WinLibs](https://winlibs.com/) or MSYS2 UCRT64), **or** Microsoft Visual Studio 2019/2022 ("Desktop development with C++").
2. **CMake (3.16+):**
   - Download from [cmake.org/download](https://cmake.org/download/) and select **"Add CMake to the system PATH"**.
3. **Web Browser:** Any modern browser (Chrome, Edge, Firefox).
4. **VS Code (Optional):** Install the *C/C++* and *CMake Tools* extensions.

> **Note on Dependencies:** Header-only **Crow C++** and standalone **Asio** are already bundled inside `backend/third_party/`, so no external package manager (`vcpkg` / `conan`) or internet download is required to compile on another Windows laptop after extracting the ZIP.

---

## Setup & Compilation Instructions

Open **PowerShell** or **Command Prompt** inside the extracted `Quantum-Lite` folder:

### Option A: Build with MinGW-w64 (GCC)

1. Ensure `g++` (MinGW-w64) and `cmake` are on your `PATH`:
   ```powershell
   g++ --version
   cmake --version
   ```
2. Configure and build the backend and test executable:
   ```powershell
   cmake -S backend -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
   cmake --build build -j
   ```
   *(If `mingw32-make` is installed without `Ninja`, `-G "MinGW Makefiles"` works out of the box; if using Ninja, use `-G "Ninja"`.)*

### Option B: Build with Visual Studio (MSVC)

```powershell
cmake -S backend -B build
cmake --build build --config Release
```

---

## Running Tests

Run the C++ simulation engine unit tests using CTest or the compiled test binary:

```powershell
# Using CTest
ctest --test-dir build --output-on-failure

# Or directly (MinGW)
.\build\quantum_lite_tests.exe

# Or directly (MSVC)
.\build\Release\quantum_lite_tests.exe
```

Expected output: `All 10/10 test suites passed successfully!`

---

## Running the Application (Backend + Frontend)

### 1. Start the C++ Backend Server

```powershell
# MinGW build:
.\build\quantum_lite_server.exe

# MSVC build:
.\build\Release\quantum_lite_server.exe
```

You should see:
```text
====================================================
  Quantum Lite — C++ Quantum Circuit Simulator
  Server listening at: http://localhost:8080
  Serving frontend from: .../Quantum-Lite/frontend
====================================================
```

### 2. Open the Frontend in Your Browser

- **Recommended (Integrated):** Open your browser and navigate directly to:
  ```text
  http://localhost:8080
  ```
  The C++ Crow server serves both the REST API (`/api/*`) and the static frontend files (`index.html`, CSS, and JavaScript).
- **Alternative (VS Code Live Server):** If you serve `frontend/index.html` separately (e.g., on `http://127.0.0.1:5500`), ensure `quantum_lite_server.exe` is running on port `8080`. CORS is enabled on the C++ backend so API calls automatically connect to `http://localhost:8080`.

---

## REST API Reference

| Method | Endpoint | Description |
| :--- | :--- | :--- |
| `GET` | `/api/health` | Returns backend status, service name, and C++ standard version. |
| `GET` | `/api/gates` | Returns metadata and matrices for `H`, `X`, `Y`, `Z`, and `CNOT`. |
| `POST` | `/api/circuit` | Validates and stores a circuit JSON configuration. |
| `POST` | `/api/simulate` | Runs C++ state-vector simulation and returns amplitudes, probabilities, Dirac strings, and execution steps. |
| `POST` | `/api/reset` | Resets the active server-side circuit to `|0...0⟩`. |

### Sample `POST /api/simulate` Request (Bell State)

```json
{
  "numQubits": 2,
  "initialStates": ["0", "0"],
  "gates": [
    { "type": "H", "qubits": [0], "column": 0 },
    { "type": "CNOT", "qubits": [0, 1], "column": 1 }
  ]
}
```

---

## Troubleshooting

- **`cmake` or `g++` is not recognized:** Verify that CMake (`bin`) and MinGW-w64 (`bin`) are added to your Windows `PATH` environment variable, then reopen your terminal.
- **Legacy 32-bit MinGW (`gcc 6.x`, win32 threads) errors:** Crow requires C++17 and `std::thread`/`std::mutex`. Upgrade to a modern MinGW-w64 POSIX-threads distribution (such as WinLibs GCC 13/14+) or Visual Studio 2022.
- **Port 8080 already in use:** Pass a custom port as the first argument:
  ```powershell
  .\build\quantum_lite_server.exe 9090
  ```
- **Frontend shows "C++ Backend Offline":** Confirm `quantum_lite_server.exe` is running in a terminal and no firewall prompt blocked local loopback connections.
