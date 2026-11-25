#pragma once

#include "MatrixUtils.h"
#include <string>

namespace qlite {

/**
 * @brief Represents a single quantum bit (qubit) in state alpha|0> + beta|1>.
 *
 * Demonstrates encapsulation, constructor overloading, copy semantics, and
 * quantum state validation.
 */
class Qubit {
private:
    int id_;
    std::string label_;
    Complex alpha_; // Amplitude of |0>
    Complex beta_;  // Amplitude of |1>

public:
    Qubit();
    explicit Qubit(int id, const std::string& label = "");
    Qubit(int id, Complex alpha, Complex beta, const std::string& label = "");
    Qubit(const Qubit& other);
    Qubit& operator=(const Qubit& other);
    ~Qubit();

    void initializeZero();
    void initializeOne();
    void setState(Complex alpha, Complex beta);

    int getId() const;
    const std::string& getLabel() const;
    void setLabel(const std::string& label);

    Complex getAlpha() const;
    Complex getBeta() const;
    StateVector getStateVector() const;

    double getProbabilityZero() const;
    double getProbabilityOne() const;
    bool isNormalized(double tolerance = 1e-9) const;

    std::string toString() const;
};

} // namespace qlite
