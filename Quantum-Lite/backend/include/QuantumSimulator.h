#pragma once

#include "QuantumCircuit.h"
#include <string>
#include <vector>

namespace qlite {

/**
 * @brief Captures the quantum state immediately after applying one gate.
 */
struct ExecutionStep {
    int stepIndex;
    int column;
    std::string gateSymbol;
    std::string gateName;
    std::vector<int> qubits;
    StateVector stateAfter;
    std::vector<double> probabilitiesAfter;
    std::string diracAfter;
};

/**
 * @brief Complete output of simulating a QuantumCircuit.
 */
struct SimulationResult {
    int numQubits;
    int circuitDepth;
    size_t gateCount;
    StateVector initialState;
    StateVector finalState;
    std::vector<double> probabilities;
    std::vector<std::string> basisStates;
    double normSum;
    bool isNormalized;
    std::string initialDirac;
    std::string finalDirac;
    std::vector<ExecutionStep> steps;
};

/**
 * @brief Executes QuantumCircuit instances using state-vector quantum simulation.
 */
class QuantumSimulator {
public:
    QuantumSimulator() = default;
    ~QuantumSimulator() = default;

    StateVector buildInitialState(const QuantumCircuit& circuit) const;
    std::vector<double> calculateProbabilities(const StateVector& state) const;
    SimulationResult simulate(const QuantumCircuit& circuit) const;
};

} // namespace qlite
