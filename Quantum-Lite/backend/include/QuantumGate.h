#pragma once

#include "MatrixUtils.h"
#include <memory>
#include <string>
#include <vector>

namespace qlite {

/**
 * @brief Abstract base class defining the common interface for all quantum gates.
 *
 * Demonstrates abstraction, inheritance, and runtime polymorphism.
 */
class QuantumGate {
protected:
    int column_;
    std::vector<int> targetQubits_;

    QuantumGate(int column, std::vector<int> targetQubits)
        : column_(column), targetQubits_(std::move(targetQubits)) {}

public:
    virtual ~QuantumGate() = default;

    virtual std::string getName() const = 0;
    virtual std::string getSymbol() const = 0;
    virtual std::string getDescription() const = 0;
    virtual std::string getFunctionSummary() const = 0;
    virtual std::string getExampleOperation() const = 0;
    virtual int getNumQubits() const = 0;
    virtual ComplexMatrix getMatrix() const = 0;

    /**
     * @brief Polymorphically applies this gate to the given multi-qubit state vector.
     */
    virtual void apply(StateVector& state, int totalQubits) const = 0;

    /**
     * @brief Virtual copy constructor for deep cloning gates inside circuits.
     */
    virtual std::unique_ptr<QuantumGate> clone() const = 0;

    virtual bool isControlled() const { return false; }
    virtual int getControlQubit() const { return -1; }
    virtual int getPrimaryTargetQubit() const {
        return targetQubits_.empty() ? -1 : targetQubits_.front();
    }

    int getColumn() const { return column_; }
    void setColumn(int column) { column_ = column; }
    const std::vector<int>& getTargetQubits() const { return targetQubits_; }
};

} // namespace qlite
