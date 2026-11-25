#pragma once

#include "QuantumCircuit.h"
#include "QuantumSimulator.h"
#include <mutex>
#include <string>

namespace qlite {

/**
 * @brief REST API server built on the Crow C++ web framework.
 *
 * Exposes quantum circuit management and simulation endpoints over HTTP/JSON
 * and serves the Quantum Lite frontend assets.
 */
class ApiServer {
private:
    int port_;
    std::string frontendDir_;
    QuantumCircuit activeCircuit_;
    QuantumSimulator simulator_;
    mutable std::mutex circuitMutex_;

    static std::string resolveFrontendDirectory(const std::string& hint);

public:
    explicit ApiServer(int port = 8080, const std::string& frontendDir = "");
    void run();
};

} // namespace qlite
