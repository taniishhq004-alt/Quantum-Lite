/**
 * api.js — REST API client communicating with the C++ Crow backend.
 * Zero quantum simulation math is performed in JavaScript.
 */
const QuantumAPI = (() => {
    // When served from Crow (http://localhost:8080), relative /api works directly.
    // If opened from file:// or Live Server (e.g., port 5500), fallback to localhost:8080.
    const isSameOriginCrow =
        window.location.protocol.startsWith("http") &&
        window.location.port === "8080";

    const BASE_URL = isSameOriginCrow ? "" : "http://localhost:8080";

    async function request(path, options = {}) {
        const response = await fetch(`${BASE_URL}${path}`, {
            headers: {
                "Content-Type": "application/json",
                ...(options.headers || {}),
            },
            ...options,
        });

        let payload = null;
        try {
            payload = await response.json();
        } catch (_) {
            payload = null;
        }

        if (!response.ok) {
            const msg =
                (payload && (payload.details || payload.error)) ||
                `HTTP ${response.status} error from C++ backend`;
            throw new Error(msg);
        }

        return payload;
    }

    return {
        checkHealth() {
            return request("/api/health", { method: "GET" });
        },
        fetchGates() {
            return request("/api/gates", { method: "GET" });
        },
        saveCircuit(circuitPayload) {
            return request("/api/circuit", {
                method: "POST",
                body: JSON.stringify(circuitPayload),
            });
        },
        simulateCircuit(circuitPayload) {
            return request("/api/simulate", {
                method: "POST",
                body: JSON.stringify(circuitPayload),
            });
        },
        resetCircuit(numQubits = 2) {
            return request("/api/reset", {
                method: "POST",
                body: JSON.stringify({ numQubits }),
            });
        },
    };
})();
