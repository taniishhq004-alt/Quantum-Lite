#pragma once

#include <complex>
#include <stdexcept>
#include <string>
#include <vector>

namespace qlite {

using Complex = std::complex<double>;
using StateVector = std::vector<Complex>;
using ComplexMatrix = std::vector<std::vector<Complex>>;

/**
 * @brief Base exception class for all Quantum Lite simulation errors.
 */
class QuantumException : public std::runtime_error {
public:
    explicit QuantumException(const std::string& message)
        : std::runtime_error(message) {}
};

/**
 * @brief Thrown when a qubit index or qubit count is invalid.
 */
class InvalidQubitException : public QuantumException {
public:
    explicit InvalidQubitException(const std::string& message)
        : QuantumException(message) {}
};

/**
 * @brief Thrown when a gate type, matrix, or placement is invalid.
 */
class InvalidGateException : public QuantumException {
public:
    explicit InvalidGateException(const std::string& message)
        : QuantumException(message) {}
};

/**
 * @brief Thrown when a quantum state fails normalization checks.
 */
class NormalizationException : public QuantumException {
public:
    explicit NormalizationException(const std::string& message)
        : QuantumException(message) {}
};

/**
 * @brief Linear algebra, tensor product, and state formatting utilities.
 */
namespace MatrixUtils {

    /**
     * @brief Generic template helper to compute the tensor (Kronecker) product of two matrices.
     */
    template <typename T>
    std::vector<std::vector<T>> genericTensorProduct(
        const std::vector<std::vector<T>>& a,
        const std::vector<std::vector<T>>& b) {
        if (a.empty() || b.empty() || a[0].empty() || b[0].empty()) {
            throw QuantumException("Cannot compute tensor product of empty matrices.");
        }
        const size_t aRows = a.size();
        const size_t aCols = a[0].size();
        const size_t bRows = b.size();
        const size_t bCols = b[0].size();

        std::vector<std::vector<T>> result(
            aRows * bRows, std::vector<T>(aCols * bCols, T{}));

        for (size_t i = 0; i < aRows; ++i) {
            for (size_t j = 0; j < aCols; ++j) {
                for (size_t k = 0; k < bRows; ++k) {
                    for (size_t l = 0; l < bCols; ++l) {
                        result[i * bRows + k][j * bCols + l] = a[i][j] * b[k][l];
                    }
                }
            }
        }
        return result;
    }

    ComplexMatrix identityMatrix(size_t dimension);
    ComplexMatrix multiplyMatrices(const ComplexMatrix& a, const ComplexMatrix& b);
    StateVector multiplyMatrixVector(const ComplexMatrix& mat, const StateVector& vec);
    ComplexMatrix tensorProduct(const ComplexMatrix& a, const ComplexMatrix& b);
    StateVector tensorProductVectors(const StateVector& a, const StateVector& b);

    bool isUnitary(const ComplexMatrix& mat, double tolerance = 1e-9);
    double normSquaredSum(const StateVector& state);
    bool isNormalized(const StateVector& state, double tolerance = 1e-9);

    std::string basisStateLabel(size_t index, int numQubits);
    std::string formatComplex(const Complex& c, int precision = 4);
    std::string formatStateDirac(const StateVector& state, int numQubits, double threshold = 1e-6);

} // namespace MatrixUtils

} // namespace qlite
