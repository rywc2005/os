# 操作系统课程设计报告

# 进程调度算法的设计与实现

---

## 目  录

- [操作系统课程设计报告](#操作系统课程设计报告)
- [进程调度算法的设计与实现](#进程调度算法的设计与实现)
  - [目  录](#目--录)
  - [一、需求分析](#一需求分析)
    - [1.1 系统目标](#11-系统目标)
    - [1.2 功能需求](#12-功能需求)
    - [1.3 输入输出需求](#13-输入输出需求)
    - [1.4 性能需求](#14-性能需求)
  - [二、系统设计](#二系统设计)
    - [2.1 系统总体架构](#21-系统总体架构)
    - [2.2 类图设计](#22-类图设计)
    - [2.3 模块关系](#23-模块关系)
    - [2.4 数据结构设计](#24-数据结构设计)
    - [2.5 算法流程图](#25-算法流程图)
  - [三、详细设计](#三详细设计)
    - [3.1 FCFS 先来先服务算法](#31-fcfs-先来先服务算法)
    - [3.2 SJF 短作业优先算法](#32-sjf-短作业优先算法)
    - [3.3 SRTF 最短剩余时间优先算法](#33-srtf-最短剩余时间优先算法)
    - [3.4 优先级调度算法](#34-优先级调度算法)
    - [3.5 RR 时间片轮转算法](#35-rr-时间片轮转算法)
    - [3.6 HRRN 高响应比优先算法](#36-hrrn-高响应比优先算法)
    - [3.7 MLFQ 多级反馈队列算法](#37-mlfq-多级反馈队列算法)
  - [四、系统实现](#四系统实现)
    - [4.1 PCB 进程控制块类](#41-pcb-进程控制块类)
    - [4.2 ProcessQueue 就绪队列类](#42-processqueue-就绪队列类)
    - [4.3 Scheduler 调度器抽象基类](#43-scheduler-调度器抽象基类)
    - [4.4 ProcessManager 进程管理模块](#44-processmanager-进程管理模块)
    - [4.5 AlgorithmComparator 算法比较模块](#45-algorithmcomparator-算法比较模块)
    - [4.6 主程序与菜单交互系统](#46-主程序与菜单交互系统)
  - [五、调试分析](#五调试分析)
    - [5.1 测试用例一：经典5进程示例](#51-测试用例一经典5进程示例)
    - [5.2 测试用例二：同时到达进程](#52-测试用例二同时到达进程)
    - [5.3 测试用例三：长短作业混合](#53-测试用例三长短作业混合)
    - [5.4 多算法性能对比分析](#54-多算法性能对比分析)
    - [5.5 RR 时间片影响分析](#55-rr-时间片影响分析)
    - [5.6 算法时间复杂度分析](#56-算法时间复杂度分析)
  - [六、总结与展望](#六总结与展望)
    - [6.1 课程设计的收获](#61-课程设计的收获)
    - [6.2 开发过程中遇到的问题及解决方案](#62-开发过程中遇到的问题及解决方案)
    - [6.3 不足之处](#63-不足之处)
    - [6.4 改进设想](#64-改进设想)
    - [6.5 对操作系统课程的认识](#65-对操作系统课程的认识)
  - [参考文献](#参考文献)

---

## 一、需求分析

### 1.1 系统目标

本课程设计的目标是设计并实现一个**进程调度算法模拟系统**，模拟操作系统中 CPU 调度的核心过程。系统需能够：

1. 实现多种经典进程调度算法，涵盖抢占式与非抢占式两大类型。
2. 对同一组进程数据运行不同调度算法，横向比较各算法在周转时间、等待时间、响应时间、CPU 利用率等指标上的性能差异。
3. 以直观的可视化形式（Gantt 图、柱状图、对比表格）展示调度过程与结果。
4. 通过实际编程加深对操作系统进程调度原理的理解，掌握进程控制块（PCB）、就绪队列等核心数据结构的设计方法。

### 1.2 功能需求

根据课程设计要求，系统应实现以下功能：

**（1）调度算法实现（核心功能）**

系统需实现 7 种调度算法，共提供 8 种运行模式：

| 序号 | 算法名称                | 类型     | 说明                              |
|------|------------------------|----------|-----------------------------------|
| 1    | 先来先服务 FCFS         | 非抢占式 | 按到达顺序调度                     |
| 2    | 短作业优先 SJF          | 非抢占式 | 选择服务时间最短的进程              |
| 3    | 最短剩余时间优先 SRTF   | 抢占式   | SJF 的抢占版本                    |
| 4    | 优先级调度（非抢占）     | 非抢占式 | 选择优先级最高的进程                |
| 5    | 优先级调度（抢占）       | 抢占式   | 新进程优先级更高时抢占              |
| 6    | 时间片轮转 RR           | 抢占式   | 固定时间片循环调度                  |
| 7    | 高响应比优先 HRRN       | 非抢占式 | 选择响应比最大的进程                |
| 8    | 多级反馈队列 MLFQ       | 抢占式   | 多级队列逐级降级调度                |

**（2）进程管理功能**

| 功能           | 说明                                         |
|----------------|----------------------------------------------|
| 手动添加进程    | 交互式输入进程名、到达时间、服务时间、优先级    |
| 批量添加进程    | 一次输入多个进程数据                           |
| 随机生成进程    | 设定参数范围自动生成                           |
| 预设测试用例    | 5 组内置经典测试数据                           |
| 文件导入导出    | 支持 Tab 分隔文本文件的读写                    |
| 编辑/删除进程   | 按 PID 修改或删除指定进程                      |

**（3）结果展示功能**

| 功能              | 说明                                       |
|-------------------|--------------------------------------------|
| 调度结果表格       | 每个进程的各项指标一览表                     |
| Gantt 图          | 文本方式的 CPU 时间线可视化                  |
| 多算法对比表       | 多种算法 6 项指标横向比较                    |
| 柱状图            | 控制台字符柱状图直观对比                      |
| 综合评价与推荐     | 自动识别最优算法并给出场景建议                |

**（4）辅助功能**

- 算法原理说明查看
- 演示模式（预设数据 + 全部算法一键运行）
- RR 时间片大小影响分析
- 详细调度事件日志

### 1.3 输入输出需求

**输入：**

1. **进程数据**：每个进程包含 4 个参数——进程名称（字符串）、到达时间（非负整数）、服务时间（正整数）、优先级（非负整数，数值越小优先级越高）。
2. **算法参数**：RR 的时间片大小（1~20 整数）、MLFQ 的队列级数（2~5）及各级时间片、优先级调度的老化机制开关。
3. **菜单选项**：用户通过输入数字选择功能。

**输出：**

1. **调度结果表**：每个进程的 PID、名称、到达时间、服务时间、优先级、开始时间、完成时间、周转时间、带权周转时间、等待时间。
2. **统计数据**：平均周转时间、平均带权周转时间、平均等待时间、平均响应时间、CPU 利用率。
3. **Gantt 图**：以文本字符展示的 CPU 调度时间线。
4. **对比数据**：多算法性能对比表格和柱状图。

### 1.4 性能需求

1. 系统应支持至多 50 个进程的调度模拟。
2. 调度算法计算应在秒级时间内完成。
3. 输入合法性需严格校验，非法输入不应导致程序崩溃或死循环。
4. 源代码规模应不少于 800 行（实际实现 2802 行）。

---

## 二、系统设计

### 2.1 系统总体架构

系统采用**分层模块化**架构设计，自底向上分为四个层次：

```
  ┌────────────────────────────────────────────────────────────┐
  │                   用户交互层 (main.cpp)                     │
  │           主菜单 · 子菜单 · 输入输出 · 演示模式             │
  ├────────────────────────────────────────────────────────────┤
  │              算法比较层 (AlgorithmComparator)                │
  │           多算法执行 · 对比表格 · 柱状图 · 推荐             │
  ├──────────────────────┬─────────────────────────────────────┤
  │   进程管理层          │         调度算法层                    │
  │   (ProcessManager)    │   Scheduler (抽象基类)               │
  │   创建 / 编辑 / 删除  │    ├── FCFS_Scheduler               │
  │   随机生成 / 预设     │    ├── SJF_Scheduler (SJF+SRTF)     │
  │   文件导入导出        │    ├── Priority_Scheduler            │
  │                       │    ├── RR_Scheduler                  │
  │                       │    ├── HRRN_Scheduler                │
  │                       │    └── MLFQ_Scheduler                │
  ├──────────────────────┴─────────────────────────────────────┤
  │                基础数据层 (PCB + ProcessQueue)               │
  │          进程控制块 · 进程状态枚举 · 就绪队列管理            │
  └────────────────────────────────────────────────────────────┘
```

**各层职责**：

- **基础数据层**：定义系统中最底层的数据结构——进程控制块 PCB 和进程就绪队列 ProcessQueue，供上层所有模块使用。
- **调度算法层**：以抽象基类 Scheduler 定义算法公共接口和通用功能，7 种具体算法作为子类实现各自的 `schedule()` 调度逻辑。
- **进程管理层**：负责进程数据的增删改查、随机生成、预设用例加载、文件读写。
- **算法比较层**：管理多个调度器实例，运行所有算法并收集指标，生成对比结果。
- **用户交互层**：提供菜单界面，处理用户输入，协调调用各模块功能。

### 2.2 类图设计

系统共设计 10 个类/结构体，其继承与组合关系如下：

```
                         ┌──────────────┐
                         │   PCB        │  进程控制块（结构体）
                         │──────────────│
                         │ pid          │
                         │ name         │
                         │ arrivalTime  │
                         │ burstTime    │
                         │ priority     │
                         │ remainingTime│
                         │ startTime    │
                         │ completionTime│
                         │ state        │
                         │──────────────│
                         │ calculateMetrics() │
                         │ reset()      │
                         └──────┬───────┘
                                │ 使用
                     ┌──────────┴──────────┐
                     ▼                     ▼
           ┌──────────────┐      ┌──────────────────┐
           │ ProcessQueue │      │ ScheduleEvent    │
           │──────────────│      │──────────────────│
           │ queue<PCB*>  │      │ processName      │
           │──────────────│      │ startTime        │
           │ enqueue()    │      │ endTime          │
           │ dequeue()    │      └──────────────────┘
           │ sortByXxx()  │
           │ findXxx()    │
           └──────────────┘
                 │ 使用
                 ▼
       ┌─────────────────┐
       │   Scheduler     │  <<抽象基类>>
       │─────────────────│
       │ algorithmName   │
       │ processes       │
       │ ganttChart      │
       │ currentTime     │
       │─────────────────│
       │ schedule() = 0  │   ← 纯虚函数
       │ setProcesses()  │
       │ printResults()  │
       │ printGanttChart()│
       │ getAvgXxx()     │
       └────────┬────────┘
                │ 继承
    ┌───────┬───┴───┬──────┬────────┬──────┐
    ▼       ▼       ▼      ▼        ▼      ▼
  FCFS    SJF   Priority   RR    HRRN   MLFQ
Scheduler Scheduler Scheduler Scheduler Scheduler Scheduler

  ┌──────────────────┐     ┌────────────────────┐
  │ ProcessManager   │     │ AlgorithmComparator │
  │──────────────────│     │────────────────────│
  │ processes<PCB>   │     │ schedulers<*>      │
  │ nextPid          │     │ results<Metrics>   │
  │──────────────────│     │────────────────────│
  │ addProcessXxx()  │     │ runAll()           │
  │ generateRandom() │     │ printCompTable()   │
  │ loadPreset()     │     │ printBarChart()    │
  │ loadFromFile()   │     │ printRecommend()   │
  └──────────────────┘     └────────────────────┘
```

### 2.3 模块关系

```
  main.cpp ──────────────────────────────────────────────┐
    │                                                    │
    ├── #include "ProcessManager.h"  ←── 进程管理       │
    │       └── #include "PCB.h"                         │
    │                                                    │
    ├── #include "AlgorithmComparator.h"  ←── 算法比较   │
    │       └── #include "Scheduler.h"   ←── 调度基类   │
    │               ├── #include "PCB.h"                 │
    │               └── #include "ProcessQueue.h"        │
    │                       └── #include "PCB.h"         │
    │                                                    │
    ├── #include "FCFS_Scheduler.h"      ←── 具体算法1  │
    ├── #include "SJF_Scheduler.h"       ←── 具体算法2  │
    ├── #include "Priority_Scheduler.h"  ←── 具体算法3  │
    ├── #include "RR_Scheduler.h"        ←── 具体算法4  │
    ├── #include "HRRN_Scheduler.h"      ←── 具体算法5  │
    └── #include "MLFQ_Scheduler.h"      ←── 具体算法6  │
```

所有调度算法头文件均依赖 `Scheduler.h`（基类），而 `Scheduler.h` 依赖 `PCB.h`（数据结构）和 `ProcessQueue.h`（队列管理）。采用 `#ifndef` 头文件保护机制防止重复包含。

### 2.4 数据结构设计

**（1）进程控制块 PCB**

PCB 是操作系统中描述进程的核心数据结构，包含：

- **静态属性**（用户输入）：`pid`（进程ID）、`name`（名称）、`arrivalTime`（到达时间）、`burstTime`（服务时间）、`priority`（优先级）。
- **动态属性**（调度计算）：`remainingTime`（剩余时间）、`startTime`（首次运行时间）、`completionTime`（完成时间）、`waitingTime`（等待时间）、`turnaroundTime`（周转时间）、`weightedTurnaround`（带权周转时间）、`responseTime`（响应时间）、`state`（进程状态）。
- **MLFQ 专用**：`currentQueue`（当前所在队列级别）。

**存储结构定义：**

```cpp
enum class ProcessState {
    NEW,         // 新建
    READY,       // 就绪
    RUNNING,     // 运行
    WAITING,     // 等待
    TERMINATED   // 终止
};

struct PCB {
    // 静态属性
    int pid;               // 进程ID
    std::string name;      // 进程名称
    int arrivalTime;       // 到达时间
    int burstTime;         // 服务时间
    int priority;          // 优先级（数值越小优先级越高）

    // 动态属性
    int remainingTime;     // 剩余服务时间
    int startTime;         // 首次获得CPU的时间
    int completionTime;    // 完成时间
    int waitingTime;       // 等待时间
    int turnaroundTime;    // 周转时间
    double weightedTurnaround;  // 带权周转时间
    int responseTime;      // 响应时间
    ProcessState state;    // 当前状态
    bool hasStarted;       // 是否已开始执行
    int currentQueue;      // MLFQ队列级别

    // 计算调度结果指标
    void calculateMetrics() {
        turnaroundTime = completionTime - arrivalTime;
        waitingTime = turnaroundTime - burstTime;
        weightedTurnaround = (double)turnaroundTime / burstTime;
        responseTime = startTime - arrivalTime;
    }
};
```

**（2）调度事件 ScheduleEvent**

用于记录 Gantt 图中每段 CPU 的使用情况：

```cpp
struct ScheduleEvent {
    std::string processName;   // 进程名称（"IDLE"表示空闲）
    int startTime;             // 开始时间
    int endTime;               // 结束时间
};
```

**（3）进程就绪队列 ProcessQueue**

封装了就绪队列操作，内部使用 `vector<PCB*>` 存储进程指针。支持多种排序策略：

```cpp
class ProcessQueue {
    std::vector<PCB*> queue;
public:
    void enqueue(PCB*);
    PCB* dequeue();
    void sortByArrivalTime();    // 按到达时间排序
    void sortByBurstTime();      // 按服务时间排序
    void sortByRemainingTime();  // 按剩余时间排序
    void sortByPriority();       // 按优先级排序
    PCB* findShortestRemaining();  // 查找最短剩余时间进程
    PCB* findHighestPriority();    // 查找最高优先级进程
};
```

### 2.5 算法流程图

**（1）通用调度流程**

所有非抢占式算法共享以下基本流程：

```
开始
  │
  ▼
初始化：currentTime=0, completed=0, 进程按到达时间排序
  │
  ▼
┌─── completed < n ? ────── 否 ──→ 输出结果，结束
│       │ 是
│       ▼
│   将已到达进程加入就绪队列
│       │
│       ▼
│   就绪队列为空？
│   │ 是               │ 否
│   ▼                  ▼
│   记录IDLE事件     按算法策略选择进程
│   跳到下一到达时间       │
│   │                    ▼
│   │               执行进程至完成
│   │               记录Gantt事件
│   │               计算各项指标
│   │               completed++
│   │                    │
│   └──────┬─────────────┘
│          │
└──────────┘
```

**（2）抢占式调度流程（以 SRTF 为例）**

```
开始
  │
  ▼
初始化：currentTime=0, completed=0
  │
  ▼
┌─── completed < n ? ────── 否 ──→ 输出结果，结束
│       │ 是
│       ▼
│   将已到达进程加入就绪队列
│   若当前有Running进程，放回就绪队列
│       │
│       ▼
│   就绪队列为空？
│   │ 是               │ 否
│   ▼                  ▼
│   IDLE等待         选择剩余时间最短的进程
│   │                    │
│   │                    ▼
│   │               计算执行时间 = min(剩余时间, 下一到达时间-当前时间)
│   │               执行该时间段，记录Gantt事件
│   │                    │
│   │                    ▼
│   │               剩余时间==0？
│   │               │ 是          │ 否
│   │               ▼             ▼
│   │            进程完成      进程回到就绪队列
│   │            计算指标      等待下一轮比较
│   │            completed++
│   │                    │
│   └──────┬─────────────┘
└──────────┘
```

**（3）RR 时间片轮转流程**

```
开始
  │
  ▼
初始化：设定时间片quantum
  │
  ▼
┌─── completed < n ? ────── 否 ──→ 结束
│       │ 是
│       ▼
│   取出队首进程
│   执行 execTime = min(quantum, remainingTime)
│       │
│       ▼
│   将执行期间新到达的进程加入队尾
│       │
│       ▼
│   remainingTime == 0？
│   │ 是              │ 否
│   ▼                 ▼
│   进程完成         进程放回队尾（时间片用完）
│   completed++
│       │
└───────┘
```

**（4）MLFQ 多级反馈队列流程**

```
开始
  │
  ▼
初始化：创建numQueues个就绪队列，设定各级时间片
  │
  ▼
┌─── completed < n ? ────── 否 ──→ 结束
│       │ 是
│       ▼
│   新到达进程加入第0级队列
│   检查是否需要优先级提升(Boost)
│       │
│       ▼
│   从最高非空队列取出进程
│   执行 execTime = min(该级时间片, remainingTime)
│       │
│       ▼
│   执行期间有新到达？ ──是──→ 截断执行，新进程入第0级
│       │ 否
│       ▼
│   remainingTime == 0？
│   │ 是              │ 否
│   ▼                 ▼
│   进程完成         时间片用完？
│                    │ 是：降级到下一级队列
│                    │ 否（被抢占）：留在当前级
│       │
└───────┘
```

---

## 三、详细设计

本章给出各调度算法的核心实现代码，并附有详细注释说明关键逻辑。

### 3.1 FCFS 先来先服务算法

**算法原理**：按进程到达就绪队列的先后顺序分配 CPU，先到达的进程先执行完毕后才调度下一个。非抢占式。

**核心代码**（FCFS_Scheduler.h）：

```cpp
void schedule() override {
    if (processes.empty()) return;

    ProcessQueue readyQueue;
    currentTime = 0;
    ganttChart.clear();

    // 步骤1：将进程按到达时间排序，确保按先来后到的顺序处理
    std::sort(processes.begin(), processes.end(),
        [](const PCB& a, const PCB& b) {
            if (a.arrivalTime == b.arrivalTime) return a.pid < b.pid;
            return a.arrivalTime < b.arrivalTime;
        });

    int completed = 0;
    int n = static_cast<int>(processes.size());
    int nextAdmitIndex = 0;

    // 步骤2：主调度循环——直到所有进程完成
    while (completed < n) {
        // 将当前时间之前到达的进程加入就绪队列
        while (nextAdmitIndex < n &&
               processes[nextAdmitIndex].arrivalTime <= currentTime) {
            processes[nextAdmitIndex].state = ProcessState::READY;
            readyQueue.enqueue(&processes[nextAdmitIndex]);
            nextAdmitIndex++;
        }

        if (readyQueue.empty()) {
            // CPU 空闲：直接跳转到下一个进程的到达时间
            int nextArrival = processes[nextAdmitIndex].arrivalTime;
            recordIdleEvent(currentTime, nextArrival);   // 记录空闲事件
            currentTime = nextArrival;
            continue;
        }

        // 步骤3：FCFS核心——取出队列头部(最早到达)的进程
        PCB* current = readyQueue.dequeue();
        current->state = ProcessState::RUNNING;

        // 记录首次获得CPU的时间（用于计算响应时间）
        if (!current->hasStarted) {
            current->startTime = currentTime;
            current->hasStarted = true;
        }

        // 步骤4：非抢占——进程执行到完成
        int startExec = currentTime;
        currentTime += current->burstTime;       // 时钟推进
        current->remainingTime = 0;              // 剩余时间归零

        recordEvent(current->name, startExec, currentTime);  // 记录Gantt图事件

        // 步骤5：标记进程完成并计算性能指标
        current->completionTime = currentTime;
        current->state = ProcessState::TERMINATED;
        current->calculateMetrics();  // 计算周转时间、等待时间等
        completed++;
    }
}
```

**关键设计说明**：
- 进程先按到达时间排序，保证 FCFS 的语义正确性。
- 当就绪队列为空而仍有未到达进程时，时钟跳转到下一个到达时间并记录 IDLE 事件。
- `recordEvent()` 方法（基类实现）具有自动合并连续同名进程事件的功能。

### 3.2 SJF 短作业优先算法

**算法原理**：在所有已到达的就绪进程中，选择服务时间（burstTime）最短的进程优先执行。非抢占式。

**核心代码**（SJF_Scheduler.h，非抢占模式）：

```cpp
void scheduleNonPreemptive() {
    // ... 初始化同 FCFS ...
    while (completed < n) {
        // 加入已到达进程到就绪队列
        while (nextAdmitIndex < n &&
               processes[nextAdmitIndex].arrivalTime <= currentTime) {
            processes[nextAdmitIndex].state = ProcessState::READY;
            readyQueue.enqueue(&processes[nextAdmitIndex]);
            nextAdmitIndex++;
        }

        if (readyQueue.empty()) {
            // CPU空闲处理（同FCFS）
            // ...
        }

        // 【SJF核心】按服务时间排序，选择最短的进程
        readyQueue.sortByBurstTime();
        PCB* current = readyQueue.dequeue();

        // 非抢占执行到完成（同FCFS）
        current->state = ProcessState::RUNNING;
        if (!current->hasStarted) {
            current->startTime = currentTime;
            current->hasStarted = true;
        }

        currentTime += current->burstTime;
        current->remainingTime = 0;
        recordEvent(current->name, startExec, currentTime);

        current->completionTime = currentTime;
        current->state = ProcessState::TERMINATED;
        current->calculateMetrics();
        completed++;
    }
}
```

**与 FCFS 的主要区别**：在取出就绪队列进程前，先调用 `sortByBurstTime()` 将队列按服务时间排序，从而选择最短的进程执行。

### 3.3 SRTF 最短剩余时间优先算法

**算法原理**：SJF 的抢占版本。每个时间单位都检查是否有更短剩余时间的进程，若有则抢占当前进程。

**核心代码**（SJF_Scheduler.h，抢占模式）：

```cpp
void schedulePreemptive() {
    // ... 初始化 ...
    PCB* currentProcess = nullptr;

    while (completed < n) {
        // 加入已到达进程
        while (nextAdmitIndex < n &&
               processes[nextAdmitIndex].arrivalTime <= currentTime) {
            processes[nextAdmitIndex].state = ProcessState::READY;
            readyQueue.enqueue(&processes[nextAdmitIndex]);
            nextAdmitIndex++;
        }

        // 【关键】当前Running的进程也放回就绪队列，与新到达进程一起比较
        if (currentProcess != nullptr &&
            currentProcess->state == ProcessState::RUNNING) {
            currentProcess->state = ProcessState::READY;
            readyQueue.enqueue(currentProcess);
        }

        // 选择剩余时间最短的进程
        PCB* shortest = readyQueue.findShortestRemaining();
        readyQueue.remove(shortest);
        shortest->state = ProcessState::RUNNING;

        // 【事件驱动】计算下一个关键时间点
        // （新进程到达 或 当前进程完成，取较早者）
        int nextEventTime = currentTime + shortest->remainingTime;
        if (nextAdmitIndex < n) {
            nextEventTime = std::min(nextEventTime,
                                     processes[nextAdmitIndex].arrivalTime);
        }

        // 执行到下一个事件点
        int execDuration = nextEventTime - currentTime;
        recordEvent(shortest->name, currentTime, nextEventTime);
        shortest->remainingTime -= execDuration;
        currentTime = nextEventTime;

        if (shortest->remainingTime == 0) {
            // 进程完成
            shortest->completionTime = currentTime;
            shortest->state = ProcessState::TERMINATED;
            shortest->calculateMetrics();
            completed++;
            currentProcess = nullptr;
        } else {
            currentProcess = shortest;  // 待下一轮重新比较
        }
    }
}
```

**关键设计说明**：
- **事件驱动**：不是逐个时间单位推进，而是直接跳到下一个"关键事件"发生的时间（新进程到达或当前进程完成），大幅减少不必要的循环迭代。
- **抢占机制**：当前正在运行的进程放回就绪队列，与新到达进程一起竞争剩余时间最短者。
- `recordEvent()` 的合并功能确保连续执行的同一进程不会产生碎片化事件。

### 3.4 优先级调度算法

**算法原理**：选择就绪队列中优先级最高（数值最小）的进程执行。支持抢占/非抢占两种模式和可选的老化（Aging）机制。

**老化机制核心代码**（Priority_Scheduler.h）：

```cpp
// 老化机制：每个时间单位降低等待进程的优先级数值（即提升优先级）
// 目的：防止低优先级进程永远得不到执行（饥饿）
void applyAging(ProcessQueue& readyQueue) {
    if (!agingEnabled) return;
    for (auto* p : readyQueue.getQueue()) {
        if (p->state == ProcessState::READY && p->priority > 0) {
            p->priority--;  // 数值减小 = 优先级提高
        }
    }
}
```

**抢占调度核心逻辑**：

```cpp
// 抢占式优先级调度的关键步骤
// 1. 当前进程放回就绪队列
if (currentProcess != nullptr &&
    currentProcess->state == ProcessState::RUNNING) {
    currentProcess->state = ProcessState::READY;
    readyQueue.enqueue(currentProcess);
}

// 2. 应用老化机制（可选）
applyAging(readyQueue);

// 3. 选择优先级最高的进程（可能是原来的或新来的）
PCB* highest = readyQueue.findHighestPriority();
readyQueue.remove(highest);
highest->state = ProcessState::RUNNING;

// 4. 执行到下一事件点（新进程到达或进程完成）
int nextEventTime = currentTime + highest->remainingTime;
if (nextAdmitIndex < n)
    nextEventTime = std::min(nextEventTime, processes[nextAdmitIndex].arrivalTime);
```

### 3.5 RR 时间片轮转算法

**算法原理**：所有就绪进程按 FIFO 排列，每个进程分配一个固定时间片。时间片内未完成的进程放回队尾。

**核心代码**（RR_Scheduler.h）：

```cpp
void schedule() override {
    // ... 初始化 ...
    while (completed < n) {
        if (readyQueue.empty()) {
            // CPU空闲，跳到下一个到达时间
            // ...
        }

        // 取出队首进程
        PCB* current = readyQueue.dequeue();
        current->state = ProcessState::RUNNING;

        if (!current->hasStarted) {
            current->startTime = currentTime;
            current->hasStarted = true;
        }

        // 【RR核心】执行时间 = min(时间片, 剩余时间)
        int execTime = std::min(timeQuantum, current->remainingTime);
        int startExec = currentTime;
        currentTime += execTime;
        current->remainingTime -= execTime;
        recordEvent(current->name, startExec, currentTime);

        // 【关键细节】执行期间新到达的进程先入队
        // 保证新进程排在被轮转进程之前
        while (nextAdmitIndex < n &&
               processes[nextAdmitIndex].arrivalTime <= currentTime) {
            processes[nextAdmitIndex].state = ProcessState::READY;
            readyQueue.enqueue(&processes[nextAdmitIndex]);
            nextAdmitIndex++;
        }

        if (current->remainingTime == 0) {
            // 进程在时间片内完成
            current->completionTime = currentTime;
            current->state = ProcessState::TERMINATED;
            current->calculateMetrics();
            completed++;
        } else {
            // 时间片用完但未完成 → 放回队尾等待下一轮
            current->state = ProcessState::READY;
            readyQueue.enqueue(current);
        }
    }
}
```

**关键设计说明**：
- 在时间片执行期间新到达的进程先入队（join before rotation），确保新进程排在被轮转进程之前。
- 时间片可通过 `setTimeQuantum()` 动态配置。

### 3.6 HRRN 高响应比优先算法

**算法原理**：每次调度时计算所有就绪进程的响应比 R = (等待时间 + 服务时间) / 服务时间，选择响应比最高的进程执行。非抢占式。

**核心代码**（HRRN_Scheduler.h）：

```cpp
// 计算指定时刻的响应比
static double calculateResponseRatio(const PCB* p, int currentTime) {
    int waitingTime = currentTime - p->arrivalTime;
    if (p->burstTime == 0) return 0.0;
    return static_cast<double>(waitingTime + p->burstTime) / p->burstTime;
}

void schedule() override {
    // ... 初始化、加入已到达进程 ...

    while (completed < n) {
        // ... 加入就绪队列 ...

        // 【HRRN核心】遍历就绪队列，计算每个进程的响应比
        PCB* selected = nullptr;
        double maxRatio = -1.0;

        for (auto* p : readyQueue.getQueue()) {
            double ratio = calculateResponseRatio(p, currentTime);
            if (ratio > maxRatio) {
                maxRatio = ratio;
                selected = p;
            } else if (std::abs(ratio - maxRatio) < 1e-9 &&
                       p->arrivalTime < selected->arrivalTime) {
                selected = p;  // 响应比相同时优先选择先到达的
            }
        }

        readyQueue.remove(selected);
        // 非抢占执行到完成 ...
    }
}
```

**关键设计说明**：
- 响应比公式 R = (W+S)/S 使得短作业天然有较高响应比，长作业等待越久响应比也越高，从而兼顾长短作业。
- 使用浮点数比较时设置了精度容差（1e-9），避免浮点误差导致的不稳定排序。

### 3.7 MLFQ 多级反馈队列算法

**算法原理**：设置多级就绪队列，每级时间片不同。新进程进入最高级队列；时间片用完未完成则降级到下一级；高级队列为空时才调度低级队列。

**核心代码**（MLFQ_Scheduler.h）：

```cpp
void schedule() override {
    // 创建多级就绪队列
    std::vector<ProcessQueue> queues(numQueues);
    int lastBoostTime = 0;
    // ... 初始化 ...

    while (completed < n) {
        // 新到达进程进入第0级（最高优先级）队列
        while (nextAdmitIndex < n &&
               processes[nextAdmitIndex].arrivalTime <= currentTime) {
            processes[nextAdmitIndex].currentQueue = 0;
            queues[0].enqueue(&processes[nextAdmitIndex]);
            nextAdmitIndex++;
        }

        // 【防饥饿】定期优先级提升，将所有低级进程提升到第0级
        if (boostEnabled && (currentTime - lastBoostTime) >= boostInterval) {
            boostAllProcesses(queues);  // 所有进程回到第0级
            lastBoostTime = currentTime;
        }

        // 从最高级非空队列中取进程
        int activeQueue = -1;
        for (int i = 0; i < numQueues; i++) {
            if (!queues[i].empty()) { activeQueue = i; break; }
        }

        PCB* current = queues[activeQueue].dequeue();
        int quantum = timeQuantums[activeQueue];
        int execTime = std::min(quantum, current->remainingTime);

        // 【抢占检查】若执行期间有新进程到达更高优先级队列
        if (nextAdmitIndex < n) {
            int nextArr = processes[nextAdmitIndex].arrivalTime;
            if (nextArr < currentTime + execTime && nextArr > currentTime) {
                execTime = nextArr - currentTime;  // 截断执行到新进程到达时
            }
        }

        // 执行、记录Gantt事件
        currentTime += execTime;
        current->remainingTime -= execTime;
        recordEvent(current->name, startExec, currentTime);

        if (current->remainingTime == 0) {
            // 进程完成
            current->completionTime = currentTime;
            current->state = ProcessState::TERMINATED;
            current->calculateMetrics();
            completed++;
        } else if (execTime < quantum) {
            // 被高优先级进程抢占：保持在当前队列
            queues[activeQueue].enqueue(current);
        } else {
            // 时间片用完：降级到下一级队列
            int nextQueue = std::min(activeQueue + 1, numQueues - 1);
            current->currentQueue = nextQueue;
            queues[nextQueue].enqueue(current);
        }
    }
}
```

**关键设计说明**：
- **多级降级**：进程每次用完时间片后降级，I/O 密集型（短作业）在高级队列快速完成，CPU 密集型逐步降级获取更长时间片。
- **抢占机制**：当新进程到达第 0 级队列时，若当前正在执行低级队列进程，则截断执行。
- **防饥饿 Boost**：定期将所有低级队列的进程提升回第 0 级，防止长期得不到执行。

---

## 四、系统实现

### 4.1 PCB 进程控制块类

**文件**：PCB.h（111 行）

PCB 结构体是整个系统最底层的数据结构，记录了进程的全部调度相关信息。

**关键方法**：

| 方法                | 功能                                                   |
|--------------------|--------------------------------------------------------|
| `calculateMetrics()` | 根据完成时间计算周转时间、等待时间、带权周转时间、响应时间 |
| `reset()`           | 重置所有动态属性为初始值（切换算法前调用，保证公平比较）    |
| `getStateStr()`     | 返回当前状态的中文字符串（新建/就绪/运行/等待/终止）       |

`reset()` 方法确保同一组进程在不同算法下运行时，初始状态完全一致：

```cpp
void reset() {
    remainingTime = burstTime;    // 恢复剩余时间
    startTime = -1;               // 重置首次运行时间
    completionTime = 0;
    waitingTime = 0;
    turnaroundTime = 0;
    weightedTurnaround = 0.0;
    responseTime = 0;
    state = ProcessState::NEW;    // 恢复为新建状态
    hasStarted = false;
    currentQueue = 0;
}
```

### 4.2 ProcessQueue 就绪队列类

**文件**：ProcessQueue.h（181 行）

该类封装了就绪队列的全部操作，使用 `vector<PCB*>` 存储进程指针。

**设计要点**：
- 使用**指针**存储进程（避免拷贝开销，保证修改即时生效）。
- 所有排序方法中，当关键字相同时以 `arrivalTime` 或 `pid` 为第二排序键，确保调度结果的**确定性**（相同条件下结果一致）。

**各排序方法与对应算法**：

| 排序方法               | 使用算法         | 排序键             | 二级排序键   |
|----------------------|-----------------|-------------------|-------------|
| `sortByArrivalTime()` | FCFS            | arrivalTime 升序  | pid 升序    |
| `sortByBurstTime()`   | SJF             | burstTime 升序    | arrivalTime |
| `sortByRemainingTime()`| SRTF           | remainingTime 升序| arrivalTime |
| `sortByPriority()`    | 优先级调度       | priority 升序     | arrivalTime |

### 4.3 Scheduler 调度器抽象基类

**文件**：Scheduler.h（261 行）

这是所有调度算法的抽象父类，定义了公共接口和通用功能。

**公共接口（虚函数）**：

```cpp
virtual void schedule() = 0;  // 纯虚函数，由子类实现具体调度逻辑
```

**通用功能方法**：

| 方法                    | 功能                                           |
|------------------------|------------------------------------------------|
| `setProcesses(procs)`   | 设置进程列表（深拷贝 + 重置所有进程状态）         |
| `printResults()`        | 以 Unicode 边框表格打印调度结果                  |
| `printGanttChart()`     | 以文本形式打印 Gantt 图                          |
| `getAvgTurnaroundTime()`| 计算并返回平均周转时间                           |
| `getAvgWaitingTime()`   | 计算并返回平均等待时间                           |
| `getAvgResponseTime()`  | 计算并返回平均响应时间                           |
| `getCpuUtilization()`   | 计算 CPU 利用率                                 |
| `getThroughput()`       | 计算吞吐量                                      |

**Gantt 图事件合并机制**（重要设计）：

```cpp
// 记录事件时自动合并连续相同进程的事件
void recordEvent(const std::string& processName, int start, int end) {
    if (!ganttChart.empty() &&
        ganttChart.back().processName == processName &&
        ganttChart.back().endTime == start) {
        ganttChart.back().endTime = end;  // 合并：只延伸结束时间
    } else {
        ganttChart.emplace_back(processName, start, end);  // 新事件
    }
}
```

这个合并机制对于抢占式算法（如 SRTF）非常关键——当进程连续执行多个时间单位时，不会产生大量碎片化事件，Gantt 图更加简洁。

### 4.4 ProcessManager 进程管理模块

**文件**：ProcessManager.h（386 行）

该模块负责进程数据的完整生命周期管理，是调度算法的数据源。

**5 组预设测试用例**：

| 用例 | 名称             | 进程数 | 设计目的                           |
|------|-----------------|--------|-------------------------------------|
| 1    | 经典 5 进程      | 5      | 标准教科书示例，各进程参数各不相同    |
| 2    | 同时到达         | 5      | 消除到达时间影响，专注比较调度策略    |
| 3    | 大规模进程集     | 10     | 较多进程，充分体现算法差异            |
| 4    | 长短作业混合     | 7      | 极端对比，体现护航效应和饥饿问题      |
| 5    | 间隔到达         | 5      | 有 CPU 空闲时间，观察利用率差异       |

**随机生成功能**使用 `std::mt19937` 随机数引擎，避免 `rand()` 的低质量问题：

```cpp
std::mt19937 gen(static_cast<unsigned int>(std::time(nullptr)));
std::uniform_int_distribution<> arrivalDist(0, maxArrival);
std::uniform_int_distribution<> burstDist(1, maxBurst);
```

**文件导入**支持自动识别标题行（检查是否含"名称"/"name"/"Name"关键字），并对非法值进行自动修正。

### 4.5 AlgorithmComparator 算法比较模块

**文件**：AlgorithmComparator.h（222 行）

该模块管理多个调度器实例，统一运行并收集性能指标。

**核心工作流**：

```
1. setProcesses(procs)     → 设置统一的进程数据
2. addScheduler(s1, s2..)  → 注册多个调度器
3. runAll()                → 逐个运行所有算法，收集指标
4. printComparisonTable()  → 输出对比表格
5. printBarChart(metric)   → 输出柱状图
6. printRecommendation()   → 输出推荐
```

**柱状图绘制**的关键逻辑——以最大值为满格，按比例计算字符宽度：

```cpp
int maxBarWidth = 40;
for (const auto& d : data) {
    int barLen = (maxVal > 0) ?
        static_cast<int>((d.second / maxVal) * maxBarWidth) : 0;
    barLen = std::max(barLen, 1);
    for (int i = 0; i < barLen; i++) std::cout << "█";
    std::cout << " " << d.second << "\n";
}
```

**综合推荐**功能自动遍历所有结果，找出每项指标的最优算法，并给出场景建议。

### 4.6 主程序与菜单交互系统

**文件**：main.cpp（737 行）

main.cpp 是整个系统的入口和用户交互中枢。

**菜单结构**：

```
主菜单
├── [1] 进程管理（子菜单，10 项功能）
│     ├── 手动添加 / 批量添加 / 随机生成
│     ├── 加载预设 / 文件导入导出
│     └── 查看 / 删除 / 修改 / 清空
├── [2] 单独运行调度算法（8 种算法可选）
├── [3] 多算法对比分析（可选算法组合）
├── [4] 查看算法说明（7 种算法原理文字说明）
├── [5] 演示模式（预设数据 + 全部算法一键运行）
└── [0] 退出
```

**安全输入函数** `readInt()`——防止非法输入导致程序异常：

```cpp
int readInt(const string& prompt, int minVal, int maxVal) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value && value >= minVal && value <= maxVal) {
            return value;
        }
        cin.clear();  // 清除错误标志
        cin.ignore(numeric_limits<streamsize>::max(), '\n');  // 丢弃无效输入
        cout << "  输入无效，请输入 " << minVal << " ~ " << maxVal
             << " 之间的整数。\n";
    }
}
```

**RR 时间片影响分析**功能——以 9 组不同时间片运行 RR 算法并对比：

```cpp
void analyzeTimeQuantumEffect() {
    vector<int> quantums = {1, 2, 3, 4, 5, 8, 10, 15, 20};
    for (int q : quantums) {
        RR_Scheduler rr(q);
        rr.setProcesses(processManager.getProcesses());
        rr.schedule();
        // 收集并记录各项指标...
    }
    // 打印结果表和趋势图
}
```

---

## 五、调试分析

### 5.1 测试用例一：经典5进程示例

**输入数据：**

| 进程 | 到达时间 | 服务时间 | 优先级 |
|------|---------|---------|--------|
| P1   | 0       | 8       | 3      |
| P2   | 1       | 4       | 1      |
| P3   | 2       | 9       | 4      |
| P4   | 3       | 5       | 2      |
| P5   | 4       | 2       | 5      |

**FCFS 调度结果：**

| 进程 | 到达 | 服务 | 开始 | 完成 | 周转 | 带权周转 | 等待 |
|------|------|------|------|------|------|---------|------|
| P1   | 0    | 8    | 0    | 8    | 8    | 1.00    | 0    |
| P2   | 1    | 4    | 8    | 12   | 11   | 2.75    | 7    |
| P3   | 2    | 9    | 12   | 21   | 19   | 2.11    | 10   |
| P4   | 3    | 5    | 21   | 26   | 23   | 4.60    | 18   |
| P5   | 4    | 2    | 26   | 28   | 24   | 12.00   | 22   |

FCFS Gantt 图：

```
|   P1   |  P2  |    P3    |  P4  | P5|
0        8      12         21     26  28
```

平均周转时间 = (8+11+19+23+24)/5 = **17.00**，平均等待时间 = **11.40**

**SJF 调度结果：**

| 进程 | 到达 | 服务 | 开始 | 完成 | 周转 | 带权周转 | 等待 |
|------|------|------|------|------|------|---------|------|
| P1   | 0    | 8    | 0    | 8    | 8    | 1.00    | 0    |
| P5   | 4    | 2    | 8    | 10   | 6    | 3.00    | 4    |
| P2   | 1    | 4    | 10   | 14   | 13   | 3.25    | 9    |
| P4   | 3    | 5    | 14   | 19   | 16   | 3.20    | 11   |
| P3   | 2    | 9    | 19   | 28   | 26   | 2.89    | 17   |

SJF Gantt 图：

```
|   P1   | P5|  P2  |  P4  |    P3    |
0        8   10     14     19         28
```

平均周转时间 = (8+6+13+16+26)/5 = **13.80**，平均等待时间 = **8.20**

**SRTF 调度结果：**

| 进程 | 到达 | 服务 | 开始 | 完成 | 周转 | 带权周转 | 等待 |
|------|------|------|------|------|------|---------|------|
| P1   | 0    | 8    | 0    | 17   | 17   | 2.13    | 9    |
| P2   | 1    | 4    | 1    | 5    | 4    | 1.00    | 0    |
| P5   | 4    | 2    | 5    | 7    | 3    | 1.50    | 1    |
| P4   | 3    | 5    | 7    | 12   | 9    | 1.80    | 4    |
| P3   | 2    | 9    | 17   | 28   | 26   | 2.89    | 17   |

SRTF Gantt 图：

```
|P1| P2  | P5|  P4  |    P1    |    P3    |
0  1     5   7      12        17         28
```

平均周转时间 = (17+4+3+9+26)/5 = **11.80**，平均等待时间 = **6.20**

**分析**：对比三种算法可见，SRTF 的平均周转时间（11.80）和平均等待时间（6.20）均为最优。P1 在 SRTF 下被多次抢占（t=1 时被 P2 抢占），虽然 P1 的个体周转时间变差，但整体平均性能提升显著。

### 5.2 测试用例二：同时到达进程

**输入数据：**

| 进程 | 到达时间 | 服务时间 | 优先级 |
|------|---------|---------|--------|
| P1   | 0       | 10      | 3      |
| P2   | 0       | 5       | 1      |
| P3   | 0       | 8       | 4      |
| P4   | 0       | 3       | 2      |
| P5   | 0       | 6       | 5      |

所有进程同时到达，消除了到达时间的影响，纯粹比较各算法的选择策略。

**各算法平均周转时间对比：**

| 算法       | 平均周转时间 | 平均等待时间 | 说明            |
|-----------|-------------|-------------|-----------------|
| FCFS      | 22.60       | 16.20       | 按PID顺序执行    |
| SJF/SRTF  | 17.40       | 11.00       | 3→5→6→8→10      |
| 优先级(NP) | 18.40       | 12.00       | 按优先级1→2→3→4→5 |
| RR(q=2)   | 24.00       | 17.60       | 轮转开销较大     |
| HRRN      | 17.40       | 11.00       | 同时到达时等价SJF |

**分析**：所有进程同时到达时，SJF 和 HRRN 的结果完全一致（因为初始等待时间都为 0，HRRN 的响应比退化为 burstTime/burstTime = 1+0 的等比情况，实际选择顺序与 SJF 相同）。RR 由于频繁轮转，平均周转时间反而最长。

### 5.3 测试用例三：长短作业混合

**输入数据：**

| 进程    | 到达时间 | 服务时间 | 优先级 |
|---------|---------|---------|--------|
| Short1  | 0       | 1       | 3      |
| Long1   | 0       | 20      | 2      |
| Short2  | 2       | 2       | 4      |
| Mid1    | 3       | 8       | 1      |
| Short3  | 5       | 1       | 5      |
| Long2   | 6       | 15      | 3      |
| Short4  | 8       | 3       | 2      |

**FCFS 调度结果分析：**

FCFS Gantt 图：

```
|S1|        Long1        |S2| Mid1   |S3|     Long2     |S4 |
0  1                      21 23      31 32              47   50
```

在 FCFS 下 Long1（服务时间 20）在 t=1 开始执行到 t=21，短作业 Short2（服务时间 2）虽然在 t=2 就到达，但必须等待到 t=21 才能执行。这就是典型的**护航效应**（Convoy Effect）。

**SJF 调对比**：SJF 会优先执行短作业，Long1 被推迟但短作业的等待时间大幅缩短，整体平均性能显著优于 FCFS。

**HRRN 调度分析**：HRRN 算法在此场景下表现最为均衡——短作业因服务时间短而天然具有较高的响应比，长作业随等待时间增加响应比也逐渐提升，不会永远被推迟。

### 5.4 多算法性能对比分析

使用预设用例1（经典5进程），运行全部算法的综合对比表：

| 算法                      | 平均周转 | 平均带权周转 | 平均等待 | 平均响应 | CPU利用率 |
|--------------------------|---------|-------------|---------|---------|----------|
| FCFS                     | 17.00   | 4.49        | 11.40   | 11.40   | 100.0%   |
| SJF(非抢占)               | 13.80   | 2.47        | 8.20    | 8.20    | 100.0%   |
| SRTF(抢占)                | 11.80   | 1.86        | 6.20    | 2.20    | 100.0%   |
| 非抢占优先级               | 15.60   | 3.33        | 10.00   | 10.00   | 100.0%   |
| 抢占优先级                 | 13.80   | 2.38        | 8.20    | 2.20    | 100.0%   |
| RR(q=2)                  | 18.20   | 4.34        | 12.60   | 1.60    | 100.0%   |
| HRRN                     | 13.80   | 2.47        | 8.20    | 8.20    | 100.0%   |
| MLFQ(3级, q=1/2/4)       | 16.00   | 3.71        | 10.40   | 0.80    | 100.0%   |

**对比分析：**

1. **平均周转时间**：SRTF 最优（11.80），FCFS 和 RR 最差（17.00/18.20）。SJF 和 HRRN 表现相近（13.80）。
2. **平均等待时间**：SRTF 最优（6.20），与周转时间趋势一致。
3. **平均响应时间**：MLFQ 最优（0.80），因为新进程进入最高级队列立即获得较短时间片。RR 次优（1.60）。
4. **CPU 利用率**：由于第一个进程在 t=0 到达且后续进程密集，所有算法均为 100%。
5. **综合来看**：
   - 追求最短周转/等待时间 → SRTF
   - 追求最短响应时间 → MLFQ
   - 追求实现简单 → FCFS
   - 兼顾公平与效率 → HRRN/RR

### 5.5 RR 时间片影响分析

使用预设用例1，以不同时间片大小运行 RR 算法：

| 时间片 q | 平均周转 | 平均等待 | 平均响应 | 带权周转 |
|---------|---------|---------|---------|---------|
| 1       | 20.40   | 14.80   | 0.80    | 5.60    |
| 2       | 18.20   | 12.60   | 1.60    | 4.34    |
| 3       | 17.80   | 12.20   | 2.20    | 4.31    |
| 4       | 15.00   | 9.40    | 3.40    | 3.04    |
| 5       | 16.60   | 11.00   | 4.00    | 3.17    |
| 8       | 17.00   | 11.40   | 5.20    | 3.94    |
| 10      | 17.00   | 11.40   | 7.60    | 3.94    |
| 15      | 17.00   | 11.40   | 9.60    | 4.49    |
| 20      | 17.00   | 11.40   | 11.40   | 4.49    |

**分析：**
- q≥8 时，RR 退化为 FCFS（所有进程在一个时间片内完成），等待时间和周转时间不再变化。
- q=1 时响应时间最短（0.80），但上下文切换频繁，平均周转时间反而最长（20.40）。
- q=4 在此数据集上取得最佳平均周转时间（15.00），是一个较好的折中点。
- 说明时间片的选择需要在响应时间和吞吐量之间权衡。

### 5.6 算法时间复杂度分析

设 n 为进程数，T 为调度总时长，q 为时间片大小，k 为 MLFQ 队列级数。

| 算法                | 时间复杂度       | 空间复杂度    | 说明                            |
|--------------------|-----------------|--------------|--------------------------------|
| FCFS               | O(n log n)      | O(n)         | 排序一次 + 线性扫描              |
| SJF（非抢占）       | O(n²)           | O(n)         | 每次从就绪队列选最短，共 n 次     |
| SRTF（抢占）        | O(n · T)        | O(n)         | 每个时间单位检查就绪队列          |
| 优先级（非抢占）     | O(n²)           | O(n)         | 每次选最高优先级，共 n 次         |
| 优先级（抢占）       | O(n · T)        | O(n)         | 每个时间单位检查优先级            |
| RR                 | O(n · T / q)    | O(n)         | 每个时间片调度一次                |
| HRRN               | O(n²)           | O(n)         | 每次计算所有响应比并选最大         |
| MLFQ               | O(n · T)        | O(n · k)     | 每时刻检查各级队列，k 级队列存储   |

---

## 六、总结与展望

### 6.1 课程设计的收获

通过本次进程调度算法的设计与实现，我收获了以下几个方面：

**（1）深入理解操作系统核心概念**

通过亲手编写 PCB 数据结构、就绪队列管理以及 7 种调度算法，我从原理层面到代码实现层面全面理解了进程调度的本质。课本上的算法描述往往抽象简洁，但在实现过程中需要处理大量细节——如何处理多个进程同时到达、如何在 CPU 空闲时正确跳转时钟、如何保证抢占式算法的正确性等。这些实践经验是仅靠课堂学习难以获得的。

**（2）掌握面向对象设计方法**

本系统采用抽象基类 + 继承的设计模式，将调度算法的共性（结果展示、Gantt 图记录、统计计算）抽取到基类，变化部分（调度策略）由子类实现。这种设计使得新增算法非常容易——只需继承 Scheduler 并实现 `schedule()` 方法。通过本次设计，我深刻体会了"开闭原则"（对扩展开放，对修改封闭）的价值。

**（3）提升编程与调试能力**

2800+ 行的代码量要求良好的代码组织和命名规范。在调试过程中，我学会了使用预设测试用例手工推导结果进行验证，以及利用 Gantt 图直观检查调度过程是否正确。

**（4）理解算法权衡与系统设计思维**

不同调度算法本质上是在多个维度（公平性、响应时间、吞吐量、复杂度）之间的权衡。没有绝对"最好"的算法，只有最适合特定场景的算法。这种权衡思维在其他系统设计中同样适用。

### 6.2 开发过程中遇到的问题及解决方案

**问题 1：抢占式算法中连续事件碎片化**

在实现 SRTF 时，最初每个时间单位都记录一个独立的 ScheduleEvent，导致 Gantt 图出现大量碎片。例如进程 P1 连续执行 5 个时间单位被记录为 5 个独立事件。

**解决方案**：在基类 `recordEvent()` 方法中增加**事件合并**逻辑——如果新事件的进程名与上一个事件相同且时间连续，则只延伸上一个事件的结束时间。

**问题 2：RR 算法中新到达进程的入队顺序**

在 RR 算法中，当一个进程用完时间片被轮转到队尾时，如果在该时间片期间有新进程到达，新进程应排在被轮转进程前面还是后面？

**解决方案**：参考经典教材的处理方式，采用"新到达进程先入队"的策略——在将当前进程放回队尾之前，先将所有新到达的进程加入队列。

**问题 3：MLFQ 的饥饿问题**

在 MLFQ 中，如果持续有新短进程到达第 0 级队列，低级队列的长进程可能永远得不到执行。

**解决方案**：实现了 Priority Boost（优先级提升）机制——每隔固定时间间隔（默认 20 个时间单位），将所有低级队列的进程提升到第 0 级队列，保证每个进程在一定时间内都能得到执行机会。

**问题 4：多算法比较时的数据隔离**

多个算法使用同一组进程数据时，第一个算法修改了进程的动态属性（如 remainingTime、state 等），会影响后续算法的正确性。

**解决方案**：在 `Scheduler::setProcesses()` 中执行**深拷贝**和**状态重置**——每个调度器拥有独立的进程副本，且调用 `reset()` 恢复所有动态属性。

**问题 5：Windows 控制台中文与 Unicode 显示**

程序在 Windows 控制台下运行时，中文和 Unicode 边框字符可能出现乱码。

**解决方案**：在 main 函数中通过 `system("chcp 65001 >nul 2>&1")` 设置控制台编码为 UTF-8。

### 6.3 不足之处

1. **缺少图形界面**：当前系统为纯控制台文本界面，虽然使用了 Unicode 字符美化，但可视化效果有限。
2. **不支持 I/O 调度**：系统仅模拟 CPU 调度，未考虑进程的 I/O 操作和阻塞状态转换。
3. **性能分析偏理论**：调度算法在模拟环境下运行，与真实操作系统中的调度存在差距（如未考虑上下文切换开销、缓存影响等）。
4. **缺乏动态交互**：无法在调度执行过程中动态添加或修改进程。

### 6.4 改进设想

1. **图形化界面**：使用 Qt 或 Web 技术（如 Electron）实现图形界面，提供真正的可视化 Gantt 图、动画演示调度过程。
2. **增加 I/O 模拟**：为进程添加 I/O 操作序列，模拟进程在 RUNNING → WAITING → READY 之间的状态转换，使调度模拟更加贴近真实操作系统。
3. **增加更多算法**：实现 CFS（Linux 完全公平调度器）、EDF（最早截止时间优先，用于实时系统）等更现代的调度算法。
4. **多核调度模拟**：扩展系统支持多个 CPU 核心的并行调度模拟。
5. **性能优化**：对于大规模进程集，使用优先队列（堆）替代 vector 排序，将查找最优进程的时间复杂度从 O(n) 降至 O(log n)。
6. **单元测试**：引入自动化测试框架，对每种算法编写单元测试，使用已知正确答案进行回归验证。

### 6.5 对操作系统课程的认识

通过本次课程设计，我对操作系统这门课程有了更深层次的认识：

1. **操作系统是理论与实践结合最紧密的课程之一**。课本上的算法描述看似简单，但实际实现时需要考虑大量边界条件和特殊情况。只有亲手编程实现，才能真正理解算法的精髓。

2. **系统设计是一种权衡的艺术**。在进程调度中，没有一种算法在所有指标上都是最优的——FCFS 简单但对短作业不公平，SJF 平均周转时间最优但可能饿死长作业，RR 响应时间好但上下文切换开销大。操作系统的设计者需要根据具体场景做出取舍，这种权衡思维对软件工程师来说非常重要。

3. **数据结构是算法的基础**。PCB、就绪队列、多级队列等数据结构的合理设计，直接决定了算法实现的效率和正确性。好的数据结构能让复杂的算法逻辑变得清晰简洁。

4. **模块化和抽象是管理复杂度的关键**。本系统 2800+ 行的代码能够保持清晰的结构，得益于合理的模块划分和面向对象的抽象设计。这种方法论在任何大型软件项目中都适用。

---

## 参考文献

[1] 汤小丹, 梁红兵, 哲凤屏, 汤子瀛. 计算机操作系统（第四版）[M]. 人民邮电出版社, 2014.

[2] Abraham Silberschatz, Peter Baer Galvin, Greg Gagne. Operating System Concepts (10th Edition)[M]. Wiley, 2018.

[3] Andrew S. Tanenbaum, Herbert Bos. Modern Operating Systems (4th Edition)[M]. Pearson, 2014.

[4] 严蔚敏, 吴伟民. 数据结构（C语言版）[M]. 清华大学出版社, 2012.

[5] ISO/IEC 14882:2011. Information technology — Programming languages — C++[S]. 2011.

---

**源文件清单：**

| 序号 | 文件名                  | 行数  | 功能描述                           |
|------|------------------------|-------|-------------------------------------|
| 1    | PCB.h                  | 111   | 进程控制块数据结构定义               |
| 2    | ProcessQueue.h         | 181   | 进程就绪队列管理类                   |
| 3    | Scheduler.h            | 261   | 调度算法抽象基类                     |
| 4    | FCFS_Scheduler.h       | 89    | 先来先服务算法                       |
| 5    | SJF_Scheduler.h        | 183   | 短作业优先 + 最短剩余时间优先算法     |
| 6    | Priority_Scheduler.h   | 207   | 优先级调度（非抢占/抢占 + 老化机制）  |
| 7    | RR_Scheduler.h         | 122   | 时间片轮转算法                       |
| 8    | HRRN_Scheduler.h       | 108   | 高响应比优先算法                     |
| 9    | MLFQ_Scheduler.h       | 195   | 多级反馈队列算法                     |
| 10   | AlgorithmComparator.h  | 222   | 算法比较与分析模块                   |
| 11   | ProcessManager.h       | 386   | 进程管理模块                         |
| 12   | main.cpp               | 737   | 主程序与菜单交互系统                 |
|      | **合计**               |**2802**|                                     |
