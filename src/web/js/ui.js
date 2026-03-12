// ==========================
// UI Initialization
// ==========================
function initAlgoChips() {
    const container = document.getElementById('algoChips');
    container.innerHTML = '';
    ALGORITHMS.forEach(algo => {
        const chip = document.createElement('div');
        chip.className = 'algo-chip' + (selectedAlgos.has(algo.id) ? ' selected' : '');
        chip.textContent = algo.name;
        chip.onclick = () => {
            if (selectedAlgos.has(algo.id)) selectedAlgos.delete(algo.id);
            else selectedAlgos.add(algo.id);
            chip.classList.toggle('selected');
        };
        container.appendChild(chip);
    });
}

function selectAllAlgos() {
    ALGORITHMS.forEach(a => selectedAlgos.add(a.id));
    initAlgoChips();
}

// ==========================
// Process Management
// ==========================
function renderProcessTable() {
    const tbody = document.getElementById('processTableBody');
    tbody.innerHTML = '';
    processes.forEach((p, i) => {
        const tr = document.createElement('tr');
        tr.innerHTML = `<td>${p.pid}</td><td>${p.name}</td><td>${p.arrivalTime}</td><td>${p.burstTime}</td><td>${p.priority}</td>
            <td><button class="delete-btn" onclick="deleteProcess(${i})">✕</button></td>`;
        tbody.appendChild(tr);
    });
    document.getElementById('processCount').textContent = processes.length;
}

function addProcess() {
    const name = document.getElementById('pName').value.trim() || ('P' + nextPid);
    const arrival = parseInt(document.getElementById('pArrival').value) || 0;
    const burst = Math.max(1, parseInt(document.getElementById('pBurst').value) || 1);
    const priority = parseInt(document.getElementById('pPriority').value) || 0;
    processes.push({ pid: nextPid++, name, arrivalTime: Math.max(0, arrival), burstTime: burst, priority: Math.max(0, priority) });
    document.getElementById('pName').value = '';
    renderProcessTable();
    showToast('已添加进程 ' + name);
}

function deleteProcess(index) {
    processes.splice(index, 1);
    renderProcessTable();
}

function clearProcesses() {
    processes = [];
    nextPid = 1;
    renderProcessTable();
}

function loadPreset(id) {
    processes = [];
    nextPid = 1;
    PRESETS[id].forEach(p => {
        processes.push({ pid: nextPid++, ...p });
    });
    renderProcessTable();
    showToast('已加载预设用例 ' + id);
}

function addRandomProcesses() {
    for (let i = 0; i < 5; i++) {
        processes.push({
            pid: nextPid,
            name: 'P' + nextPid,
            arrivalTime: Math.floor(Math.random() * 15),
            burstTime: 1 + Math.floor(Math.random() * 12),
            priority: Math.floor(Math.random() * 8)
        });
        nextPid++;
    }
    renderProcessTable();
    showToast('已随机生成 5 个进程');
}

// ==========================
// Load JSON from C++ export
// ==========================
function loadJsonData() {
    document.getElementById('jsonFileInput').click();
}
function handleJsonFile(event) {
    const file = event.target.files[0];
    if (!file) return;
    const reader = new FileReader();
    reader.onload = function(e) {
        try {
            const data = JSON.parse(e.target.result);
            // Load processes
            if (data.processes) {
                processes = data.processes.map(p => ({
                    pid: p.pid, name: p.name, arrivalTime: p.arrivalTime,
                    burstTime: p.burstTime, priority: p.priority
                }));
                nextPid = Math.max(...processes.map(p => p.pid)) + 1;
                renderProcessTable();
            }
            // Load algorithm results
            if (data.algorithms) {
                schedulingResults = data.algorithms.map(a => ({
                    name: a.name,
                    procs: a.results,
                    gantt: a.ganttChart,
                    metrics: a.metrics
                }));
                renderAllResults();
                showToast('已加载 JSON 数据（' + data.algorithms.length + ' 种算法）');
                switchTab('gantt');
            }
        } catch (err) {
            showToast('JSON 解析失败: ' + err.message);
        }
    };
    reader.readAsText(file);
    event.target.value = '';
}

// ==========================
// Run Scheduling
// ==========================
function runScheduling() {
    if (processes.length === 0) {
        showToast('请先添加进程！');
        return;
    }
    if (selectedAlgos.size === 0) {
        showToast('请至少选择一种算法！');
        return;
    }

    schedulingResults = [];
    const inputProcs = processes.map(p => ({...p}));

    ALGORITHMS.forEach(algo => {
        if (selectedAlgos.has(algo.id) && ALGO_MAP[algo.id]) {
            const result = ALGO_MAP[algo.id](inputProcs);
            schedulingResults.push({ name: algo.name, ...result });
        }
    });

    renderAllResults();
    showToast('调度完成！已运行 ' + schedulingResults.length + ' 种算法');
    switchTab('gantt');
}

// ==========================
// Render Results
// ==========================
let chartInstances = [];

function renderAllResults() {
    renderGantt();
    renderComparison();
    renderDetail();
}

function getColorForProcess(name) {
    if (name === 'IDLE') return null;
    // Deterministic color based on process name
    let hash = 0;
    for (let i = 0; i < name.length; i++) hash = name.charCodeAt(i) + ((hash << 5) - hash);
    return COLORS[Math.abs(hash) % COLORS.length];
}

// Build a consistent color map from all unique process names across results
function buildColorMap() {
    const names = new Set();
    schedulingResults.forEach(r => {
        r.gantt.forEach(e => { if (e.process !== 'IDLE') names.add(e.process); });
    });
    const map = {};
    let i = 0;
    [...names].sort().forEach(n => { map[n] = COLORS[i % COLORS.length]; i++; });
    return map;
}

function renderGantt() {
    const container = document.getElementById('ganttContainer');
    if (schedulingResults.length === 0) {
        container.innerHTML = '<h2>📊 Gantt 图可视化</h2><p class="loading">请先运行调度算法</p>';
        return;
    }

    const colorMap = buildColorMap();
    let maxEnd = 0;
    schedulingResults.forEach(r => {
        r.gantt.forEach(e => { if (e.end > maxEnd) maxEnd = e.end; });
    });
    if (maxEnd === 0) maxEnd = 1;

    let html = '<h2>📊 Gantt 图可视化</h2>';

    // Legend
    html += '<div style="display:flex;flex-wrap:wrap;gap:0.8rem;margin-bottom:1.5rem;">';
    Object.entries(colorMap).forEach(([name, color]) => {
        html += `<div style="display:flex;align-items:center;gap:0.3rem;font-size:0.85rem;">
            <div style="width:14px;height:14px;border-radius:3px;background:${color}"></div>${name}</div>`;
    });
    html += `<div style="display:flex;align-items:center;gap:0.3rem;font-size:0.85rem;">
        <div style="width:14px;height:14px;border-radius:3px;background:repeating-linear-gradient(45deg,var(--surface2),var(--surface2) 3px,var(--bg) 3px,var(--bg) 6px)"></div>IDLE</div>`;
    html += '</div>';

    schedulingResults.forEach(result => {
        html += `<div class="gantt-wrapper">`;
        html += `<div class="gantt-label"><strong>${result.name}</strong></div>`;
        html += `<div class="gantt-row" style="position:relative;height:36px;">`;

        result.gantt.forEach(event => {
            const left = (event.start / maxEnd * 100).toFixed(2);
            const width = ((event.end - event.start) / maxEnd * 100).toFixed(2);
            const color = colorMap[event.process] || '';
            const isIdle = event.process === 'IDLE';
            const label = (event.end - event.start) >= maxEnd * 0.03 ? event.process : '';
            const title = `${event.process}: [${event.start}, ${event.end})  时长=${event.end - event.start}`;
            html += `<div class="gantt-bar${isIdle ? ' idle' : ''}" 
                style="left:${left}%;width:${width}%;${isIdle ? '' : 'background:'+color}"
                title="${title}">${label}</div>`;
        });

        html += `</div>`;

        // Timeline ticks
        html += `<div class="gantt-timeline" style="position:relative;height:18px;">`;
        const ticks = new Set();
        result.gantt.forEach(e => { ticks.add(e.start); ticks.add(e.end); });
        // Limit tick density
        const sortedTicks = [...ticks].sort((a,b) => a - b);
        const minGap = maxEnd * 0.025;
        let lastRendered = -Infinity;
        sortedTicks.forEach(t => {
            if (t - lastRendered >= minGap || t === 0 || t === sortedTicks[sortedTicks.length-1]) {
                const left = (t / maxEnd * 100).toFixed(2);
                html += `<span class="gantt-tick" style="left:${left}%">${t}</span>`;
                lastRendered = t;
            }
        });
        html += `</div></div>`;
    });

    container.innerHTML = html;
}

function renderComparison() {
    const container = document.getElementById('compareContainer');
    if (schedulingResults.length === 0) {
        container.innerHTML = '<h2>📈 多算法性能对比</h2><p class="loading">请先运行调度算法</p>';
        return;
    }

    // Destroy old charts
    chartInstances.forEach(c => c.destroy());
    chartInstances = [];

    let html = '<h2>📈 多算法性能对比</h2>';

    // Comparison table
    html += '<div style="overflow-x:auto;margin-bottom:1.5rem"><table class="comp-table"><thead><tr>';
    html += '<th>算法</th><th>平均周转</th><th>平均带权周转</th><th>平均等待</th><th>平均响应</th><th>CPU利用率</th><th>吞吐量</th>';
    html += '</tr></thead><tbody>';

    // Find best for each metric
    const fields = ['avgTurnaroundTime','avgWeightedTurnaround','avgWaitingTime','avgResponseTime','cpuUtilization','throughput'];
    const best = {};
    fields.forEach(f => {
        let bestVal = f === 'cpuUtilization' || f === 'throughput' ? -Infinity : Infinity;
        let bestIdx = 0;
        schedulingResults.forEach((r,i) => {
            const v = r.metrics[f];
            if (f === 'cpuUtilization' || f === 'throughput') {
                if (v > bestVal) { bestVal = v; bestIdx = i; }
            } else {
                if (v < bestVal) { bestVal = v; bestIdx = i; }
            }
        });
        best[f] = bestIdx;
    });

    schedulingResults.forEach((r, ri) => {
        html += '<tr>';
        html += `<td style="text-align:left;font-weight:600">${r.name}</td>`;
        fields.forEach(f => {
            const v = r.metrics[f];
            const isBest = best[f] === ri;
            const formatted = f === 'cpuUtilization' ? v.toFixed(1) + '%' : f === 'throughput' ? v.toFixed(3) : v.toFixed(2);
            html += `<td class="${isBest ? 'best' : ''}">${formatted}</td>`;
        });
        html += '</tr>';
    });
    html += '</tbody></table></div>';

    // Chart containers
    html += '<div class="chart-grid">';
    html += '<div class="chart-box"><canvas id="chartTurnaround"></canvas></div>';
    html += '<div class="chart-box"><canvas id="chartWaiting"></canvas></div>';
    html += '<div class="chart-box"><canvas id="chartResponse"></canvas></div>';
    html += '<div class="chart-box"><canvas id="chartRadar"></canvas></div>';
    html += '</div>';

    container.innerHTML = html;

    // Create charts
    const labels = schedulingResults.map(r => r.name.replace(/\(.*?\)/g, '').trim());
    const bgColors = schedulingResults.map((_, i) => COLORS[i % COLORS.length]);

    function makeBarChart(canvasId, label, dataKey) {
        const ctx = document.getElementById(canvasId);
        if (!ctx) return;
        const c = new Chart(ctx, {
            type: 'bar',
            data: {
                labels,
                datasets: [{
                    label, data: schedulingResults.map(r => parseFloat(r.metrics[dataKey].toFixed(2))),
                    backgroundColor: bgColors.map(c => c + '99'),
                    borderColor: bgColors, borderWidth: 1
                }]
            },
            options: {
                responsive: true,
                plugins: {
                    legend: { display: false },
                    title: { display: true, text: label, color: '#e2e8f0' }
                },
                scales: {
                    x: { ticks: { color: '#94a3b8', font: { size: 10 } }, grid: { color: '#334155' } },
                    y: { ticks: { color: '#94a3b8' }, grid: { color: '#334155' }, beginAtZero: true }
                }
            }
        });
        chartInstances.push(c);
    }

    makeBarChart('chartTurnaround', '平均周转时间', 'avgTurnaroundTime');
    makeBarChart('chartWaiting', '平均等待时间', 'avgWaitingTime');
    makeBarChart('chartResponse', '平均响应时间', 'avgResponseTime');

    // Radar chart
    const radarCtx = document.getElementById('chartRadar');
    if (radarCtx) {
        // Normalize metrics to 0-1 (inverted for "lower is better")
        const metricKeys = ['avgTurnaroundTime','avgWaitingTime','avgResponseTime','avgWeightedTurnaround'];
        const metricLabels = ['周转时间','等待时间','响应时间','带权周转'];
        const maxVals = metricKeys.map(k => Math.max(...schedulingResults.map(r => r.metrics[k]), 0.01));

        const datasets = schedulingResults.map((r, i) => ({
            label: labels[i],
            data: metricKeys.map((k, ki) => parseFloat((1 - r.metrics[k] / maxVals[ki]).toFixed(3))),
            borderColor: COLORS[i % COLORS.length],
            backgroundColor: COLORS[i % COLORS.length] + '22',
            pointBackgroundColor: COLORS[i % COLORS.length],
            borderWidth: 2
        }));

        const rc = new Chart(radarCtx, {
            type: 'radar',
            data: { labels: metricLabels, datasets },
            options: {
                responsive: true,
                plugins: {
                    title: { display: true, text: '综合性能雷达图（越大越好）', color: '#e2e8f0' },
                    legend: { labels: { color: '#94a3b8', font: { size: 10 } } }
                },
                scales: {
                    r: {
                        beginAtZero: true, max: 1,
                        ticks: { display: false },
                        grid: { color: '#334155' },
                        pointLabels: { color: '#94a3b8' }
                    }
                }
            }
        });
        chartInstances.push(rc);
    }
}

function renderDetail() {
    const container = document.getElementById('detailContainer');
    if (schedulingResults.length === 0) {
        container.innerHTML = '<h2>📋 详细调度结果</h2><p class="loading">请先运行调度算法</p>';
        return;
    }

    let html = '<h2>📋 详细调度结果</h2>';

    // Algorithm selector
    html += '<div class="algo-chips" style="margin-bottom:1rem">';
    schedulingResults.forEach((r, i) => {
        html += `<div class="algo-chip selected" onclick="toggleDetail(this, ${i})" data-idx="${i}">${r.name}</div>`;
    });
    html += '</div>';

    schedulingResults.forEach((result, ri) => {
        html += `<div class="detail-block" id="detail-${ri}">`;
        html += `<div class="card" style="margin-bottom:1rem">`;
        html += `<h2 style="font-size:1rem">${result.name}</h2>`;

        // Metric cards
        const m = result.metrics;
        html += '<div class="metric-grid" style="margin-bottom:1rem">';
        html += `<div class="metric-card"><div class="metric-value">${m.avgTurnaroundTime.toFixed(2)}</div><div class="metric-label">平均周转时间</div></div>`;
        html += `<div class="metric-card"><div class="metric-value">${m.avgWaitingTime.toFixed(2)}</div><div class="metric-label">平均等待时间</div></div>`;
        html += `<div class="metric-card"><div class="metric-value">${m.avgResponseTime.toFixed(2)}</div><div class="metric-label">平均响应时间</div></div>`;
        html += `<div class="metric-card"><div class="metric-value">${m.avgWeightedTurnaround.toFixed(2)}</div><div class="metric-label">平均带权周转</div></div>`;
        html += `<div class="metric-card"><div class="metric-value">${m.cpuUtilization.toFixed(1)}%</div><div class="metric-label">CPU 利用率</div></div>`;
        html += `<div class="metric-card"><div class="metric-value">${m.throughput.toFixed(3)}</div><div class="metric-label">吞吐量</div></div>`;
        html += '</div>';

        // Per-process result table
        html += '<div style="overflow-x:auto"><table class="comp-table"><thead><tr>';
        html += '<th>PID</th><th>名称</th><th>到达</th><th>服务</th><th>优先级</th><th>开始</th><th>完成</th><th>周转</th><th>带权周转</th><th>等待</th><th>响应</th>';
        html += '</tr></thead><tbody>';
        result.procs.forEach(p => {
            html += `<tr>
                <td>${p.pid}</td><td>${p.name}</td><td>${p.arrivalTime}</td><td>${p.burstTime}</td>
                <td>${p.priority}</td><td>${p.startTime}</td><td>${p.completionTime}</td>
                <td>${p.turnaroundTime}</td><td>${(p.weightedTurnaround||0).toFixed(2)}</td>
                <td>${p.waitingTime}</td><td>${p.responseTime}</td></tr>`;
        });
        html += '</tbody></table></div>';
        html += '</div></div>';
    });

    container.innerHTML = html;
}

function toggleDetail(chip, idx) {
    chip.classList.toggle('selected');
    const block = document.getElementById('detail-' + idx);
    if (block) block.style.display = chip.classList.contains('selected') ? 'block' : 'none';
}

// ==========================
// Utilities
// ==========================
function switchTab(tabName) {
    document.querySelectorAll('.tab-btn').forEach(btn => {
        btn.classList.toggle('active', btn.dataset.tab === tabName);
    });
    document.querySelectorAll('.tab-content').forEach(tc => {
        tc.classList.toggle('active', tc.id === 'tab-' + tabName);
    });
}

document.querySelectorAll('.tab-btn').forEach(btn => {
    btn.addEventListener('click', () => switchTab(btn.dataset.tab));
});

function showToast(msg) {
    const t = document.getElementById('toast');
    t.textContent = msg;
    t.classList.add('show');
    setTimeout(() => t.classList.remove('show'), 2000);
}

// ==========================
// Init
// ==========================
initAlgoChips();
renderProcessTable();

// Auto-load JSON if available (when opened from C++ export)
fetch('schedule_data.json')
    .then(r => { if (r.ok) return r.json(); throw new Error('no file'); })
    .then(data => {
        if (data.processes) {
            processes = data.processes.map(p => ({
                pid: p.pid, name: p.name, arrivalTime: p.arrivalTime,
                burstTime: p.burstTime, priority: p.priority
            }));
            nextPid = Math.max(...processes.map(p => p.pid)) + 1;
            renderProcessTable();
        }
        if (data.algorithms) {
            schedulingResults = data.algorithms.map(a => ({
                name: a.name,
                procs: a.results,
                gantt: a.ganttChart,
                metrics: a.metrics
            }));
            renderAllResults();
            switchTab('gantt');
            showToast('已自动加载 C++ 导出的调度数据');
        }
    })
    .catch(() => {});

