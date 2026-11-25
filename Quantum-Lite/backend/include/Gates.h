#pragma once

#include "QuantumGate.h"
#include <memory>
#include <string>
#include <vector>

namespace qlite {

/**
 * @brief Intermediate base class for 1-qubit gates sharing state-vector application logic.
 */
class SingleQubitGate : public QuantumGate {
protected:
    SingleQubitGate(int targetQubit, int column);

public:
    int getNumQubits() const override { return 1; }
    void apply(StateVector& state, int totalQubits) const override;
};

/**
 * @brief Hadamard (H) gate: creates an equal superposition from |0> or |1>.
 */
class HadamardGate : public SingleQubitGate {
public:
    explicit HadamardGate(int targetQubit, int column = 0);

    std::string getName() const override;
    std::string getSymbol() const override;
    std::string getDescription() const override;
    std::string getFunctionSummary() const override;
    std::string getExampleOperation() const override;
    ComplexMatrix getMatrix() const override;
    std::unique_ptr<QuantumGate> clone() const override;
};

/**
 * @brief Pauli-X (NOT) gate: flips |0> <-> |1>.
 */
class PauliXGate : public SingleQubitGate {
public:
    explicit PauliXGate(int targetQubit, int column = 0);

    std::string getName() const override;
    std::string getSymbol() const override;
    std::string getDescription() const override;
    std::string getFunctionSummary() const override;
    std::string getExampleOperation() const override;
    ComplexMatrix getMatrix() const override;
    std::unique_ptr<QuantumGate> clone() const override;
};

/**
 * @brief Pauli-Y gate: rotates qubit state around the Y-axis of the Bloch sphere by pi.
 */
class PauliYGate : public SingleQubitGate {
public:
    explicit PauliYGate(int targetQubit, int column = 0);

    std::string getName() const override;
    std::string getSymbol() const override;
    std::string getDescription() const override;
    std::string getFunctionSummary() const override;
    std::string getExampleOperation() const override;
    ComplexMatrix getMatrix() const override;
    std::unique_ptr<QuantumGate> clone() const override;
};

/**
 * @brief Pauli-Z gate: applies a phase flip (-1) to the |1> component.
 */
class PauliZGate : public SingleQubitGate {
public:
    explicit PauliZGate(int targetQubit, int column = 0);

    std::string getName() const override;
    std::string getSymbol() const override;
    std::string getDescription() const override;
    std::string getFunctionSummary() const override;
    std::string getExampleOperation() const override;
    ComplexMatrix getMatrix() const override;
    std::unique_ptr<QuantumGate> clone() const override;
};

/**
 * @brief Controlled-NOT (CNOT) gate: flips target qubit if control qubit is |1>.
 */
class CNOTGate : public QuantumGate {
public:
    CNOTGate(int controlQubit, int targetQubit, int column = 0);

    std::string getName() const override;
    std::string getSymbol() const override;
    std::string getDescription() const override;
    std::string getFunctionSummary() const override;
    std::string getExampleOperation() const override;
    int getNumQubits() const override { return 2; }
    ComplexMatrix getMatrix() const override;
    void apply(StateVector& state, int totalQubits) const override;
    std::unique_ptr<QuantumGate> clone() const override;

    bool isControlled() const override { return true; }
    int getControlQubit() const override { return targetQubits_[0]; }
    int getPrimaryTargetQubit() const override { return targetQubits_[1]; }
};

/**
 * @brief Factory functions for creating gate instances from symbolic names.
 */
std::unique_ptr<QuantumGate> createGate(
    const std::string& symbol,
    const std::vector<int>& qubits,
    int column = 0);

std::vector<std::unique_ptr<QuantumGate>> createGateCatalog();

} // namespace qlite
