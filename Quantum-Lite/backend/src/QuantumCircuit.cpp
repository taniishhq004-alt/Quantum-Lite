#include "QuantumCircuit.h"

#include <algorithm>
#include <set>
#include <sstream>
#include <utility>

namespace qlite {

QuantumCircuit::QuantumCircuit(int numQubits) {
    setNumQubits(numQubits);
}

QuantumCircuit::QuantumCircuit(const QuantumCircuit& other)
    : qubits_(other.qubits_) {
    gates_.reserve(other.gates_.size());
    for (const auto& gate : other.gates_) {
        gates_.push_back(gate->clone());
    }
}

QuantumCircuit& QuantumCircuit::operator=(const QuantumCircuit& other) {
    if (this == &other) {
        return *this;
    }
    qubits_ = other.qubits_;
    gates_.clear();
    gates_.reserve(other.gates_.size());
    for (const auto& gate : other.gates_) {
        gates_.push_back(gate->clone());
    }
    return *this;
}

void QuantumCircuit::addQubit() {
    if (static_cast<int>(qubits_.size()) >= MAX_QUBITS) {
        throw InvalidQubitException("Maximum supported qubit limit reached (" +
                                    std::to_string(MAX_QUBITS) + ").");
    }
    const int newId = static_cast<int>(qubits_.size());
    qubits_.emplace_back(newId);
}

void QuantumCircuit::addQubit(const std::string& label) {
    if (static_cast<int>(qubits_.size()) >= MAX_QUBITS) {
        throw InvalidQubitException("Maximum supported qubit limit reached (" +
                                    std::to_string(MAX_QUBITS) + ").");
    }
    const int newId = static_cast<int>(qubits_.size());
    qubits_.emplace_back(newId, label);
}

void QuantumCircuit::removeQubit() {
    if (static_cast<int>(qubits_.size()) <= MIN_QUBITS) {
        throw InvalidQubitException("A circuit must contain at least 1 qubit.");
    }
    removeQubit(static_cast<int>(qubits_.size()) - 1);
}

void QuantumCircuit::removeQubit(int qubitIndex) {
    const int count = static_cast<int>(qubits_.size());
    if (count <= MIN_QUBITS) {
        throw InvalidQubitException("A circuit must contain at least 1 qubit.");
    }
    if (qubitIndex < 0 || qubitIndex >= count) {
        throw InvalidQubitException("Qubit index out of range for removal.");
    }

    qubits_.erase(qubits_.begin() + qubitIndex);
    for (size_t i = 0; i < qubits_.size(); ++i) {
        qubits_[i].setLabel("q" + std::to_string(i));
    }

    // Remove gates referencing the removed qubit and shift higher qubit indices down.
    std::vector<std::unique_ptr<QuantumGate>> updatedGates;
    for (const auto& gate : gates_) {
        const auto& targets = gate->getTargetQubits();
        bool referencesRemoved = false;
        std::vector<int> shiftedTargets;
        shiftedTargets.reserve(targets.size());
        for (int q : targets) {
            if (q == qubitIndex) {
                referencesRemoved = true;
                break;
            }
            shiftedTargets.push_back(q > qubitIndex ? q - 1 : q);
        }
        if (!referencesRemoved) {
            updatedGates.push_back(createGate(gate->getSymbol(), shiftedTargets, gate->getColumn()));
        }
    }
    gates_ = std::move(updatedGates);
}

void QuantumCircuit::setNumQubits(int numQubits) {
    if (numQubits < MIN_QUBITS || numQubits > MAX_QUBITS) {
        throw InvalidQubitException("Number of qubits must be between " +
                                    std::to_string(MIN_QUBITS) + " and " +
                                    std::to_string(MAX_QUBITS) + ".");
    }
    qubits_.clear();
    qubits_.reserve(static_cast<size_t>(numQubits));
    for (int i = 0; i < numQubits; ++i) {
        qubits_.emplace_back(i);
    }
    gates_.clear();
}

void QuantumCircuit::addGate(std::unique_ptr<QuantumGate> gate) {
    if (!gate) {
        throw InvalidGateException("Cannot add a null quantum gate.");
    }

    const int n = getNumQubits();
    for (int q : gate->getTargetQubits()) {
        if (q < 0 || q >= n) {
            throw InvalidQubitException("Gate references qubit " + std::to_string(q) +
                                        " which is outside [0, " + std::to_string(n - 1) + "].");
        }
    }

    if (gate->getColumn() < 0) {
        throw InvalidGateException("Gate column position cannot be negative.");
    }

    // Check slot conflict at the same column.
    const int col = gate->getColumn();
    for (const auto& existing : gates_) {
        if (existing->getColumn() != col) {
            continue;
        }
        for (int qNew : gate->getTargetQubits()) {
            for (int qOld : existing->getTargetQubits()) {
                if (qNew == qOld) {
                    throw InvalidGateException(
                        "Slot conflict: qubit q" + std::to_string(qNew) +
                        " already has a gate at column " + std::to_string(col) + ".");
                }
            }
        }
    }

    gates_.push_back(std::move(gate));
}

void QuantumCircuit::addGate(const std::string& symbol, int targetQubit, int column) {
    addGate(createGate(symbol, {targetQubit}, column));
}

void QuantumCircuit::addGate(const std::string& symbol, int controlQubit, int targetQubit, int column) {
    addGate(createGate(symbol, {controlQubit, targetQubit}, column));
}

bool QuantumCircuit::removeGate(int column, int qubit) {
    for (auto it = gates_.begin(); it != gates_.end(); ++it) {
        if ((*it)->getColumn() != column) {
            continue;
        }
        const auto& targets = (*it)->getTargetQubits();
        if (std::find(targets.begin(), targets.end(), qubit) != targets.end()) {
            gates_.erase(it);
            return true;
        }
    }
    return false;
}

void QuantumCircuit::clearGates() {
    gates_.clear();
}

void QuantumCircuit::reset() {
    gates_.clear();
    for (auto& qubit : qubits_) {
        qubit.initializeZero();
    }
}

void QuantumCircuit::reset(int numQubits) {
    setNumQubits(numQubits);
}

void QuantumCircuit::validate() const {
    const int n = getNumQubits();
    if (n < MIN_QUBITS || n > MAX_QUBITS) {
        throw InvalidQubitException("Circuit qubit count out of valid range.");
    }
    for (const auto& qubit : qubits_) {
        if (!qubit.isNormalized()) {
            throw NormalizationException("Initial qubit state for " + qubit.getLabel() +
                                         " is not normalized.");
        }
    }

    std::set<std::pair<int, int>> occupiedSlots;
    for (const auto& gate : gates_) {
        if (!gate) {
            throw InvalidGateException("Circuit contains a null gate entry.");
        }
        if (gate->getColumn() < 0) {
            throw InvalidGateException("Gate column cannot be negative.");
        }
        const auto& targets = gate->getTargetQubits();
        if (static_cast<int>(targets.size()) != gate->getNumQubits()) {
            throw InvalidGateException("Gate qubit count mismatch for " + gate->getSymbol() + ".");
        }
        std::set<int> uniqueTargets;
        for (int q : targets) {
            if (q < 0 || q >= n) {
                throw InvalidQubitException("Gate qubit index out of bounds.");
            }
            if (!uniqueTargets.insert(q).second) {
                throw InvalidGateException("Gate cannot target the same qubit twice.");
            }
            if (!occupiedSlots.emplace(gate->getColumn(), q).second) {
                throw InvalidGateException("Multiple gates occupy the same column and qubit.");
            }
        }
    }
}

int QuantumCircuit::getNumQubits() const {
    return static_cast<int>(qubits_.size());
}

int QuantumCircuit::getDepth() const {
    if (gates_.empty()) {
        return 0;
    }
    int maxCol = 0;
    for (const auto& gate : gates_) {
        if (gate->getColumn() > maxCol) {
            maxCol = gate->getColumn();
        }
    }
    return maxCol + 1;
}

size_t QuantumCircuit::getGateCount() const {
    return gates_.size();
}

std::vector<const QuantumGate*> QuantumCircuit::getOrderedGates() const {
    struct GateEntry {
        const QuantumGate* gate;
        size_t order;
    };
    std::vector<GateEntry> entries;
    entries.reserve(gates_.size());
    for (size_t i = 0; i < gates_.size(); ++i) {
        entries.push_back({gates_[i].get(), i});
    }
    std::stable_sort(entries.begin(), entries.end(),
                     [](const GateEntry& a, const GateEntry& b) {
                         if (a.gate->getColumn() != b.gate->getColumn()) {
                             return a.gate->getColumn() < b.gate->getColumn();
                         }
                         return a.order < b.order;
                     });

    std::vector<const QuantumGate*> ordered;
    ordered.reserve(entries.size());
    for (const auto& entry : entries) {
        ordered.push_back(entry.gate);
    }
    return ordered;
}

const std::vector<Qubit>& QuantumCircuit::getQubits() const {
    return qubits_;
}

std::vector<Qubit>& QuantumCircuit::getQubitsMutable() {
    return qubits_;
}

std::string QuantumCircuit::getSummary() const {
    std::ostringstream oss;
    oss << getNumQubits() << " qubit(s), " << getGateCount()
        << " gate(s), depth " << getDepth();
    return oss.str();
}

} // namespace qlite
