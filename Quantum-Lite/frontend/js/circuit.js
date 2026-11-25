/**
 * circuit.js — Interactive quantum circuit editor and SVG CNOT connector renderer.
 */
class CircuitBuilderUI {
    constructor(gridElement, onChangeCallback) {
        this.gridEl = gridElement;
        this.onChange = onChangeCallback || (() => {});
        this.numQubits = 2;
        this.numColumns = 6;
        this.initialStates = ["0", "0"];
        this.gates = []; // { id, type, qubits: [...], column }
        this.selectedGateType = "H";
        this.pendingCnot = null; // { column, control }
        this.nextId = 1;
    }

    setSelectedGate(type) {
        this.selectedGateType = type;
        this.pendingCnot = null;
        this.render();
    }

    addQubit() {
        if (this.numQubits >= 8) {
            throw new Error("UI supports up to 8 qubits (256 basis states) for clear visualization.");
        }
        this.numQubits += 1;
        this.initialStates.push("0");
        this.pendingCnot = null;
        this.render();
        this.onChange();
    }

    removeQubit() {
        if (this.numQubits <= 1) {
            throw new Error("A quantum circuit must have at least 1 qubit.");
        }
        const removedIdx = this.numQubits - 1;
        this.numQubits -= 1;
        this.initialStates.pop();
        this.gates = this.gates.filter((g) => !g.qubits.includes(removedIdx));
        this.pendingCnot = null;
        this.render();
        this.onChange();
    }

    addColumn() {
        if (this.numColumns >= 16) {
            throw new Error("Maximum 16 circuit step columns reached.");
        }
        this.numColumns += 1;
        this.render();
        this.onChange();
    }

    toggleInitialState(qubitIndex) {
        this.initialStates[qubitIndex] = this.initialStates[qubitIndex] === "0" ? "1" : "0";
        this.render();
        this.onChange();
    }

    reset(numQubits = 2) {
        this.numQubits = numQubits;
        this.numColumns = 6;
        this.initialStates = Array(numQubits).fill("0");
        this.gates = [];
        this.pendingCnot = null;
        this.render();
        this.onChange();
    }

    loadPreset(presetName) {
        this.pendingCnot = null;
        if (presetName === "superposition") {
            this.numQubits = 1;
            this.numColumns = 5;
            this.initialStates = ["0"];
            this.gates = [{ id: this.nextId++, type: "H", qubits: [0], column: 0 }];
        } else if (presetName === "bitflip") {
            this.numQubits = 1;
            this.numColumns = 5;
            this.initialStates = ["0"];
            this.gates = [{ id: this.nextId++, type: "X", qubits: [0], column: 0 }];
        } else if (presetName === "bell") {
            this.numQubits = 2;
            this.numColumns = 6;
            this.initialStates = ["0", "0"];
            this.gates = [
                { id: this.nextId++, type: "H", qubits: [0], column: 0 },
                { id: this.nextId++, type: "CNOT", qubits: [0, 1], column: 1 },
            ];
        } else if (presetName === "phase") {
            this.numQubits = 1;
            this.numColumns = 5;
            this.initialStates = ["0"];
            this.gates = [
                { id: this.nextId++, type: "H", qubits: [0], column: 0 },
                { id: this.nextId++, type: "Z", qubits: [0], column: 1 },
                { id: this.nextId++, type: "H", qubits: [0], column: 2 },
            ];
        } else if (presetName === "ghz") {
            this.numQubits = 3;
            this.numColumns = 6;
            this.initialStates = ["0", "0", "0"];
            this.gates = [
                { id: this.nextId++, type: "H", qubits: [0], column: 0 },
                { id: this.nextId++, type: "CNOT", qubits: [0, 1], column: 1 },
                { id: this.nextId++, type: "CNOT", qubits: [1, 2], column: 2 },
            ];
        }
        this.render();
        this.onChange();
    }

    findGateAt(column, qubit) {
        return this.gates.find((g) => g.column === column && g.qubits.includes(qubit)) || null;
    }

    placeGateAt(type, qubit, column) {
        // Remove existing gate at this exact slot first if user clicks to remove
        const existing = this.findGateAt(column, qubit);
        if (existing) {
            this.gates = this.gates.filter((g) => g.id !== existing.id);
            this.pendingCnot = null;
            this.render();
            this.onChange();
            return { action: "removed", gate: existing };
        }

        if (type === "CNOT") {
            if (this.numQubits < 2) {
                throw new Error("CNOT gate requires at least 2 qubits. Click '+ Add Qubit' first.");
            }
            if (this.pendingCnot && this.pendingCnot.column === column) {
                if (this.pendingCnot.control === qubit) {
                    this.pendingCnot = null;
                    this.render();
                    return { action: "cancelled" };
                }
                const newGate = {
                    id: this.nextId++,
                    type: "CNOT",
                    qubits: [this.pendingCnot.control, qubit],
                    column,
                };
                this.gates.push(newGate);
                this.pendingCnot = null;
                this.ensureTrailingColumn(column);
                this.render();
                this.onChange();
                return { action: "placed", gate: newGate };
            } else {
                this.pendingCnot = { column, control: qubit };
                this.render();
                return { action: "cnot_pending", control: qubit, column };
            }
        }

        const newGate = {
            id: this.nextId++,
            type,
            qubits: [qubit],
            column,
        };
        this.gates.push(newGate);
        this.ensureTrailingColumn(column);
        this.render();
        this.onChange();
        return { action: "placed", gate: newGate };
    }

    ensureTrailingColumn(usedColumn) {
        if (usedColumn >= this.numColumns - 1 && this.numColumns < 16) {
            this.numColumns += 1;
        }
    }

    getPayload() {
        return {
            numQubits: this.numQubits,
            initialStates: [...this.initialStates],
            gates: this.gates.map((g) => ({
                type: g.type,
                qubits: [...g.qubits],
                column: g.column,
            })),
        };
    }

    getDepth() {
        if (this.gates.length === 0) return 0;
        return Math.max(...this.gates.map((g) => g.column)) + 1;
    }

    render() {
        this.gridEl.innerHTML = "";
        const colTemplate = `112px repeat(${this.numColumns}, minmax(66px, 1fr))`;

        // Column headers
        const headerRow = document.createElement("div");
        headerRow.className = "circuit-header-row";
        headerRow.style.gridTemplateColumns = colTemplate;

        const corner = document.createElement("div");
        corner.className = "col-label";
        corner.textContent = "Qubit";
        headerRow.appendChild(corner);

        for (let c = 0; c < this.numColumns; c++) {
            const colLbl = document.createElement("div");
            colLbl.className = "col-label";
            colLbl.textContent = `Step ${c + 1}`;
            headerRow.appendChild(colLbl);
        }
        this.gridEl.appendChild(headerRow);

        // Qubit rows
        for (let q = 0; q < this.numQubits; q++) {
            const row = document.createElement("div");
            row.className = "qubit-row";
            row.style.gridTemplateColumns = colTemplate;

            const labelBox = document.createElement("div");
            labelBox.className = "qubit-label-box";

            const qName = document.createElement("span");
            qName.className = "q-name";
            qName.textContent = `q${q}`;

            const initBtn = document.createElement("button");
            initBtn.type = "button";
            initBtn.className = `q-init-btn ${this.initialStates[q] === "1" ? "state-one" : ""}`;
            initBtn.textContent = `|${this.initialStates[q]}\u27E9`;
            initBtn.title = "Click to toggle initial state |0> / |1>";
            initBtn.addEventListener("click", () => this.toggleInitialState(q));

            labelBox.appendChild(qName);
            labelBox.appendChild(initBtn);
            row.appendChild(labelBox);

            for (let c = 0; c < this.numColumns; c++) {
                const cell = document.createElement("div");
                cell.className = "circuit-cell";
                cell.dataset.qubit = String(q);
                cell.dataset.column = String(c);

                const gate = this.findGateAt(c, q);
                if (gate) {
                    if (gate.type === "CNOT") {
                        const isControl = gate.qubits[0] === q;
                        const el = document.createElement("div");
                        el.className = isControl ? "cnot-control-dot" : "cnot-target-xor";
                        if (!isControl) el.textContent = "\u2295";
                        el.title = `CNOT (Control: q${gate.qubits[0]}, Target: q${gate.qubits[1]}) — Click to remove`;
                        cell.appendChild(el);
                    } else {
                        const el = document.createElement("div");
                        el.className = `placed-gate type-${gate.type}`;
                        el.textContent = gate.type;
                        el.title = `${gate.type} Gate on q${q} — Click to remove`;
                        cell.appendChild(el);
                    }
                    const rm = document.createElement("span");
                    rm.className = "gate-remove-x";
                    rm.textContent = "\u00D7";
                    cell.appendChild(rm);
                } else if (this.pendingCnot && this.pendingCnot.column === c) {
                    if (this.pendingCnot.control === q) {
                        const dot = document.createElement("div");
                        dot.className = "cnot-control-dot";
                        dot.title = "CNOT Control Selected — Click another qubit in this column";
                        cell.appendChild(dot);
                    } else {
                        cell.classList.add("cnot-target-candidate");
                        cell.title = `Click to set q${q} as CNOT target`;
                    }
                }

                // Drag & drop handlers
                cell.addEventListener("dragover", (e) => {
                    e.preventDefault();
                    cell.classList.add("drag-over");
                });
                cell.addEventListener("dragleave", () => {
                    cell.classList.remove("drag-over");
                });
                cell.addEventListener("drop", (e) => {
                    e.preventDefault();
                    cell.classList.remove("drag-over");
                    const droppedType = e.dataTransfer.getData("text/plain") || this.selectedGateType;
                    if (droppedType) {
                        this.gridEl.dispatchEvent(
                            new CustomEvent("cell-interact", {
                                detail: { type: droppedType, qubit: q, column: c },
                            })
                        );
                    }
                });

                cell.addEventListener("click", () => {
                    this.gridEl.dispatchEvent(
                        new CustomEvent("cell-interact", {
                            detail: { type: this.selectedGateType, qubit: q, column: c },
                        })
                    );
                });

                row.appendChild(cell);
            }

            this.gridEl.appendChild(row);
        }

        // Draw SVG connector lines for CNOT gates
        requestAnimationFrame(() => this.drawCnotConnectors());
    }

    drawCnotConnectors() {
        const existingSvg = this.gridEl.querySelector(".cnot-overlay-svg");
        if (existingSvg) existingSvg.remove();

        const cnotGates = this.gates.filter((g) => g.type === "CNOT");
        if (cnotGates.length === 0) return;

        const gridRect = this.gridEl.getBoundingClientRect();
        if (gridRect.width === 0 || gridRect.height === 0) return;

        const svgNS = "http://www.w3.org/2000/svg";
        const svg = document.createElementNS(svgNS, "svg");
        svg.setAttribute("class", "cnot-overlay-svg");
        svg.setAttribute("viewBox", `0 0 ${gridRect.width} ${gridRect.height}`);

        for (const gate of cnotGates) {
            const ctrlCell = this.gridEl.querySelector(
                `.circuit-cell[data-column="${gate.column}"][data-qubit="${gate.qubits[0]}"]`
            );
            const targCell = this.gridEl.querySelector(
                `.circuit-cell[data-column="${gate.column}"][data-qubit="${gate.qubits[1]}"]`
            );
            if (!ctrlCell || !targCell) continue;

            const cRect = ctrlCell.getBoundingClientRect();
            const tRect = targCell.getBoundingClientRect();
            const x = cRect.left - gridRect.left + cRect.width / 2;
            const y1 = cRect.top - gridRect.top + cRect.height / 2;
            const y2 = tRect.top - gridRect.top + tRect.height / 2;

            const line = document.createElementNS(svgNS, "line");
            line.setAttribute("x1", String(x));
            line.setAttribute("y1", String(y1));
            line.setAttribute("x2", String(x));
            line.setAttribute("y2", String(y2));
            line.setAttribute("stroke", "#f59e0b");
            line.setAttribute("stroke-width", "2.5");
            line.setAttribute("stroke-linecap", "round");
            svg.appendChild(line);
        }

        this.gridEl.appendChild(svg);
    }
}
