#include "Gates.h"

#include <algorithm>
#include <cctype>
#include <cmath>

namespace qlite {

namespace {
constexpr double kInvSqrt2 = 0.70710678118654752440;
} // namespace

// ---------------------------------------------------------------------------
// SingleQubitGate
// ---------------------------------------------------------------------------

SingleQubitGate::SingleQubitGate(int targetQubit, int column)
    : QuantumGate(column, {targetQubit}) {
    if (targetQubit < 0) {
        throw InvalidQubitException("Target qubit index cannot be negative.");
    }
    if (column < 0) {
        throw InvalidGateException("Gate column index cannot be negative.");
    }
}

void SingleQubitGate::apply(StateVector& state, int totalQubits) const {
    const int target = targetQubits_.front();
    if (target < 0 || target >= totalQubits) {
        throw InvalidQubitException("Target qubit index out of bounds.");
    }
    const size_t expectedSize = 1ULL << totalQubits;
    if (state.size() != expectedSize) {
        throw QuantumException("State vector size does not match qubit count.");
    }

    const ComplexMatrix mat = getMatrix();
    const int bitShift = totalQubits - 1 - target; // MSB-first: q0 is highest bit
    const size_t stepMask = 1ULL << bitShift;

    for (size_t i = 0; i < expectedSize; ++i) {
        if ((i & stepMask) == 0) {
            const size_t idx0 = i;
            const size_t idx1 = i | stepMask;
            const Complex a0 = state[idx0];
            const Complex a1 = state[idx1];
            state[idx0] = mat[0][0] * a0 + mat[0][1] * a1;
            state[idx1] = mat[1][0] * a0 + mat[1][1] * a1;
        }
    }
}

// ---------------------------------------------------------------------------
// HadamardGate
// ---------------------------------------------------------------------------

HadamardGate::HadamardGate(int targetQubit, int column)
    : SingleQubitGate(targetQubit, column) {}

std::string HadamardGate::getName() const { return "Hadamard Gate"; }
std::string HadamardGate::getSymbol() const { return "H"; }
std::string HadamardGate::getDescription() const {
    return "Places a basis state into an equal quantum superposition.";
}
std::string HadamardGate::getFunctionSummary() const {
    return "Maps |0> to (|0> + |1>)/sqrt(2) and |1> to (|0> - |1>)/sqrt(2).";
}
std::string HadamardGate::getExampleOperation() const {
    return "H|0> = 0.7071|0> + 0.7071|1> (50% |0>, 50% |1>)";
}
ComplexMatrix HadamardGate::getMatrix() const {
    return {
        {Complex(kInvSqrt2, 0.0), Complex(kInvSqrt2, 0.0)},
        {Complex(kInvSqrt2, 0.0), Complex(-kInvSqrt2, 0.0)}
    };
}
std::unique_ptr<QuantumGate> HadamardGate::clone() const {
    return std::make_unique<HadamardGate>(targetQubits_[0], column_);
}

// ---------------------------------------------------------------------------
// PauliXGate
// ---------------------------------------------------------------------------

PauliXGate::PauliXGate(int targetQubit, int column)
    : SingleQubitGate(targetQubit, column) {}

std::string PauliXGate::getName() const { return "Pauli-X Gate"; }
std::string PauliXGate::getSymbol() const { return "X"; }
std::string PauliXGate::getDescription() const {
    return "Quantum equivalent of the classical NOT gate (bit flip).";
}
std::string PauliXGate::getFunctionSummary() const {
    return "Swaps probability amplitudes of |0> and |1> (pi rotation around X-axis).";
}
std::string PauliXGate::getExampleOperation() const {
    return "X|0> = |1> (100% |1>), X|1> = |0>";
}
ComplexMatrix PauliXGate::getMatrix() const {
    return {
        {Complex(0.0, 0.0), Complex(1.0, 0.0)},
        {Complex(1.0, 0.0), Complex(0.0, 0.0)}
    };
}
std::unique_ptr<QuantumGate> PauliXGate::clone() const {
    return std::make_unique<PauliXGate>(targetQubits_[0], column_);
}

// ---------------------------------------------------------------------------
// PauliYGate
// ---------------------------------------------------------------------------

PauliYGate::PauliYGate(int targetQubit, int column)
    : SingleQubitGate(targetQubit, column) {}

std::string PauliYGate::getName() const { return "Pauli-Y Gate"; }
std::string PauliYGate::getSymbol() const { return "Y"; }
std::string PauliYGate::getDescription() const {
    return "Combines a bit flip and a relative phase shift with imaginary amplitudes.";
}
std::string PauliYGate::getFunctionSummary() const {
    return "Maps |0> to i|1> and |1> to -i|0> (pi rotation around Y-axis).";
}
std::string PauliYGate::getExampleOperation() const {
    return "Y|0> = i|1> (100% |1>), Y|1> = -i|0>";
}
ComplexMatrix PauliYGate::getMatrix() const {
    return {
        {Complex(0.0, 0.0), Complex(0.0, -1.0)},
        {Complex(0.0, 1.0), Complex(0.0, 0.0)}
    };
}
std::unique_ptr<QuantumGate> PauliYGate::clone() const {
    return std::make_unique<PauliYGate>(targetQubits_[0], column_);
}

// ---------------------------------------------------------------------------
// PauliZGate
// ---------------------------------------------------------------------------

PauliZGate::PauliZGate(int targetQubit, int column)
    : SingleQubitGate(targetQubit, column) {}

std::string PauliZGate::getName() const { return "Pauli-Z Gate"; }
std::string PauliZGate::getSymbol() const { return "Z"; }
std::string PauliZGate::getDescription() const {
    return "Applies a phase flip to the |1> state while leaving |0> unchanged.";
}
std::string PauliZGate::getFunctionSummary() const {
    return "Maps |0> to |0> and |1> to -|1> (pi rotation around Z-axis).";
}
std::string PauliZGate::getExampleOperation() const {
    return "Z(|0> + |1>)/sqrt(2) = (|0> - |1>)/sqrt(2)";
}
ComplexMatrix PauliZGate::getMatrix() const {
    return {
        {Complex(1.0, 0.0), Complex(0.0, 0.0)},
        {Complex(0.0, 0.0), Complex(-1.0, 0.0)}
    };
}
std::unique_ptr<QuantumGate> PauliZGate::clone() const {
    return std::make_unique<PauliZGate>(targetQubits_[0], column_);
}

// ---------------------------------------------------------------------------
// CNOTGate
// ---------------------------------------------------------------------------

CNOTGate::CNOTGate(int controlQubit, int targetQubit, int column)
    : QuantumGate(column, {controlQubit, targetQubit}) {
    if (controlQubit < 0 || targetQubit < 0) {
        throw InvalidQubitException("CNOT qubit indices cannot be negative.");
    }
    if (controlQubit == targetQubit) {
        throw InvalidGateException("CNOT control and target qubits must be distinct.");
    }
    if (column < 0) {
        throw InvalidGateException("Gate column index cannot be negative.");
    }
}

std::string CNOTGate::getName() const { return "Controlled-NOT Gate"; }
std::string CNOTGate::getSymbol() const { return "CNOT"; }
std::string CNOTGate::getDescription() const {
    return "Two-qubit entangling gate that flips the target qubit when the control qubit is |1>.";
}
std::string CNOTGate::getFunctionSummary() const {
    return "Maps |c, t> to |c, t XOR c>. Essential for creating Bell entanglement.";
}
std::string CNOTGate::getExampleOperation() const {
    return "CNOT((|00> + |10>)/sqrt(2)) = (|00> + |11>)/sqrt(2) (Bell State)";
}
ComplexMatrix CNOTGate::getMatrix() const {
    return {
        {Complex(1.0, 0.0), Complex(0.0, 0.0), Complex(0.0, 0.0), Complex(0.0, 0.0)},
        {Complex(0.0, 0.0), Complex(1.0, 0.0), Complex(0.0, 0.0), Complex(0.0, 0.0)},
        {Complex(0.0, 0.0), Complex(0.0, 0.0), Complex(0.0, 0.0), Complex(1.0, 0.0)},
        {Complex(0.0, 0.0), Complex(0.0, 0.0), Complex(1.0, 0.0), Complex(0.0, 0.0)}
    };
}

void CNOTGate::apply(StateVector& state, int totalQubits) const {
    const int control = targetQubits_[0];
    const int target = targetQubits_[1];

    if (control < 0 || control >= totalQubits || target < 0 || target >= totalQubits) {
        throw InvalidQubitException("CNOT qubit index out of bounds.");
    }
    if (control == target) {
        throw InvalidGateException("CNOT control and target qubits cannot be the same.");
    }

    const size_t expectedSize = 1ULL << totalQubits;
    if (state.size() != expectedSize) {
        throw QuantumException("State vector size does not match qubit count.");
    }

    const size_t controlMask = 1ULL << (totalQubits - 1 - control);
    const size_t targetMask = 1ULL << (totalQubits - 1 - target);

    for (size_t i = 0; i < expectedSize; ++i) {
        if ((i & controlMask) != 0 && (i & targetMask) == 0) {
            std::swap(state[i], state[i | targetMask]);
        }
    }
}

std::unique_ptr<QuantumGate> CNOTGate::clone() const {
    return std::make_unique<CNOTGate>(targetQubits_[0], targetQubits_[1], column_);
}

// ---------------------------------------------------------------------------
// Gate Factory & Catalog
// ---------------------------------------------------------------------------

std::unique_ptr<QuantumGate> createGate(
    const std::string& symbol,
    const std::vector<int>& qubits,
    int column) {
    std::string upper = symbol;
    std::transform(upper.begin(), upper.end(), upper.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::toupper(ch)); });

    if (upper == "H" || upper == "HADAMARD") {
        if (qubits.size() != 1) {
            throw InvalidGateException("Hadamard gate requires exactly 1 target qubit.");
        }
        return std::make_unique<HadamardGate>(qubits[0], column);
    }
    if (upper == "X" || upper == "PAULI-X" || upper == "NOT") {
        if (qubits.size() != 1) {
            throw InvalidGateException("Pauli-X gate requires exactly 1 target qubit.");
        }
        return std::make_unique<PauliXGate>(qubits[0], column);
    }
    if (upper == "Y" || upper == "PAULI-Y") {
        if (qubits.size() != 1) {
            throw InvalidGateException("Pauli-Y gate requires exactly 1 target qubit.");
        }
        return std::make_unique<PauliYGate>(qubits[0], column);
    }
    if (upper == "Z" || upper == "PAULI-Z") {
        if (qubits.size() != 1) {
            throw InvalidGateException("Pauli-Z gate requires exactly 1 target qubit.");
        }
        return std::make_unique<PauliZGate>(qubits[0], column);
    }
    if (upper == "CNOT" || upper == "CX") {
        if (qubits.size() != 2) {
            throw InvalidGateException("CNOT gate requires exactly 2 qubits (control, target).");
        }
        return std::make_unique<CNOTGate>(qubits[0], qubits[1], column);
    }

    throw InvalidGateException("Unsupported quantum gate type: " + symbol);
}

std::vector<std::unique_ptr<QuantumGate>> createGateCatalog() {
    std::vector<std::unique_ptr<QuantumGate>> catalog;
    catalog.push_back(std::make_unique<HadamardGate>(0, 0));
    catalog.push_back(std::make_unique<PauliXGate>(0, 0));
    catalog.push_back(std::make_unique<PauliYGate>(0, 0));
    catalog.push_back(std::make_unique<PauliZGate>(0, 0));
    catalog.push_back(std::make_unique<CNOTGate>(0, 1, 0));
    return catalog;
}

} // namespace qlite
