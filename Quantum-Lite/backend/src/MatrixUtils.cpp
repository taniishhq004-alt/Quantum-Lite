#include "MatrixUtils.h"

#include <cmath>
#include <iomanip>
#include <sstream>

namespace qlite {
namespace MatrixUtils {

ComplexMatrix identityMatrix(size_t dimension) {
    if (dimension == 0) {
        throw QuantumException("Identity matrix dimension must be positive.");
    }
    ComplexMatrix mat(dimension, std::vector<Complex>(dimension, Complex(0.0, 0.0)));
    for (size_t i = 0; i < dimension; ++i) {
        mat[i][i] = Complex(1.0, 0.0);
    }
    return mat;
}

ComplexMatrix multiplyMatrices(const ComplexMatrix& a, const ComplexMatrix& b) {
    if (a.empty() || b.empty() || a[0].size() != b.size()) {
        throw QuantumException("Matrix dimensions are incompatible for multiplication.");
    }
    const size_t rows = a.size();
    const size_t inner = b.size();
    const size_t cols = b[0].size();
    ComplexMatrix result(rows, std::vector<Complex>(cols, Complex(0.0, 0.0)));

    for (size_t i = 0; i < rows; ++i) {
        for (size_t k = 0; k < inner; ++k) {
            const Complex aik = a[i][k];
            for (size_t j = 0; j < cols; ++j) {
                result[i][j] += aik * b[k][j];
            }
        }
    }
    return result;
}

StateVector multiplyMatrixVector(const ComplexMatrix& mat, const StateVector& vec) {
    if (mat.empty() || mat[0].size() != vec.size()) {
        throw QuantumException("Matrix and state vector dimensions do not match.");
    }
    const size_t rows = mat.size();
    const size_t cols = vec.size();
    StateVector result(rows, Complex(0.0, 0.0));

    for (size_t i = 0; i < rows; ++i) {
        Complex sum(0.0, 0.0);
        for (size_t j = 0; j < cols; ++j) {
            sum += mat[i][j] * vec[j];
        }
        result[i] = sum;
    }
    return result;
}

ComplexMatrix tensorProduct(const ComplexMatrix& a, const ComplexMatrix& b) {
    return genericTensorProduct<Complex>(a, b);
}

StateVector tensorProductVectors(const StateVector& a, const StateVector& b) {
    if (a.empty() || b.empty()) {
        throw QuantumException("Cannot compute tensor product of empty state vectors.");
    }
    StateVector result;
    result.reserve(a.size() * b.size());
    for (const Complex& va : a) {
        for (const Complex& vb : b) {
            result.push_back(va * vb);
        }
    }
    return result;
}

bool isUnitary(const ComplexMatrix& mat, double tolerance) {
    if (mat.empty() || mat.size() != mat[0].size()) {
        return false;
    }
    const size_t n = mat.size();
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            Complex dot(0.0, 0.0);
            for (size_t k = 0; k < n; ++k) {
                dot += std::conj(mat[k][i]) * mat[k][j];
            }
            const double expected = (i == j) ? 1.0 : 0.0;
            if (std::abs(dot.real() - expected) > tolerance || std::abs(dot.imag()) > tolerance) {
                return false;
            }
        }
    }
    return true;
}

double normSquaredSum(const StateVector& state) {
    double total = 0.0;
    for (const Complex& amp : state) {
        total += std::norm(amp);
    }
    return total;
}

bool isNormalized(const StateVector& state, double tolerance) {
    if (state.empty()) {
        return false;
    }
    return std::abs(normSquaredSum(state) - 1.0) <= tolerance;
}

std::string basisStateLabel(size_t index, int numQubits) {
    std::string label = "|";
    for (int bit = numQubits - 1; bit >= 0; --bit) {
        label.push_back(((index >> bit) & 1ULL) ? '1' : '0');
    }
    label += "\xE2\x9F\xA9"; // UTF-8 right angle bracket ⟩
    return label;
}

static std::string trimNumber(double val, int precision) {
    if (std::abs(val) < 1e-9) {
        val = 0.0;
    }
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << val;
    std::string s = oss.str();
    const auto dotPos = s.find('.');
    if (dotPos != std::string::npos) {
        while (!s.empty() && s.back() == '0') {
            s.pop_back();
        }
        if (!s.empty() && s.back() == '.') {
            s.pop_back();
        }
    }
    if (s == "-0") {
        s = "0";
    }
    return s;
}

std::string formatComplex(const Complex& c, int precision) {
    const double re = (std::abs(c.real()) < 1e-9) ? 0.0 : c.real();
    const double im = (std::abs(c.imag()) < 1e-9) ? 0.0 : c.imag();

    if (im == 0.0) {
        return trimNumber(re, precision);
    }
    if (re == 0.0) {
        if (std::abs(im - 1.0) < 1e-9) return "i";
        if (std::abs(im + 1.0) < 1e-9) return "-i";
        return trimNumber(im, precision) + "i";
    }

    std::string out = trimNumber(re, precision);
    if (im > 0.0) {
        out += " + ";
        out += (std::abs(im - 1.0) < 1e-9) ? "i" : (trimNumber(im, precision) + "i");
    } else {
        out += " - ";
        const double absIm = std::abs(im);
        out += (std::abs(absIm - 1.0) < 1e-9) ? "i" : (trimNumber(absIm, precision) + "i");
    }
    return out;
}

std::string formatStateDirac(const StateVector& state, int numQubits, double threshold) {
    std::ostringstream oss;
    bool first = true;
    for (size_t i = 0; i < state.size(); ++i) {
        const double mag = std::abs(state[i]);
        if (mag < threshold) {
            continue;
        }
        const std::string coeff = formatComplex(state[i], 4);
        const std::string ket = basisStateLabel(i, numQubits);
        const bool hasSignOrSpace = (coeff.find(' ') != std::string::npos);

        if (!first) {
            if (!hasSignOrSpace && coeff[0] == '-') {
                oss << " - " << coeff.substr(1);
            } else if (hasSignOrSpace) {
                oss << " + (" << coeff << ")";
            } else {
                oss << " + " << coeff;
            }
        } else {
            if (hasSignOrSpace) {
                oss << "(" << coeff << ")";
            } else {
                oss << coeff;
            }
            first = false;
        }
        oss << ket;
    }
    if (first) {
        return "0";
    }
    return oss.str();
}

} // namespace MatrixUtils
} // namespace qlite
