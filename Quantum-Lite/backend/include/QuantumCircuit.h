#pragma once

#include "Gates.h"
#include "Qubit.h"
#include <memory>
#include <string>
#include <vector>

namespace qlite {

/**
 * @brief Manages qubits, gate placement, validation, and execution order.
 *
 * Demonstrates STL containers, smart pointers (RAII), function overloading,
 * deep-copy semantics, and exception handling.
 */
class QuantumCircuit {
private:
    std::vector<Qubit> qubits_;
    std::vector<std::unique_ptr<QuantumGate>> gates_;

public:
    static constexpr int MIN_QUBITS = 1;
    static constexpr int MAX_QUBITS = 10;

    explicit QuantumCircuit(int numQubits = 2);
    QuantumCircuit(const QuantumCircuit& other);
    QuantumCircuit& operator=(const QuantumCircuit& other);
    QuantumCircuit(QuantumCircuit&& other) noexcept = default;
    QuantumCircuit& operator=(QuantumCircuit&& other) noexcept = default;
    ~QuantumCircuit() = default;

    // Qubit management (demonstrates function overloading)
    void addQubit();
    void addQubit(const std::string& label);
    void removeQubit();
    void removeQubit(int qubitIndex);
    void setNumQubits(int numQubits);

    // Gate management (demonstrates function overloading + polymorphism)
    void addGate(std::unique_ptr<QuantumGate> gate);
    void addGate(const std::string& symbol, int targetQubit, int column);
    void addGate(const std::string& symbol, int controlQubit, int targetQubit, int column);
    bool removeGate(int column, int qubit);
    void clearGates();

    // Circuit reset (overloaded)
    void reset();
    void reset(int numQubits);

    // Validation & inspection
    void validate() const;
    int getNumQubits() const;
    int getDepth() const;
    size_t getGateCount() const;
    std::vector<const QuantumGate*> getOrderedGates() const;
    const std::vector<Qubit>& getQubits() const;
    std::vector<Qubit>& getQubitsMutable();
    std::string getSummary() const;
};

} // namespace qlite
