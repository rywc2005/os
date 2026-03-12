// ==========================
// Scheduling Algorithms (JS implementation)
// ==========================
function deepCopyProcs(procs) {
    return procs.map(p => ({
        ...p,
        remainingTime: p.burstTime,
        startTime: -1,
        completionTime: 0,
        turnaroundTime: 0,
        weightedTurnaround: 0,
        waitingTime: 0,
        responseTime: 0,
        state: 'NEW',
        hasStarted: false,
        currentQueue: 0
    }));
}

function calcMetrics(p) {
    p.turnaroundTime = p.completionTime - p.arrivalTime;
    p.waitingTime = p.turnaroundTime - p.burstTime;
    p.weightedTurnaround = p.burstTime > 0 ? p.turnaroundTime / p.burstTime : 0;
    p.responseTime = p.startTime - p.arrivalTime;
}

function computeStats(procs, gantt) {
    const n = procs.length;
    if (n === 0) return {};
    const avgTT = procs.reduce((s,p) => s+p.turnaroundTime, 0) / n;
    const avgWT = procs.reduce((s,p) => s+p.waitingTime, 0) / n;
    const avgRT = procs.reduce((s,p) => s+p.responseTime, 0) / n;
    const avgWTT = procs.reduce((s,p) => s+p.weightedTurnaround, 0) / n;
    let totalTime = 0, busyTime = 0;
    if (gantt.length > 0) {
        totalTime = gantt[gantt.length-1].end - gantt[0].start;
        busyTime = gantt.filter(e => e.process !== 'IDLE').reduce((s,e) => s + (e.end - e.start), 0);
    }
    const cpuUtil = totalTime > 0 ? (busyTime / totalTime * 100) : 0;
    const throughput = totalTime > 0 ? n / totalTime : 0;
    return { avgTurnaroundTime: avgTT, avgWaitingTime: avgWT, avgResponseTime: avgRT,
             avgWeightedTurnaround: avgWTT, cpuUtilization: cpuUtil, throughput };
}

function mergeEvent(gantt, name, start, end) {
    if (gantt.length > 0 && gantt[gantt.length-1].process === name && gantt[gantt.length-1].end === start) {
        gantt[gantt.length-1].end = end;
    } else {
        gantt.push({process: name, start, end});
    }
}

// FCFS
function scheduleFCFS(inputProcs) {
    const procs = deepCopyProcs(inputProcs).sort((a,b) => a.arrivalTime - b.arrivalTime || a.pid - b.pid);
    const gantt = [];
    let time = 0;
    for (const p of procs) {
        if (time < p.arrivalTime) {
            mergeEvent(gantt, 'IDLE', time, p.arrivalTime);
            time = p.arrivalTime;
        }
        if (!p.hasStarted) { p.startTime = time; p.hasStarted = true; }
        mergeEvent(gantt, p.name, time, time + p.burstTime);
        time += p.burstTime;
        p.remainingTime = 0;
        p.completionTime = time;
        p.state = 'TERMINATED';
        calcMetrics(p);
    }
    return { procs, gantt, metrics: computeStats(procs, gantt) };
}

// SJF non-preemptive
function scheduleSJF(inputProcs) {
    const procs = deepCopyProcs(inputProcs);
    const gantt = [];
    let time = 0, completed = 0, n = procs.length;
    const done = new Array(n).fill(false);
    while (completed < n) {
        let ready = [];
        for (let i = 0; i < n; i++) {
            if (!done[i] && procs[i].arrivalTime <= time) ready.push(i);
        }
        if (ready.length === 0) {
            let next = Infinity;
            for (let i = 0; i < n; i++) if (!done[i]) next = Math.min(next, procs[i].arrivalTime);
            mergeEvent(gantt, 'IDLE', time, next);
            time = next;
            continue;
        }
        ready.sort((a,b) => procs[a].burstTime - procs[b].burstTime || procs[a].arrivalTime - procs[b].arrivalTime);
        let idx = ready[0];
        let p = procs[idx];
        if (!p.hasStarted) { p.startTime = time; p.hasStarted = true; }
        mergeEvent(gantt, p.name, time, time + p.burstTime);
        time += p.burstTime;
        p.remainingTime = 0;
        p.completionTime = time;
        p.state = 'TERMINATED';
        calcMetrics(p);
        done[idx] = true;
        completed++;
    }
    return { procs, gantt, metrics: computeStats(procs, gantt) };
}

// SRTF preemptive
function scheduleSRTF(inputProcs) {
    const procs = deepCopyProcs(inputProcs);
    const gantt = [];
    let time = 0, completed = 0, n = procs.length;
    while (completed < n) {
        let ready = [];
        for (let i = 0; i < n; i++) {
            if (procs[i].arrivalTime <= time && procs[i].state !== 'TERMINATED') ready.push(i);
        }
        if (ready.length === 0) {
            let next = Infinity;
            for (let i = 0; i < n; i++) if (procs[i].state !== 'TERMINATED') next = Math.min(next, procs[i].arrivalTime);
            mergeEvent(gantt, 'IDLE', time, next);
            time = next;
            continue;
        }
        ready.sort((a,b) => procs[a].remainingTime - procs[b].remainingTime || procs[a].arrivalTime - procs[b].arrivalTime);
        let idx = ready[0];
        let p = procs[idx];
        if (!p.hasStarted) { p.startTime = time; p.hasStarted = true; }
        // Find next event: new arrival or completion
        let nextEvent = time + p.remainingTime;
        for (let i = 0; i < n; i++) {
            if (procs[i].arrivalTime > time && procs[i].state !== 'TERMINATED') {
                nextEvent = Math.min(nextEvent, procs[i].arrivalTime);
            }
        }
        let exec = nextEvent - time;
        mergeEvent(gantt, p.name, time, time + exec);
        p.remainingTime -= exec;
        time += exec;
        if (p.remainingTime === 0) {
            p.completionTime = time;
            p.state = 'TERMINATED';
            calcMetrics(p);
            completed++;
        }
    }
    return { procs, gantt, metrics: computeStats(procs, gantt) };
}

// Priority non-preemptive
function schedulePriorityNP(inputProcs) {
    const procs = deepCopyProcs(inputProcs);
    const gantt = [];
    let time = 0, completed = 0, n = procs.length;
    const done = new Array(n).fill(false);
    while (completed < n) {
        let ready = [];
        for (let i = 0; i < n; i++) {
            if (!done[i] && procs[i].arrivalTime <= time) ready.push(i);
        }
        if (ready.length === 0) {
            let next = Infinity;
            for (let i = 0; i < n; i++) if (!done[i]) next = Math.min(next, procs[i].arrivalTime);
            mergeEvent(gantt, 'IDLE', time, next);
            time = next;
            continue;
        }
        ready.sort((a,b) => procs[a].priority - procs[b].priority || procs[a].arrivalTime - procs[b].arrivalTime);
        let idx = ready[0];
        let p = procs[idx];
        if (!p.hasStarted) { p.startTime = time; p.hasStarted = true; }
        mergeEvent(gantt, p.name, time, time + p.burstTime);
        time += p.burstTime;
        p.remainingTime = 0;
        p.completionTime = time;
        p.state = 'TERMINATED';
        calcMetrics(p);
        done[idx] = true;
        completed++;
    }
    return { procs, gantt, metrics: computeStats(procs, gantt) };
}

// Priority preemptive
function schedulePriorityP(inputProcs) {
    const procs = deepCopyProcs(inputProcs);
    const gantt = [];
    let time = 0, completed = 0, n = procs.length;
    while (completed < n) {
        let ready = [];
        for (let i = 0; i < n; i++) {
            if (procs[i].arrivalTime <= time && procs[i].state !== 'TERMINATED') ready.push(i);
        }
        if (ready.length === 0) {
            let next = Infinity;
            for (let i = 0; i < n; i++) if (procs[i].state !== 'TERMINATED') next = Math.min(next, procs[i].arrivalTime);
            mergeEvent(gantt, 'IDLE', time, next);
            time = next;
            continue;
        }
        ready.sort((a,b) => procs[a].priority - procs[b].priority || procs[a].arrivalTime - procs[b].arrivalTime);
        let idx = ready[0];
        let p = procs[idx];
        if (!p.hasStarted) { p.startTime = time; p.hasStarted = true; }
        let nextEvent = time + p.remainingTime;
        for (let i = 0; i < n; i++) {
            if (procs[i].arrivalTime > time && procs[i].state !== 'TERMINATED') {
                nextEvent = Math.min(nextEvent, procs[i].arrivalTime);
            }
        }
        let exec = nextEvent - time;
        mergeEvent(gantt, p.name, time, time + exec);
        p.remainingTime -= exec;
        time += exec;
        if (p.remainingTime === 0) {
            p.completionTime = time;
            p.state = 'TERMINATED';
            calcMetrics(p);
            completed++;
        }
    }
    return { procs, gantt, metrics: computeStats(procs, gantt) };
}

// RR
function scheduleRR(inputProcs, quantum = 2) {
    const procs = deepCopyProcs(inputProcs).sort((a,b) => a.arrivalTime - b.arrivalTime || a.pid - b.pid);
    const gantt = [];
    let time = 0, completed = 0, n = procs.length;
    const queue = [];
    let nextAdmit = 0;
    // Admit initial
    while (nextAdmit < n && procs[nextAdmit].arrivalTime <= time) {
        queue.push(nextAdmit);
        nextAdmit++;
    }
    while (completed < n) {
        if (queue.length === 0) {
            if (nextAdmit < n) {
                mergeEvent(gantt, 'IDLE', time, procs[nextAdmit].arrivalTime);
                time = procs[nextAdmit].arrivalTime;
                while (nextAdmit < n && procs[nextAdmit].arrivalTime <= time) {
                    queue.push(nextAdmit);
                    nextAdmit++;
                }
            }
            continue;
        }
        let idx = queue.shift();
        let p = procs[idx];
        if (!p.hasStarted) { p.startTime = time; p.hasStarted = true; }
        let exec = Math.min(quantum, p.remainingTime);
        mergeEvent(gantt, p.name, time, time + exec);
        time += exec;
        p.remainingTime -= exec;
        // Admit new arrivals before rotating
        while (nextAdmit < n && procs[nextAdmit].arrivalTime <= time) {
            queue.push(nextAdmit);
            nextAdmit++;
        }
        if (p.remainingTime === 0) {
            p.completionTime = time;
            p.state = 'TERMINATED';
            calcMetrics(p);
            completed++;
        } else {
            queue.push(idx);
        }
    }
    return { procs, gantt, metrics: computeStats(procs, gantt) };
}

// HRRN
function scheduleHRRN(inputProcs) {
    const procs = deepCopyProcs(inputProcs);
    const gantt = [];
    let time = 0, completed = 0, n = procs.length;
    const done = new Array(n).fill(false);
    while (completed < n) {
        let ready = [];
        for (let i = 0; i < n; i++) {
            if (!done[i] && procs[i].arrivalTime <= time) ready.push(i);
        }
        if (ready.length === 0) {
            let next = Infinity;
            for (let i = 0; i < n; i++) if (!done[i]) next = Math.min(next, procs[i].arrivalTime);
            mergeEvent(gantt, 'IDLE', time, next);
            time = next;
            continue;
        }
        // Calculate response ratio
        let best = -1, bestRatio = -1;
        for (let i of ready) {
            let w = time - procs[i].arrivalTime;
            let ratio = (w + procs[i].burstTime) / procs[i].burstTime;
            if (ratio > bestRatio || (Math.abs(ratio - bestRatio) < 1e-9 && procs[i].arrivalTime < procs[best].arrivalTime)) {
                bestRatio = ratio;
                best = i;
            }
        }
        let p = procs[best];
        if (!p.hasStarted) { p.startTime = time; p.hasStarted = true; }
        mergeEvent(gantt, p.name, time, time + p.burstTime);
        time += p.burstTime;
        p.remainingTime = 0;
        p.completionTime = time;
        p.state = 'TERMINATED';
        calcMetrics(p);
        done[best] = true;
        completed++;
    }
    return { procs, gantt, metrics: computeStats(procs, gantt) };
}

// MLFQ
function scheduleMLFQ(inputProcs, numQueues = 3, quantums = [1,2,4]) {
    const procs = deepCopyProcs(inputProcs).sort((a,b) => a.arrivalTime - b.arrivalTime || a.pid - b.pid);
    const gantt = [];
    let time = 0, completed = 0, n = procs.length;
    const queues = Array.from({length: numQueues}, () => []);
    let nextAdmit = 0;

    while (nextAdmit < n && procs[nextAdmit].arrivalTime <= time) {
        queues[0].push(nextAdmit);
        nextAdmit++;
    }

    let safetyCounter = 0;
    while (completed < n && safetyCounter < 100000) {
        safetyCounter++;
        // Admit new
        while (nextAdmit < n && procs[nextAdmit].arrivalTime <= time) {
            procs[nextAdmit].currentQueue = 0;
            queues[0].push(nextAdmit);
            nextAdmit++;
        }

        // Find highest non-empty queue
        let activeQ = -1;
        for (let q = 0; q < numQueues; q++) {
            if (queues[q].length > 0) { activeQ = q; break; }
        }

        if (activeQ === -1) {
            if (nextAdmit < n) {
                mergeEvent(gantt, 'IDLE', time, procs[nextAdmit].arrivalTime);
                time = procs[nextAdmit].arrivalTime;
            }
            continue;
        }

        let idx = queues[activeQ].shift();
        let p = procs[idx];
        if (!p.hasStarted) { p.startTime = time; p.hasStarted = true; }

        let quantum = quantums[Math.min(activeQ, quantums.length - 1)];
        let exec = Math.min(quantum, p.remainingTime);

        // Check preemption by new arrival
        if (nextAdmit < n && procs[nextAdmit].arrivalTime > time && procs[nextAdmit].arrivalTime < time + exec) {
            exec = procs[nextAdmit].arrivalTime - time;
        }

        mergeEvent(gantt, p.name, time, time + exec);
        p.remainingTime -= exec;
        time += exec;

        // Admit during execution
        while (nextAdmit < n && procs[nextAdmit].arrivalTime <= time) {
            procs[nextAdmit].currentQueue = 0;
            queues[0].push(nextAdmit);
            nextAdmit++;
        }

        if (p.remainingTime === 0) {
            p.completionTime = time;
            p.state = 'TERMINATED';
            calcMetrics(p);
            completed++;
        } else if (exec < quantum) {
            // Preempted, stay in same queue
            queues[activeQ].push(idx);
        } else {
            // Quantum expired, demote
            let nextQ = Math.min(activeQ + 1, numQueues - 1);
            p.currentQueue = nextQ;
            queues[nextQ].push(idx);
        }
    }
    return { procs, gantt, metrics: computeStats(procs, gantt) };
}

const ALGO_MAP = {
    fcfs:  (p) => scheduleFCFS(p),
    sjf:   (p) => scheduleSJF(p),
    srtf:  (p) => scheduleSRTF(p),
    priNP: (p) => schedulePriorityNP(p),
    priP:  (p) => schedulePriorityP(p),
    rr:    (p) => scheduleRR(p, 2),
    hrrn:  (p) => scheduleHRRN(p),
    mlfq:  (p) => scheduleMLFQ(p, 3, [1, 2, 4])
};


