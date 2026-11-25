#include "Qubit.h"

#include <cmath>
#include <sstream>

namespace qlite {

Qubit::Qubit()
    : id_(0), label_("q0"), alpha_(1.0, 0.0), beta_(0.0, 0.0) {}

Qubit::Qubit(int id, const std::string& label)
    : id_(id),
      label_(label.empty() ? ("q" + std::to_string(id)) : label),
      alpha_(1.0, 0.0),
      beta_(0.0, 0.0) {
    if (id < 0) {
        throw InvalidQubitException("Qubit ID cannot be negative.");
    }
}

Qubit::Qubit(int id, Complex alpha, Complex beta, const std::string& label)
    : id_(id),
      label_(label.empty() ? ("q" + std::to_string(id)) : label),
      alpha_(1.0, 0.0),
      beta_(0.0, 0.0) {
    if (id < 0) {
        throw InvalidQubitException("Qubit ID cannot be negative.");
    }
    setState(alpha, beta);
}

Qubit::Qubit(const Qubit& other) = default;

Qubit& Qubit::operator=(const Qubit& other) = default;

Qubit::~Qubit() = default;

void Qubit::initializeZero() {
    alpha_ = Complex(1.0, 0.0);
    beta_ = Complex(0.0, 0.0);
}

void Qubit::initializeOne() {
    alpha_ = Complex(0.0, 0.0);
    beta_ = Complex(1.0, 0.0);
}

void Qubit::setState(Complex alpha, Complex beta) {
    const double normSq = std::norm(alpha) + std::norm(beta);
    if (normSq < 1e-12) {
        throw NormalizationException("Qubit state cannot be the zero vector.");
    }
    if (std::abs(normSq - 1.0) > 1e-6) {
        throw NormalizationException("Qubit amplitudes must satisfy |alpha|^2 + |beta|^2 = 1.");
    }
    // Re-normalize slightly to absorb floating-point rounding near 1.0.
    const double invNorm = 1.0 / std::sqrt(normSq);
    alpha_ = alpha * invNorm;
    beta_ = beta * invNorm;
}

int Qubit::getId() const {
    return id_;
}

const std::string& Qubit::getLabel() const {
    return label_;
}

void Qubit::setLabel(const std::string& label) {
    label_ = label.empty() ? ("q" + std::to_string(id_)) : label;
}

Complex Qubit::getAlpha() const {
    return alpha_;
}

Complex Qubit::getBeta() const {
    return beta_;
}

StateVector Qubit::getStateVector() const {
    return {alpha_, beta_};
}

double Qubit::getProbabilityZero() const {
    return std::norm(alpha_);
}

double Qubit::getProbabilityOne() const {
    return std::norm(beta_);
}

bool Qubit::isNormalized(double tolerance) const {
    const double normSq = std::norm(alpha_) + std::norm(beta_);
    return std::abs(normSq - 1.0) <= tolerance;
}

std::string Qubit::toString() const {
    std::ostringstream oss;
    oss << label_ << ": (" << MatrixUtils::formatComplex(alpha_) << ")|0\xE2\x9F\xA9 + ("
        << MatrixUtils::formatComplex(beta_) << ")|1\xE2\x9F\xA9";
    return oss.str();
}

} // namespace qlite
