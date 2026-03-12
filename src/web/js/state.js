// ==========================
// State
// ==========================
let processes = [];
let nextPid = 1;
let schedulingResults = [];

const COLORS = [
    '#38bdf8','#818cf8','#4ade80','#fbbf24','#f87171',
    '#a78bfa','#fb923c','#2dd4bf','#e879f9','#f472b6',
    '#34d399','#60a5fa','#facc15','#c084fc','#fb7185'
];

const ALGORITHMS = [
    { id:'fcfs',    name:'先来先服务(FCFS)',         type:'non-preemptive' },
    { id:'sjf',     name:'短作业优先(SJF)',           type:'non-preemptive' },
    { id:'srtf',    name:'最短剩余时间优先(SRTF)',    type:'preemptive' },
    { id:'priNP',   name:'优先级调度(非抢占)',        type:'non-preemptive' },
    { id:'priP',    name:'优先级调度(抢占)',          type:'preemptive' },
    { id:'rr',      name:'时间片轮转(RR, q=2)',      type:'preemptive' },
    { id:'hrrn',    name:'高响应比优先(HRRN)',        type:'non-preemptive' },
    { id:'mlfq',    name:'多级反馈队列(MLFQ)',        type:'preemptive' }
];

let selectedAlgos = new Set(['fcfs','sjf','srtf','rr','hrrn']);

// ==========================
// Presets
// ==========================
const PRESETS = {
    1: [
        {name:'P1',arrivalTime:0,burstTime:8,priority:3},
        {name:'P2',arrivalTime:1,burstTime:4,priority:1},
        {name:'P3',arrivalTime:2,burstTime:9,priority:4},
        {name:'P4',arrivalTime:3,burstTime:5,priority:2},
        {name:'P5',arrivalTime:4,burstTime:2,priority:5}
    ],
    2: [
        {name:'P1',arrivalTime:0,burstTime:10,priority:3},
        {name:'P2',arrivalTime:0,burstTime:5,priority:1},
        {name:'P3',arrivalTime:0,burstTime:8,priority:4},
        {name:'P4',arrivalTime:0,burstTime:3,priority:2},
        {name:'P5',arrivalTime:0,burstTime:6,priority:5}
    ],
    3: [
        {name:'A',arrivalTime:0,burstTime:6,priority:3},
        {name:'B',arrivalTime:1,burstTime:3,priority:5},
        {name:'C',arrivalTime:2,burstTime:8,priority:1},
        {name:'D',arrivalTime:3,burstTime:2,priority:4},
        {name:'E',arrivalTime:5,burstTime:4,priority:2},
        {name:'F',arrivalTime:6,burstTime:7,priority:6},
        {name:'G',arrivalTime:8,burstTime:5,priority:3},
        {name:'H',arrivalTime:10,burstTime:1,priority:7},
        {name:'I',arrivalTime:12,burstTime:9,priority:2},
        {name:'J',arrivalTime:15,burstTime:3,priority:4}
    ],
    4: [
        {name:'Short1',arrivalTime:0,burstTime:1,priority:3},
        {name:'Long1',arrivalTime:0,burstTime:20,priority:2},
        {name:'Short2',arrivalTime:2,burstTime:2,priority:4},
        {name:'Mid1',arrivalTime:3,burstTime:8,priority:1},
        {name:'Short3',arrivalTime:5,burstTime:1,priority:5},
        {name:'Long2',arrivalTime:6,burstTime:15,priority:3},
        {name:'Short4',arrivalTime:8,burstTime:3,priority:2}
    ],
    5: [
        {name:'P1',arrivalTime:0,burstTime:4,priority:2},
        {name:'P2',arrivalTime:5,burstTime:3,priority:1},
        {name:'P3',arrivalTime:10,burstTime:5,priority:3},
        {name:'P4',arrivalTime:15,burstTime:2,priority:4},
        {name:'P5',arrivalTime:20,burstTime:6,priority:2}
    ]
};


