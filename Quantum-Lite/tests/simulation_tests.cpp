#include "Gates.h"
#include "MatrixUtils.h"
#include "QuantumCircuit.h"
#include "QuantumSimulator.h"
#include "Qubit.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace qlite;

namespace {

constexpr double kEps = 1e-6;

bool approxEqual(double a, double b, double tol = kEps) {
    return std::abs(a - b) <= tol;
}

bool approxComplex(const Complex& a, const Complex& b, double tol = kEps) {
    return approxEqual(a.real(), b.real(), tol) && approxEqual(a.imag(), b.imag(), tol);
}

int gTestsPassed = 0;

void reportPass(const std::string& name) {
    ++gTestsPassed;
    std::cout << "[PASS] " << name << "\n";
}

// 1. Initial single-qubit state |0>
void testInitialSingleQubitState() {
    QuantumCircuit circuit(1);
    QuantumSimulator sim;
    const SimulationResult res = sim.simulate(circuit);

    assert(res.numQubits == 1);
    assert(res.finalState.size() == 2);
    assert(approxComplex(res.finalState[0], Complex(1.0, 0.0)));
    assert(approxComplex(res.finalState[1], Complex(0.0, 0.0)));
    assert(approxEqual(res.probabilities[0], 1.0));
    assert(approxEqual(res.probabilities[1], 0.0));
    assert(res.isNormalized);
    reportPass("1. Initial single-qubit state |0>");
}

// 2. Hadamard gate applied to |0>
void testHadamardOnZero() {
    QuantumCircuit circuit(1);
    circuit.addGate("H", 0, 0);
    QuantumSimulator sim;
    const SimulationResult res = sim.simulate(circuit);

    const double invSqrt2 = 1.0 / std::sqrt(2.0);
    assert(approxComplex(res.finalState[0], Complex(invSqrt2, 0.0)));
    assert(approxComplex(res.finalState[1], Complex(invSqrt2, 0.0)));
    assert(approxEqual(res.probabilities[0], 0.5));
    assert(approxEqual(res.probabilities[1], 0.5));
    assert(res.isNormalized);
    reportPass("2. Hadamard gate applied to |0>");
}

// 3. Pauli-X gate applied to |0>
void testPauliXOnZero() {
    QuantumCircuit circuit(1);
    circuit.addGate("X", 0, 0);
    QuantumSimulator sim;
    const SimulationResult res = sim.simulate(circuit);

    assert(approxComplex(res.finalState[0], Complex(0.0, 0.0)));
    assert(approxComplex(res.finalState[1], Complex(1.0, 0.0)));
    assert(approxEqual(res.probabilities[0], 0.0));
    assert(approxEqual(res.probabilities[1], 1.0));
    reportPass("3. Pauli-X gate applied to |0>");
}

// 4. Pauli-Y and Pauli-Z gates
void testPauliYAndZ() {
    // Z|0> = |0>
    {
        QuantumCircuit circuit(1);
        circuit.addGate("Z", 0, 0);
        QuantumSimulator sim;
        const SimulationResult res = sim.simulate(circuit);
        assert(approxComplex(res.finalState[0], Complex(1.0, 0.0)));
        assert(approxComplex(res.finalState[1], Complex(0.0, 0.0)));
        assert(approxEqual(res.probabilities[0], 1.0));
    }
    // H -> Z -> H |0> = |1> (phase kickback)
    {
        QuantumCircuit circuit(1);
        circuit.addGate("H", 0, 0);
        circuit.addGate("Z", 0, 1);
        circuit.addGate("H", 0, 2);
        QuantumSimulator sim;
        const SimulationResult res = sim.simulate(circuit);
        assert(approxComplex(res.finalState[0], Complex(0.0, 0.0)));
        assert(approxComplex(res.finalState[1], Complex(1.0, 0.0)));
        assert(approxEqual(res.probabilities[1], 1.0));
    }
    // Y|0> = i|1>
    {
        QuantumCircuit circuit(1);
        circuit.addGate("Y", 0, 0);
        QuantumSimulator sim;
        const SimulationResult res = sim.simulate(circuit);
        assert(approxComplex(res.finalState[0], Complex(0.0, 0.0)));
        assert(approxComplex(res.finalState[1], Complex(0.0, 1.0)));
        assert(approxEqual(res.probabilities[1], 1.0));
    }
    reportPass("4. Pauli-Z on |0>, H-Z-H phase flip, and Pauli-Y on |0>");
}

// 5. Two Hadamard gates in sequence (H * H = I)
void testDoubleHadamardIdentity() {
    QuantumCircuit circuit(1);
    circuit.addGate("H", 0, 0);
    circuit.addGate("H", 0, 1);
    QuantumSimulator sim;
    const SimulationResult res = sim.simulate(circuit);

    assert(approxComplex(res.finalState[0], Complex(1.0, 0.0)));
    assert(approxComplex(res.finalState[1], Complex(0.0, 0.0)));
    assert(approxEqual(res.probabilities[0], 1.0));
    assert(approxEqual(res.probabilities[1], 0.0));
    reportPass("5. Two Hadamard gates in sequence return |0>");
}

// 6. CNOT operation (|10> -> |11>, and reversed control q1->q0)
void testCnotOperation() {
    // Control q0 = |1>, Target q1 = |0> -> |11>
    {
        QuantumCircuit circuit(2);
        circuit.addGate("X", 0, 0);
        circuit.addGate("CNOT", 0, 1, 1);
        QuantumSimulator sim;
        const SimulationResult res = sim.simulate(circuit);
        assert(approxEqual(res.probabilities[3], 1.0)); // |11>
        assert(approxComplex(res.finalState[3], Complex(1.0, 0.0)));
    }
    // Control q1 = |1>, Target q0 = |0> -> |11>
    {
        QuantumCircuit circuit(2);
        circuit.addGate("X", 1, 0);
        circuit.addGate("CNOT", 1, 0, 1);
        QuantumSimulator sim;
        const SimulationResult res = sim.simulate(circuit);
        assert(approxEqual(res.probabilities[3], 1.0)); // |11>
    }
    // Control q0 = |0>, Target q1 = |0> -> stays |00>
    {
        QuantumCircuit circuit(2);
        circuit.addGate("CNOT", 0, 1, 0);
        QuantumSimulator sim;
        const SimulationResult res = sim.simulate(circuit);
        assert(approxEqual(res.probabilities[0], 1.0));
    }
    reportPass("6. CNOT gate with standard and reversed control-target");
}

// 7. Bell state generation using H and CNOT
void testBellStateGeneration() {
    QuantumCircuit circuit(2);
    circuit.addGate("H", 0, 0);
    circuit.addGate("CNOT", 0, 1, 1);

    QuantumSimulator sim;
    const SimulationResult res = sim.simulate(circuit);

    const double invSqrt2 = 1.0 / std::sqrt(2.0);
    assert(res.finalState.size() == 4);
    assert(approxComplex(res.finalState[0], Complex(invSqrt2, 0.0))); // |00>
    assert(approxComplex(res.finalState[1], Complex(0.0, 0.0)));      // |01>
    assert(approxComplex(res.finalState[2], Complex(0.0, 0.0)));      // |10>
    assert(approxComplex(res.finalState[3], Complex(invSqrt2, 0.0))); // |11>
    assert(approxEqual(res.probabilities[0], 0.5));
    assert(approxEqual(res.probabilities[1], 0.0));
    assert(approxEqual(res.probabilities[2], 0.0));
    assert(approxEqual(res.probabilities[3], 0.5));
    assert(res.steps.size() == 2);
    reportPass("7. Bell state (|00> + |11>)/sqrt(2) generation");
}

// 8. Multi-qubit probability normalization & 3-qubit GHZ state
void testMultiQubitNormalizationAndGHZ() {
    QuantumCircuit circuit(3);
    circuit.addGate("H", 0, 0);
    circuit.addGate("CNOT", 0, 1, 1);
    circuit.addGate("CNOT", 1, 2, 2);

    QuantumSimulator sim;
    const SimulationResult res = sim.simulate(circuit);

    assert(res.finalState.size() == 8);
    assert(approxEqual(res.normSum, 1.0));
    assert(res.isNormalized);
    assert(approxEqual(res.probabilities[0], 0.5)); // |000>
    assert(approxEqual(res.probabilities[7], 0.5)); // |111>
    for (size_t i = 1; i < 7; ++i) {
        assert(approxEqual(res.probabilities[i], 0.0));
    }
    reportPass("8. 3-qubit GHZ state and probability normalization");
}

// 9. Invalid gate configurations throw appropriate exceptions
void testInvalidGateConfigurations() {
    QuantumCircuit circuit(2);

    // Out-of-bounds qubit
    bool caughtOutOfBounds = false;
    try {
        circuit.addGate("H", 5, 0);
    } catch (const InvalidQubitException&) {
        caughtOutOfBounds = true;
    }
    assert(caughtOutOfBounds);

    // CNOT with same control and target
    bool caughtSameCnot = false;
    try {
        circuit.addGate("CNOT", 1, 1, 0);
    } catch (const InvalidGateException&) {
        caughtSameCnot = true;
    }
    assert(caughtSameCnot);

    // Unsupported gate type
    bool caughtUnknownGate = false;
    try {
        circuit.addGate("TOFFOLI_BAD", 0, 0);
    } catch (const InvalidGateException&) {
        caughtUnknownGate = true;
    }
    assert(caughtUnknownGate);

    // Slot conflict in same column
    bool caughtConflict = false;
    circuit.addGate("H", 0, 0);
    try {
        circuit.addGate("X", 0, 0);
    } catch (const InvalidGateException&) {
        caughtConflict = true;
    }
    assert(caughtConflict);

    // Unnormalized qubit throws NormalizationException
    bool caughtNorm = false;
    try {
        Qubit bad(0, Complex(1.0, 0.0), Complex(1.0, 0.0));
    } catch (const NormalizationException&) {
        caughtNorm = true;
    }
    assert(caughtNorm);

    reportPass("9. Invalid gate & qubit exception handling");
}

// 10. Circuit reset, qubit add/remove, and matrix unitarity verification
void testResetAndMatrixUnitarity() {
    QuantumCircuit circuit(2);
    circuit.addGate("H", 0, 0);
    circuit.addGate("CNOT", 0, 1, 1);
    assert(circuit.getGateCount() == 2);
    assert(circuit.getDepth() == 2);

    circuit.reset();
    assert(circuit.getGateCount() == 0);
    assert(circuit.getDepth() == 0);
    assert(circuit.getNumQubits() == 2);

    QuantumSimulator sim;
    const SimulationResult res = sim.simulate(circuit);
    assert(approxEqual(res.probabilities[0], 1.0));

    // Verify all catalog gates have unitary matrices
    for (const auto& gate : createGateCatalog()) {
        assert(MatrixUtils::isUnitary(gate->getMatrix()));
    }

    // Qubit add/remove
    circuit.addQubit("ancilla");
    assert(circuit.getNumQubits() == 3);
    circuit.removeQubit(2);
    assert(circuit.getNumQubits() == 2);

    reportPass("10. Circuit reset, qubit management, and unitary matrix checks");
}

} // namespace

int main() {
    std::cout << "Running Quantum Lite C++ Simulation Engine Tests...\n";
    std::cout << "---------------------------------------------------\n";

    testInitialSingleQubitState();
    testHadamardOnZero();
    testPauliXOnZero();
    testPauliYAndZ();
    testDoubleHadamardIdentity();
    testCnotOperation();
    testBellStateGeneration();
    testMultiQubitNormalizationAndGHZ();
    testInvalidGateConfigurations();
    testResetAndMatrixUnitarity();

    std::cout << "---------------------------------------------------\n";
    std::cout << "All " << gTestsPassed << "/10 test suites passed successfully!\n";
    return 0;
}
