#include "ApiServer.h"

#include "crow_all.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

namespace fs = std::filesystem;

namespace qlite {

namespace {

std::string mimeTypeForExtension(const std::string& ext) {
    if (ext == ".html") return "text/html; charset=utf-8";
    if (ext == ".css") return "text/css; charset=utf-8";
    if (ext == ".js") return "application/javascript; charset=utf-8";
    if (ext == ".json") return "application/json; charset=utf-8";
    if (ext == ".svg") return "image/svg+xml";
    if (ext == ".png") return "image/png";
    if (ext == ".ico") return "image/x-icon";
    return "application/octet-stream";
}

crow::response jsonError(int statusCode, const std::string& error, const std::string& details = "") {
    crow::json::wvalue body;
    body["success"] = false;
    body["error"] = error;
    if (!details.empty()) {
        body["details"] = details;
    }
    crow::response res(statusCode, body);
    res.set_header("Content-Type", "application/json; charset=utf-8");
    return res;
}

crow::json::wvalue complexToJson(const Complex& c) {
    crow::json::wvalue obj;
    const double re = (std::abs(c.real()) < 1e-12) ? 0.0 : c.real();
    const double im = (std::abs(c.imag()) < 1e-12) ? 0.0 : c.imag();
    obj["real"] = re;
    obj["imag"] = im;
    obj["magnitude"] = std::abs(c);
    obj["phase"] = std::atan2(im, re);
    obj["probability"] = std::norm(c);
    obj["formatted"] = MatrixUtils::formatComplex(c, 4);
    return obj;
}

crow::json::wvalue matrixToJson(const ComplexMatrix& mat) {
    std::vector<crow::json::wvalue> rows;
    rows.reserve(mat.size());
    for (const auto& r : mat) {
        std::vector<crow::json::wvalue> cols;
        cols.reserve(r.size());
        for (const auto& cell : r) {
            crow::json::wvalue entry;
            entry["real"] = (std::abs(cell.real()) < 1e-12) ? 0.0 : cell.real();
            entry["imag"] = (std::abs(cell.imag()) < 1e-12) ? 0.0 : cell.imag();
            entry["formatted"] = MatrixUtils::formatComplex(cell, 4);
            cols.push_back(std::move(entry));
        }
        rows.push_back(crow::json::wvalue::list(std::move(cols)));
    }
    return crow::json::wvalue::list(std::move(rows));
}

QuantumCircuit buildCircuitFromJson(const crow::json::rvalue& body) {
    if (!body || body.t() != crow::json::type::Object) {
        throw InvalidGateException("Request body must be a valid JSON object.");
    }

    int numQubits = 2;
    if (body.has("numQubits")) {
        numQubits = static_cast<int>(body["numQubits"].i());
    }

    QuantumCircuit circuit(numQubits);

    if (body.has("initialStates") && body["initialStates"].t() == crow::json::type::List) {
        const auto& initList = body["initialStates"];
        auto& qubits = circuit.getQubitsMutable();
        for (size_t i = 0; i < initList.size() && i < qubits.size(); ++i) {
            const std::string st = std::string(initList[i].s());
            if (st == "1" || st == "|1>") {
                qubits[i].initializeOne();
            } else {
                qubits[i].initializeZero();
            }
        }
    }

    if (body.has("gates")) {
        if (body["gates"].t() != crow::json::type::List) {
            throw InvalidGateException("'gates' field must be a JSON array.");
        }
        for (const auto& g : body["gates"]) {
            if (!g.has("type")) {
                throw InvalidGateException("Each gate entry must specify a 'type'.");
            }
            const std::string type = std::string(g["type"].s());
            const int column = g.has("column") ? static_cast<int>(g["column"].i()) : 0;

            std::vector<int> qubits;
            if (g.has("qubits") && g["qubits"].t() == crow::json::type::List) {
                for (const auto& q : g["qubits"]) {
                    qubits.push_back(static_cast<int>(q.i()));
                }
            } else if (g.has("control") && g.has("target")) {
                qubits.push_back(static_cast<int>(g["control"].i()));
                qubits.push_back(static_cast<int>(g["target"].i()));
            } else if (g.has("target")) {
                qubits.push_back(static_cast<int>(g["target"].i()));
            } else {
                throw InvalidGateException("Gate '" + type + "' must specify 'qubits' or 'target'.");
            }

            circuit.addGate(createGate(type, qubits, column));
        }
    }

    circuit.validate();
    return circuit;
}

crow::json::wvalue serializeSimulationResult(const SimulationResult& sim,
                                             const QuantumCircuit& circuit) {
    crow::json::wvalue res;
    res["success"] = true;
    res["numQubits"] = sim.numQubits;
    res["circuitDepth"] = sim.circuitDepth;
    res["gateCount"] = static_cast<int>(sim.gateCount);
    res["summary"] = circuit.getSummary();
    res["initialDirac"] = sim.initialDirac;
    res["finalDirac"] = sim.finalDirac;
    res["normSum"] = sim.normSum;
    res["isNormalized"] = sim.isNormalized;

    std::vector<crow::json::wvalue> basisJson;
    std::vector<crow::json::wvalue> initialJson;
    std::vector<crow::json::wvalue> finalJson;
    std::vector<crow::json::wvalue> probsJson;

    const size_t dim = sim.finalState.size();
    basisJson.reserve(dim);
    initialJson.reserve(dim);
    finalJson.reserve(dim);
    probsJson.reserve(dim);

    for (size_t i = 0; i < dim; ++i) {
        basisJson.emplace_back(sim.basisStates[i]);
        initialJson.push_back(complexToJson(sim.initialState[i]));
        finalJson.push_back(complexToJson(sim.finalState[i]));
        probsJson.emplace_back(sim.probabilities[i]);
    }

    res["basisStates"] = crow::json::wvalue::list(std::move(basisJson));
    res["initialState"] = crow::json::wvalue::list(std::move(initialJson));
    res["finalState"] = crow::json::wvalue::list(std::move(finalJson));
    res["probabilities"] = crow::json::wvalue::list(std::move(probsJson));

    std::vector<crow::json::wvalue> stepsJson;
    stepsJson.reserve(sim.steps.size());
    for (const auto& step : sim.steps) {
        crow::json::wvalue sObj;
        sObj["stepIndex"] = step.stepIndex;
        sObj["column"] = step.column;
        sObj["gateSymbol"] = step.gateSymbol;
        sObj["gateName"] = step.gateName;
        sObj["diracAfter"] = step.diracAfter;

        std::vector<crow::json::wvalue> qList;
        for (int q : step.qubits) {
            qList.emplace_back(q);
        }
        sObj["qubits"] = crow::json::wvalue::list(std::move(qList));

        std::vector<crow::json::wvalue> stepProbs;
        for (double p : step.probabilitiesAfter) {
            stepProbs.emplace_back(p);
        }
        sObj["probabilitiesAfter"] = crow::json::wvalue::list(std::move(stepProbs));

        stepsJson.push_back(std::move(sObj));
    }
    res["steps"] = crow::json::wvalue::list(std::move(stepsJson));
    return res;
}

crow::response serveStaticFile(const fs::path& baseDir, const std::string& relativePath) {
    if (baseDir.empty()) {
        return crow::response(404, "Frontend directory not found.");
    }
    if (relativePath.find("..") != std::string::npos) {
        return crow::response(403, "Forbidden path.");
    }

    fs::path filePath = baseDir / relativePath;
    std::error_code ec;
    if (!fs::exists(filePath, ec) || !fs::is_regular_file(filePath, ec)) {
        return crow::response(404, "File not found.");
    }

    std::ifstream ifs(filePath, std::ios::binary);
    if (!ifs) {
        return crow::response(500, "Failed to open static file.");
    }
    std::ostringstream buffer;
    buffer << ifs.rdbuf();

    crow::response res(200, buffer.str());
    res.set_header("Content-Type", mimeTypeForExtension(filePath.extension().string()));
    return res;
}

} // namespace

std::string ApiServer::resolveFrontendDirectory(const std::string& hint) {
    std::vector<fs::path> candidates;
    if (!hint.empty()) {
        candidates.emplace_back(hint);
    }
    candidates.emplace_back("frontend");
    candidates.emplace_back("../frontend");
    candidates.emplace_back("../../frontend");
    candidates.emplace_back("../../../frontend");

    std::error_code ec;
    for (const auto& candidate : candidates) {
        if (fs::exists(candidate / "index.html", ec)) {
            return fs::weakly_canonical(candidate, ec).string();
        }
    }
    return "";
}

ApiServer::ApiServer(int port, const std::string& frontendDir)
    : port_(port),
      frontendDir_(resolveFrontendDirectory(frontendDir)),
      activeCircuit_(2) {}

void ApiServer::run() {
    crow::App<crow::CORSHandler> app;

    auto& cors = app.get_middleware<crow::CORSHandler>();
    cors.global()
        .origin("*")
        .methods("GET"_method, "POST"_method, "OPTIONS"_method)
        .headers("Content-Type");

    // Health check
    CROW_ROUTE(app, "/api/health").methods("GET"_method)([]() {
        crow::json::wvalue res;
        res["success"] = true;
        res["status"] = "ok";
        res["service"] = "Quantum Lite C++ Simulation Engine";
        res["version"] = "1.0.0";
        res["cppStandard"] = static_cast<long>(__cplusplus);
        return res;
    });

    // Gate catalog
    CROW_ROUTE(app, "/api/gates").methods("GET"_method)([]() {
        const auto catalog = createGateCatalog();
        std::vector<crow::json::wvalue> gatesList;
        gatesList.reserve(catalog.size());

        for (const auto& gate : catalog) {
            crow::json::wvalue g;
            g["name"] = gate->getName();
            g["symbol"] = gate->getSymbol();
            g["numQubits"] = gate->getNumQubits();
            g["isControlled"] = gate->isControlled();
            g["description"] = gate->getDescription();
            g["functionSummary"] = gate->getFunctionSummary();
            g["exampleOperation"] = gate->getExampleOperation();
            g["matrix"] = matrixToJson(gate->getMatrix());
            gatesList.push_back(std::move(g));
        }

        crow::json::wvalue res;
        res["success"] = true;
        res["gates"] = crow::json::wvalue::list(std::move(gatesList));
        return res;
    });

    // Store/validate circuit configuration
    CROW_ROUTE(app, "/api/circuit").methods("POST"_method)([this](const crow::request& req) {
        try {
            const auto body = crow::json::load(req.body);
            QuantumCircuit parsed = buildCircuitFromJson(body);

            std::lock_guard<std::mutex> lock(circuitMutex_);
            activeCircuit_ = std::move(parsed);

            crow::json::wvalue res;
            res["success"] = true;
            res["message"] = "Circuit validated and stored.";
            res["numQubits"] = activeCircuit_.getNumQubits();
            res["gateCount"] = static_cast<int>(activeCircuit_.getGateCount());
            res["circuitDepth"] = activeCircuit_.getDepth();
            res["summary"] = activeCircuit_.getSummary();
            return crow::response(200, res);
        } catch (const QuantumException& ex) {
            return jsonError(400, "Invalid circuit configuration", ex.what());
        } catch (const std::exception& ex) {
            return jsonError(500, "Server error while saving circuit", ex.what());
        }
    });

    // Simulate circuit (accepts payload or simulates currently stored circuit)
    CROW_ROUTE(app, "/api/simulate").methods("POST"_method)([this](const crow::request& req) {
        try {
            QuantumCircuit targetCircuit(2);
            if (!req.body.empty()) {
                const auto body = crow::json::load(req.body);
                targetCircuit = buildCircuitFromJson(body);
                std::lock_guard<std::mutex> lock(circuitMutex_);
                activeCircuit_ = targetCircuit;
            } else {
                std::lock_guard<std::mutex> lock(circuitMutex_);
                targetCircuit = activeCircuit_;
            }

            const SimulationResult sim = simulator_.simulate(targetCircuit);
            return crow::response(200, serializeSimulationResult(sim, targetCircuit));
        } catch (const QuantumException& ex) {
            return jsonError(400, "Simulation validation error", ex.what());
        } catch (const std::exception& ex) {
            return jsonError(500, "Simulation engine failure", ex.what());
        }
    });

    // Reset active circuit
    CROW_ROUTE(app, "/api/reset").methods("POST"_method)([this](const crow::request& req) {
        try {
            int numQubits = 2;
            if (!req.body.empty()) {
                const auto body = crow::json::load(req.body);
                if (body && body.has("numQubits")) {
                    numQubits = static_cast<int>(body["numQubits"].i());
                }
            }

            std::lock_guard<std::mutex> lock(circuitMutex_);
            activeCircuit_.reset(numQubits);

            crow::json::wvalue res;
            res["success"] = true;
            res["message"] = "Circuit reset to initial |0> states.";
            res["numQubits"] = activeCircuit_.getNumQubits();
            res["gateCount"] = 0;
            res["circuitDepth"] = 0;
            return crow::response(200, res);
        } catch (const QuantumException& ex) {
            return jsonError(400, "Failed to reset circuit", ex.what());
        }
    });

    // Frontend static routes
    CROW_ROUTE(app, "/")([this]() {
        return serveStaticFile(frontendDir_, "index.html");
    });

    CROW_ROUTE(app, "/css/<path>")([this](const std::string& path) {
        return serveStaticFile(frontendDir_, "css/" + path);
    });

    CROW_ROUTE(app, "/js/<path>")([this](const std::string& path) {
        return serveStaticFile(frontendDir_, "js/" + path);
    });

    CROW_ROUTE(app, "/assets/<path>")([this](const std::string& path) {
        return serveStaticFile(frontendDir_, "assets/" + path);
    });

    std::cout << "====================================================\n";
    std::cout << "  Quantum Lite — C++ Quantum Circuit Simulator\n";
    std::cout << "  Server listening at: http://localhost:" << port_ << "\n";
    if (!frontendDir_.empty()) {
        std::cout << "  Serving frontend from: " << frontendDir_ << "\n";
    } else {
        std::cout << "  Frontend directory not auto-detected; API-only mode.\n";
    }
    std::cout << "====================================================\n";

    app.port(static_cast<uint16_t>(port_)).multithreaded().run();
}

} // namespace qlite
