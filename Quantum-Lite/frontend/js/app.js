/**
 * app.js — Application controller connecting CircuitBuilderUI, QuantumAPI, and Results visualization.
 */
document.addEventListener("DOMContentLoaded", () => {
    const circuitGridEl = document.getElementById("circuitGrid");
    const qubitCountDisplay = document.getElementById("qubitCountDisplay");
    const gateCountDisplay = document.getElementById("gateCountDisplay");
    const circuitDepthDisplay = document.getElementById("circuitDepthDisplay");
    const placementHint = document.getElementById("placementHint");
    const builderAlert = document.getElementById("builderAlert");

    const backendStatusBadge = document.getElementById("backendStatusBadge");
    const backendStatusText = document.getElementById("backendStatusText");

    const initialStateDirac = document.getElementById("initialStateDirac");
    const finalStateDirac = document.getElementById("finalStateDirac");
    const executionSummary = document.getElementById("executionSummary");
    const normStatusBadge = document.getElementById("normStatusBadge");
    const normSumValue = document.getElementById("normSumValue");
    const stateVectorTableBody = document.getElementById("stateVectorTableBody");
    const executionTraceList = document.getElementById("executionTraceList");
    const gateLibraryGrid = document.getElementById("gateLibraryGrid");
    const probCanvas = document.getElementById("probabilityChart");

    let probChartInstance = null;

    const builder = new CircuitBuilderUI(circuitGridEl, () => {
        updateStats();
    });

    function showAlert(message, type = "info") {
        builderAlert.textContent = message;
        builderAlert.className = `builder-alert ${type}`;
    }

    function hideAlert() {
        builderAlert.className = "builder-alert hidden";
    }

    function updateStats() {
        qubitCountDisplay.textContent = String(builder.numQubits);
        gateCountDisplay.textContent = String(builder.gates.length);
        circuitDepthDisplay.textContent = String(builder.getDepth());
        const initKet = `|${builder.initialStates.join("")}\u27E9`;
        initialStateDirac.textContent = initKet;
    }

    // Navigation active state
    document.querySelectorAll(".nav-link").forEach((link) => {
        link.addEventListener("click", () => {
            document.querySelectorAll(".nav-link").forEach((l) => l.classList.remove("active"));
            link.classList.add("active");
        });
    });

    document.getElementById("startSimulationBtn").addEventListener("click", () => {
        document.getElementById("builder").scrollIntoView({ behavior: "smooth" });
    });

    document.getElementById("exploreGatesBtn").addEventListener("click", () => {
        document.getElementById("library").scrollIntoView({ behavior: "smooth" });
    });

    // Gate palette selection & drag start
    const gateChips = document.querySelectorAll(".gate-chip");
    function highlightSelectedGate(type) {
        gateChips.forEach((chip) => {
            chip.classList.toggle("selected", chip.dataset.gate === type);
        });
        builder.setSelectedGate(type);
        placementHint.className = "placement-hint";
        placementHint.textContent =
            type === "CNOT"
                ? "CNOT Mode: Click or drop on Control qubit, then click Target qubit in the same column."
                : `Selected ${type} Gate: Click any grid slot or drag onto a qubit wire.`;
    }

    gateChips.forEach((chip) => {
        chip.addEventListener("click", () => highlightSelectedGate(chip.dataset.gate));
        chip.addEventListener("dragstart", (e) => {
            e.dataTransfer.setData("text/plain", chip.dataset.gate);
            highlightSelectedGate(chip.dataset.gate);
        });
    });
    highlightSelectedGate("H");

    // Circuit grid cell interaction
    circuitGridEl.addEventListener("cell-interact", (e) => {
        const { type, qubit, column } = e.detail;
        try {
            hideAlert();
            const outcome = builder.placeGateAt(type, qubit, column);
            if (outcome.action === "cnot_pending") {
                placementHint.className = "placement-hint cnot-pending";
                placementHint.textContent = `CNOT Control set on q${outcome.control} (Step ${outcome.column + 1}). Now click the Target qubit in Step ${outcome.column + 1}.`;
            } else {
                placementHint.className = "placement-hint";
                placementHint.textContent =
                    type === "CNOT"
                        ? "CNOT placed! Click a placed gate to remove it, or continue building."
                        : `Selected ${builder.selectedGateType} Gate: Click any grid slot or drag onto a qubit wire.`;
            }
        } catch (err) {
            showAlert(err.message, "error");
        }
    });

    // Toolbar buttons
    document.getElementById("addQubitBtn").addEventListener("click", () => {
        try {
            hideAlert();
            builder.addQubit();
        } catch (err) {
            showAlert(err.message, "error");
        }
    });

    document.getElementById("removeQubitBtn").addEventListener("click", () => {
        try {
            hideAlert();
            builder.removeQubit();
        } catch (err) {
            showAlert(err.message, "error");
        }
    });

    document.getElementById("addColumnBtn").addEventListener("click", () => {
        try {
            hideAlert();
            builder.addColumn();
        } catch (err) {
            showAlert(err.message, "error");
        }
    });

    document.getElementById("saveCircuitBtn").addEventListener("click", async () => {
        try {
            const resp = await QuantumAPI.saveCircuit(builder.getPayload());
            showAlert(`Validated by C++ Engine: ${resp.summary}`, "success");
        } catch (err) {
            showAlert(err.message, "error");
        }
    });

    document.getElementById("resetCircuitBtn").addEventListener("click", async () => {
        builder.reset(builder.numQubits);
        try {
            await QuantumAPI.resetCircuit(builder.numQubits);
            showAlert("Circuit reset to initial |0> states.", "info");
        } catch (err) {
            showAlert(`Local grid reset. (Backend note: ${err.message})`, "info");
        }
    });

    document.getElementById("executeCircuitBtn").addEventListener("click", async () => {
        await runSimulation(true);
    });

    // Quick presets
    document.querySelectorAll("[data-preset]").forEach((btn) => {
        btn.addEventListener("click", async () => {
            builder.loadPreset(btn.dataset.preset);
            showAlert(`Loaded preset: ${btn.textContent}. Executing via C++ backend...`, "info");
            await runSimulation(false);
        });
    });

    window.addEventListener("resize", () => builder.drawCnotConnectors());

    // Simulation execution
    async function runSimulation(scrollToResults) {
        try {
            const payload = builder.getPayload();
            const result = await QuantumAPI.simulateCircuit(payload);
            renderResults(result);
            showAlert(`Simulation complete: |psi> = ${result.finalDirac}`, "success");
            if (scrollToResults) {
                document.getElementById("results").scrollIntoView({ behavior: "smooth" });
            }
        } catch (err) {
            showAlert(`Simulation Error: ${err.message}`, "error");
        }
    }

    function renderResults(res) {
        initialStateDirac.textContent = res.initialDirac;
        finalStateDirac.textContent = res.finalDirac;
        executionSummary.textContent = `${res.summary} \u2022 ${res.basisStates.length} basis states computed`;

        normSumValue.textContent = Number(res.normSum).toFixed(4);
        normStatusBadge.classList.remove("hidden");

        // State Vector Table
        stateVectorTableBody.innerHTML = "";
        for (let i = 0; i < res.basisStates.length; i++) {
            const amp = res.finalState[i];
            const prob = res.probabilities[i];
            const pct = (prob * 100).toFixed(2);
            const tr = document.createElement("tr");

            const tdBasis = document.createElement("td");
            tdBasis.textContent = res.basisStates[i];

            const tdAmp = document.createElement("td");
            tdAmp.textContent = amp.formatted;

            const tdMag = document.createElement("td");
            tdMag.textContent = amp.magnitude.toFixed(4);

            const tdProb = document.createElement("td");
            tdProb.innerHTML = `
                <div class="prob-bar-wrap">
                    <div class="prob-bar-track"><div class="prob-bar-fill" style="width:${pct}%"></div></div>
                    <span>${pct}%</span>
                </div>
            `;

            tr.appendChild(tdBasis);
            tr.appendChild(tdAmp);
            tr.appendChild(tdMag);
            tr.appendChild(tdProb);
            stateVectorTableBody.appendChild(tr);
        }

        // Execution Trace
        executionTraceList.innerHTML = "";
        if (!res.steps || res.steps.length === 0) {
            executionTraceList.innerHTML =
                '<div class="empty-cell">No gates placed — circuit remains in initial state.</div>';
        } else {
            for (const step of res.steps) {
                const item = document.createElement("div");
                item.className = "trace-item";
                const qText = step.qubits.map((q) => `q${q}`).join(" \u2192 ");
                item.innerHTML = `
                    <div class="trace-step-badge">STEP ${step.stepIndex} (Col ${step.column + 1})</div>
                    <div class="trace-gate-info">${step.gateSymbol} on ${qText}</div>
                    <div class="trace-state-dirac">${step.diracAfter}</div>
                `;
                executionTraceList.appendChild(item);
            }
        }

        renderProbabilityChart(res.basisStates, res.probabilities);
    }

    function renderProbabilityChart(labels, probabilities) {
        const percentages = probabilities.map((p) => Number((p * 100).toFixed(2)));

        if (typeof window.Chart !== "undefined") {
            if (probChartInstance) {
                probChartInstance.destroy();
            }
            const ctx = probCanvas.getContext("2d");
            const grad = ctx.createLinearGradient(0, 0, 0, 280);
            grad.addColorStop(0, "rgba(0, 229, 255, 0.88)");
            grad.addColorStop(1, "rgba(139, 92, 246, 0.72)");

            probChartInstance = new window.Chart(ctx, {
                type: "bar",
                data: {
                    labels,
                    datasets: [
                        {
                            label: "Measurement Probability (%)",
                            data: percentages,
                            backgroundColor: grad,
                            borderColor: "#00e5ff",
                            borderWidth: 1.5,
                            borderRadius: 6,
                        },
                    ],
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    plugins: {
                        legend: { labels: { color: "#cbd5e1" } },
                        tooltip: {
                            callbacks: {
                                label: (context) => ` Probability: ${context.parsed.y}%`,
                            },
                        },
                    },
                    scales: {
                        x: {
                            ticks: { color: "#94a3b8", font: { family: "JetBrains Mono" } },
                            grid: { color: "rgba(148,163,184,0.08)" },
                        },
                        y: {
                            beginAtZero: true,
                            max: 100,
                            ticks: {
                                color: "#94a3b8",
                                callback: (v) => `${v}%`,
                            },
                            grid: { color: "rgba(148,163,184,0.1)" },
                        },
                    },
                },
            });
            return;
        }

        // Pure HTML5 Canvas fallback if Chart.js CDN is offline
        drawFallbackBarChart(probCanvas, labels, percentages);
    }

    function drawFallbackBarChart(canvas, labels, percentages) {
        const rect = canvas.parentElement.getBoundingClientRect();
        canvas.width = Math.max(360, rect.width);
        canvas.height = 290;
        const ctx = canvas.getContext("2d");
        ctx.clearRect(0, 0, canvas.width, canvas.height);

        const pad = { top: 24, right: 20, bottom: 44, left: 48 };
        const w = canvas.width - pad.left - pad.right;
        const h = canvas.height - pad.top - pad.bottom;

        ctx.strokeStyle = "rgba(148, 163, 184, 0.2)";
        ctx.fillStyle = "#94a3b8";
        ctx.font = "12px sans-serif";
        for (let pct = 0; pct <= 100; pct += 25) {
            const y = pad.top + h - (pct / 100) * h;
            ctx.beginPath();
            ctx.moveTo(pad.left, y);
            ctx.lineTo(pad.left + w, y);
            ctx.stroke();
            ctx.fillText(`${pct}%`, 8, y + 4);
        }

        const barW = Math.min(54, (w / labels.length) * 0.58);
        const slotW = w / labels.length;
        labels.forEach((lbl, idx) => {
            const val = percentages[idx];
            const barH = (val / 100) * h;
            const x = pad.left + idx * slotW + (slotW - barW) / 2;
            const y = pad.top + h - barH;

            const grad = ctx.createLinearGradient(0, y, 0, pad.top + h);
            grad.addColorStop(0, "#00e5ff");
            grad.addColorStop(1, "#8b5cf6");
            ctx.fillStyle = grad;
            ctx.fillRect(x, y, barW, barH);

            ctx.fillStyle = "#e2e8f0";
            ctx.textAlign = "center";
            ctx.fillText(`${val}%`, x + barW / 2, y - 6);
            ctx.fillStyle = "#94a3b8";
            ctx.fillText(lbl, x + barW / 2, pad.top + h + 20);
        });
    }

    // Load Gate Library from C++ /api/gates
    async function loadGateLibrary() {
        try {
            const data = await QuantumAPI.fetchGates();
            gateLibraryGrid.innerHTML = "";
            for (const gate of data.gates) {
                const card = document.createElement("article");
                card.className = "library-card glass";

                const cols = gate.matrix[0].length;
                const matrixCells = gate.matrix
                    .flat()
                    .map((cell) => `<span>${cell.formatted}</span>`)
                    .join("");

                card.innerHTML = `
                    <div class="lib-card-head">
                        <h3>${gate.name}</h3>
                        <span class="gate-chip gate-${gate.symbol}"><span class="gate-sym">${gate.symbol}</span></span>
                    </div>
                    <p class="lib-desc">${gate.description}</p>
                    <div class="matrix-box" style="grid-template-columns: repeat(${cols}, auto);">${matrixCells}</div>
                    <div class="lib-meta">
                        <div><strong>Function:</strong> ${gate.functionSummary}</div>
                        <div style="margin-top:4px;"><strong>Example:</strong> <code>${gate.exampleOperation}</code></div>
                    </div>
                `;

                const tryBtn = document.createElement("button");
                tryBtn.className = "btn btn-outline";
                tryBtn.textContent = `Try ${gate.symbol} in Builder`;
                tryBtn.addEventListener("click", async () => {
                    const presetMap = {
                        H: "superposition",
                        X: "bitflip",
                        CNOT: "bell",
                        Z: "phase",
                    };
                    if (presetMap[gate.symbol]) {
                        builder.loadPreset(presetMap[gate.symbol]);
                    } else {
                        builder.reset(1);
                        builder.placeGateAt(gate.symbol, 0, 0);
                    }
                    document.getElementById("builder").scrollIntoView({ behavior: "smooth" });
                    await runSimulation(false);
                });
                card.appendChild(tryBtn);
                gateLibraryGrid.appendChild(card);
            }
        } catch (err) {
            gateLibraryGrid.innerHTML = `<div class="panel glass empty-cell">Unable to load Gate Library from C++ backend (${err.message}). Start the C++ Crow server and refresh.</div>`;
        }
    }

    // Check C++ Backend Health
    async function checkBackend() {
        try {
            const health = await QuantumAPI.checkHealth();
            backendStatusBadge.className = "status-pill status-online";
            backendStatusText.textContent = `C++ Engine Online (${health.status})`;
        } catch (_) {
            backendStatusBadge.className = "status-pill status-offline";
            backendStatusText.textContent = "C++ Backend Offline";
        }
    }

    // Initialize workspace with Bell State preset ready to test
    builder.loadPreset("bell");
    updateStats();
    checkBackend();
    loadGateLibrary();
    runSimulation(false);
});
