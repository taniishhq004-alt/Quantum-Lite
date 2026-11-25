#include "QuantumSimulator.h"

#include <cmath>

namespace qlite {

StateVector QuantumSimulator::buildInitialState(const QuantumCircuit& circuit) const {
    const auto& qubits = circuit.getQubits();
    if (qubits.empty()) {
        throw InvalidQubitException("Cannot build initial state for an empty circuit.");
    }

    // Compute the multi-qubit initial state via successive tensor products:
    // |psi_0> = |q0> (x) |q1> (x) ... (x) |q_{n-1}>
    StateVector combined = qubits.front().getStateVector();
    for (size_t i = 1; i < qubits.size(); ++i) {
        combined = MatrixUtils::tensorProductVectors(combined, qubits[i].getStateVector());
    }

    if (!MatrixUtils::isNormalized(combined)) {
        throw NormalizationException("Combined initial state vector is not normalized.");
    }
    return combined;
}

std::vector<double> QuantumSimulator::calculateProbabilities(const StateVector& state) const {
    std::vector<double> probs;
    probs.reserve(state.size());
    for (const Complex& amp : state) {
        double p = std::norm(amp);
        if (p < 1e-15) {
            p = 0.0;
        }
        probs.push_back(p);
    }
    return probs;
}

SimulationResult QuantumSimulator::simulate(const QuantumCircuit& circuit) const {
    circuit.validate();

    const int n = circuit.getNumQubits();
    const size_t dim = 1ULL << n;

    StateVector initialState = buildInitialState(circuit);
    StateVector currentState = initialState;

    std::vector<std::string> basisLabels;
    basisLabels.reserve(dim);
    for (size_t i = 0; i < dim; ++i) {
        basisLabels.push_back(MatrixUtils::basisStateLabel(i, n));
    }

    const auto orderedGates = circuit.getOrderedGates();
    std::vector<ExecutionStep> steps;
    steps.reserve(orderedGates.size());

    int stepIdx = 1;
    for (const QuantumGate* gate : orderedGates) {
        gate->apply(currentState, n);

        if (!MatrixUtils::isNormalized(currentState, 1e-6)) {
            throw NormalizationException(
                "State vector lost normalization after applying gate " + gate->getSymbol() + ".");
        }

        ExecutionStep step;
        step.stepIndex = stepIdx++;
        step.column = gate->getColumn();
        step.gateSymbol = gate->getSymbol();
        step.gateName = gate->getName();
        step.qubits = gate->getTargetQubits();
        step.stateAfter = currentState;
        step.probabilitiesAfter = calculateProbabilities(currentState);
        step.diracAfter = MatrixUtils::formatStateDirac(currentState, n);
        steps.push_back(std::move(step));
    }

    SimulationResult result;
    result.numQubits = n;
    result.circuitDepth = circuit.getDepth();
    result.gateCount = circuit.getGateCount();
    result.initialState = std::move(initialState);
    result.finalState = std::move(currentState);
    result.probabilities = calculateProbabilities(result.finalState);
    result.basisStates = std::move(basisLabels);
    result.normSum = MatrixUtils::normSquaredSum(result.finalState);
    result.isNormalized = MatrixUtils::isNormalized(result.finalState, 1e-6);
    result.initialDirac = MatrixUtils::formatStateDirac(result.initialState, n);
    result.finalDirac = MatrixUtils::formatStateDirac(result.finalState, n);
    result.steps = std::move(steps);

    return result;
}

} // namespace qlite
