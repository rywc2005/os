/*======================================================================
 * 项目: 进程调度算法的设计与实现
 * 文件: main.cpp
 * 描述: 操作系统课程设计 —— 进程调度模拟系统主程序
 *
 * 功能概述：
 *   本系统实现了操作系统中多种经典进程调度算法的模拟与比较，包括：
 *   1. 先来先服务(FCFS)调度算法
 *   2. 短作业优先(SJF)调度算法（非抢占式）
 *   3. 最短剩余时间优先(SRTF)调度算法（抢占式SJF）
 *   4. 优先级调度算法（非抢占式/抢占式，可选老化机制）
 *   5. 时间片轮转(RR)调度算法（可配置时间片大小）
 *   6. 高响应比优先(HRRN)调度算法
 *   7. 多级反馈队列(MLFQ)调度算法
 *
 *   系统提供以下辅助功能：
 *   - 进程管理: 手动输入/批量输入/随机生成/预设用例/文件导入导出
 *   - Gantt 图可视化: 以字符方式绘制调度时序图
 *   - 算法对比分析: 多指标横向比较各算法的性能
 *   - 性能统计: 周转时间、等待时间、响应时间、CPU利用率等
 *
 * 开发环境: C++11 及以上
 * 编译方式: g++ -std=c++11 -o scheduler main.cpp
 *======================================================================*/

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <limits>
#include <memory>
#include <sstream>
#include <cstdlib>

// 引入各模块头文件
#include "PCB.h"
#include "ProcessQueue.h"
#include "Scheduler.h"
#include "FCFS_Scheduler.h"
#include "SJF_Scheduler.h"
#include "Priority_Scheduler.h"
#include "RR_Scheduler.h"
#include "HRRN_Scheduler.h"
#include "MLFQ_Scheduler.h"
#include "AlgorithmComparator.h"
#include "ProcessManager.h"
#include "JsonExporter.h"

using namespace std;

// ======================================================================
// 函数前向声明
// ======================================================================
void printDetailedLog(Scheduler* scheduler);
void analyzeTimeQuantumEffect();
void exportWebVisualization();

// ======================================================================
// 全局对象
// ======================================================================
ProcessManager processManager;          // 进程管理器
AlgorithmComparator comparator;         // 算法比较器

// ======================================================================
// 工具函数
// ======================================================================

// 清屏（跨平台）
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// 暂停等待用户按键
void pauseScreen() {
    cout << "\n  按 Enter 键继续...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

// 安全读取整数输入
int readInt(const string& prompt, int minVal = INT_MIN, int maxVal = INT_MAX) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value && value >= minVal && value <= maxVal) {
            return value;
        }
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "  输入无效，请输入 " << minVal << " ~ " << maxVal << " 之间的整数。\n";
    }
}

// ======================================================================
// 显示系统 Banner
// ======================================================================
void showBanner() {
    cout << "\n";
    cout << "  ╔══════════════════════════════════════════════════════════════╗\n";
    cout << "  ║                                                            ║\n";
    cout << "  ║          进 程 调 度 算 法 模 拟 系 统                     ║\n";
    cout << "  ║        Process Scheduling Algorithm Simulator              ║\n";
    cout << "  ║                                                            ║\n";
    cout << "  ║  操作系统课程设计                                          ║\n";
    cout << "  ║                                                            ║\n";
    cout << "  ╚══════════════════════════════════════════════════════════════╝\n";
    cout << "\n";
}

// ======================================================================
// 主菜单
// ======================================================================
void showMainMenu() {
    cout << "\n  ┌────────────────────── 主菜单 ──────────────────────┐\n";
    cout << "  │                                                    │\n";
    cout << "  │  [1] 进程管理（创建/编辑/删除/导入导出）           │\n";
    cout << "  │  [2] 单独运行调度算法                              │\n";
    cout << "  │  [3] 多算法对比分析                                │\n";
    cout << "  │  [4] 查看算法说明                                  │\n";
    cout << "  │  [5] 运行演示模式（预设数据 + 全部算法）           │\n";
    cout << "  │  [6] RR 时间片影响分析                             │\n";
    cout << "  │  [7] 导出 Web 可视化报告                           │\n";
    cout << "  │  [0] 退出系统                                      │\n";
    cout << "  │                                                    │\n";
    cout << "  └────────────────────────────────────────────────────┘\n";
}

// ======================================================================
// 进程管理子菜单
// ======================================================================
void processManagementMenu() {
    while (true) {
        cout << "\n  ┌─────────────── 进程管理 ───────────────┐\n";
        cout << "  │                                        │\n";
        cout << "  │  [1] 手动添加单个进程                  │\n";
        cout << "  │  [2] 批量添加多个进程                  │\n";
        cout << "  │  [3] 随机生成进程                      │\n";
        cout << "  │  [4] 加载预设测试用例                  │\n";
        cout << "  │  [5] 从文件导入进程                    │\n";
        cout << "  │  [6] 导出进程到文件                    │\n";
        cout << "  │  [7] 查看当前进程列表                  │\n";
        cout << "  │  [8] 删除指定进程                      │\n";
        cout << "  │  [9] 修改指定进程                      │\n";
        cout << "  │  [10] 清空所有进程                     │\n";
        cout << "  │  [0] 返回主菜单                        │\n";
        cout << "  │                                        │\n";
        cout << "  └────────────────────────────────────────┘\n";

        int choice = readInt("  请选择: ", 0, 10);

        switch (choice) {
            case 1:
                processManager.addProcessManually();
                break;
            case 2:
                processManager.addMultipleProcesses();
                break;
            case 3:
                processManager.generateRandomProcesses();
                break;
            case 4: {
                cout << "\n  预设测试用例:\n";
                cout << "  [1] 经典5进程示例\n";
                cout << "  [2] 同时到达进程\n";
                cout << "  [3] 较大规模进程集(10个)\n";
                cout << "  [4] 长短作业混合\n";
                cout << "  [5] 间隔到达进程\n";
                int preset = readInt("  请选择预设用例: ", 1, 5);
                processManager.loadPresetProcesses(preset);
                processManager.displayProcesses();
                break;
            }
            case 5: {
                cout << "\n  请输入文件路径: ";
                string filename;
                cin >> filename;
                processManager.loadFromFile(filename);
                processManager.displayProcesses();
                break;
            }
            case 6: {
                if (!processManager.hasProcesses()) {
                    cout << "  当前没有进程数据可导出。\n";
                    break;
                }
                cout << "\n  请输入保存路径: ";
                string filename;
                cin >> filename;
                processManager.saveToFile(filename);
                break;
            }
            case 7:
                processManager.displayProcesses();
                break;
            case 8: {
                processManager.displayProcesses();
                int pid = readInt("  请输入要删除的进程PID: ", 1, 9999);
                processManager.removeProcess(pid);
                break;
            }
            case 9: {
                processManager.displayProcesses();
                int pid = readInt("  请输入要修改的进程PID: ", 1, 9999);
                processManager.editProcess(pid);
                break;
            }
            case 10:
                processManager.clearProcesses();
                cout << "  ✓ 已清空所有进程。\n";
                break;
            case 0:
                return;
        }
    }
}

// ======================================================================
// 单独运行调度算法
// ======================================================================
void runSingleAlgorithm() {
    if (!processManager.hasProcesses()) {
        cout << "\n  ✗ 当前没有进程，请先在进程管理中添加进程。\n";
        return;
    }

    cout << "\n  ┌─────────── 选择调度算法 ───────────┐\n";
    cout << "  │                                    │\n";
    cout << "  │  [1] 先来先服务 (FCFS)             │\n";
    cout << "  │  [2] 短作业优先 (SJF, 非抢占)      │\n";
    cout << "  │  [3] 最短剩余时间优先 (SRTF, 抢占)  │\n";
    cout << "  │  [4] 非抢占式优先级调度             │\n";
    cout << "  │  [5] 抢占式优先级调度               │\n";
    cout << "  │  [6] 时间片轮转 (RR)               │\n";
    cout << "  │  [7] 高响应比优先 (HRRN)           │\n";
    cout << "  │  [8] 多级反馈队列 (MLFQ)           │\n";
    cout << "  │  [0] 返回                          │\n";
    cout << "  │                                    │\n";
    cout << "  └────────────────────────────────────┘\n";

    int choice = readInt("  请选择算法: ", 0, 8);
    if (choice == 0) return;

    // 先显示当前进程列表
    processManager.displayProcesses();

    // 创建对应的调度器并执行
    unique_ptr<Scheduler> scheduler;

    switch (choice) {
        case 1: {
            scheduler.reset(new FCFS_Scheduler());
            break;
        }
        case 2: {
            scheduler.reset(new SJF_Scheduler(false));
            break;
        }
        case 3: {
            scheduler.reset(new SJF_Scheduler(true));
            break;
        }
        case 4: {
            bool aging = false;
            cout << "  是否启用老化机制? (0=否, 1=是): ";
            int a;
            cin >> a;
            aging = (a == 1);
            scheduler.reset(new Priority_Scheduler(false, aging));
            break;
        }
        case 5: {
            bool aging = false;
            cout << "  是否启用老化机制? (0=否, 1=是): ";
            int a;
            cin >> a;
            aging = (a == 1);
            scheduler.reset(new Priority_Scheduler(true, aging));
            break;
        }
        case 6: {
            int quantum = readInt("  请输入时间片大小 (1~20): ", 1, 20);
            scheduler.reset(new RR_Scheduler(quantum));
            break;
        }
        case 7: {
            scheduler.reset(new HRRN_Scheduler());
            break;
        }
        case 8: {
            int queues = readInt("  请输入队列级数 (2~5): ", 2, 5);
            cout << "  请输入每级队列的时间片大小 (用空格分隔):\n  ";
            vector<int> quantums;
            for (int i = 0; i < queues; i++) {
                int q;
                cin >> q;
                if (q <= 0) q = 1;
                quantums.push_back(q);
            }
            auto* mlfq = new MLFQ_Scheduler(queues);
            mlfq->configure(queues, quantums);

            int boostChoice;
            cout << "  是否启用优先级提升(防饥饿)? (0=否, 1=是): ";
            cin >> boostChoice;
            if (boostChoice == 1) {
                int interval = readInt("  优先级提升间隔: ", 5, 100);
                mlfq->setBoost(true, interval);
            } else {
                mlfq->setBoost(false);
            }
            scheduler.reset(mlfq);
            break;
        }
    }

    if (scheduler) {
        // 执行调度
        scheduler->setProcesses(processManager.getProcesses());
        scheduler->schedule();

        // 显示结果
        scheduler->printResults();
        scheduler->printGanttChart();

        // 显示调度过程详细日志
        cout << "\n  是否查看详细调度事件序列? (0=否, 1=是): ";
        int showDetail;
        cin >> showDetail;
        if (showDetail == 1) {
            printDetailedLog(scheduler.get());
        }
    }

    pauseScreen();
}

// 打印详细调度日志
void printDetailedLog(Scheduler* scheduler) {
    const auto& events = scheduler->getGanttChart();
    cout << "\n  ┌────── 详细调度事件日志 ──────┐\n";
    cout << "  │ " << left << setw(10) << "时间段"
         << setw(12) << "执行进程"
         << setw(8) << "时长" << "│\n";
    cout << "  ├──────────────────────────────┤\n";

    for (const auto& event : events) {
        string timeRange = "[" + to_string(event.startTime) + ", "
                           + to_string(event.endTime) + ")";
        string name = (event.processName == "IDLE") ? "空闲" : event.processName;
        int duration = event.endTime - event.startTime;

        cout << "  │ " << left << setw(10) << timeRange
             << setw(12) << name
             << setw(8) << duration << "│\n";
    }
    cout << "  └──────────────────────────────┘\n";
}

// ======================================================================
// 多算法对比分析
// ======================================================================
void runComparison() {
    if (!processManager.hasProcesses()) {
        cout << "\n  ✗ 当前没有进程，请先在进程管理中添加进程。\n";
        return;
    }

    processManager.displayProcesses();
    cout << "\n  选择要对比的算法（输入编号，空格分隔，0结束）:\n";
    cout << "  [1] FCFS  [2] SJF  [3] SRTF  [4] 优先级(非抢占)\n";
    cout << "  [5] 优先级(抢占)  [6] RR  [7] HRRN  [8] MLFQ\n";
    cout << "  [9] 全部算法\n";

    vector<int> selectedAlgos;
    cout << "  请选择: ";

    // 读取用户的算法选择
    string line;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, line);
    istringstream iss(line);
    int num;
    while (iss >> num) {
        if (num == 9) {
            selectedAlgos = {1, 2, 3, 4, 5, 6, 7, 8};
            break;
        }
        if (num >= 1 && num <= 8) {
            selectedAlgos.push_back(num);
        }
        if (num == 0) break;
    }

    if (selectedAlgos.empty()) {
        cout << "  未选择任何算法。\n";
        return;
    }

    // RR 和 MLFQ 的参数
    int rrQuantum = 2;
    int mlfqQueues = 3;
    vector<int> mlfqQuantums = {1, 2, 4};

    // 检查是否需要 RR 参数
    for (int a : selectedAlgos) {
        if (a == 6) {
            rrQuantum = readInt("  RR 时间片大小 (1~20): ", 1, 20);
            break;
        }
    }
    for (int a : selectedAlgos) {
        if (a == 8) {
            mlfqQueues = readInt("  MLFQ 队列级数 (2~5): ", 2, 5);
            cout << "  每级时间片 (空格分隔): ";
            mlfqQuantums.clear();
            for (int i = 0; i < mlfqQueues; i++) {
                int q;
                cin >> q;
                mlfqQuantums.push_back(q > 0 ? q : 1);
            }
            break;
        }
    }

    // 创建调度器列表
    vector<unique_ptr<Scheduler>> schedulerList;

    for (int algo : selectedAlgos) {
        Scheduler* s = nullptr;
        switch (algo) {
            case 1: s = new FCFS_Scheduler(); break;
            case 2: s = new SJF_Scheduler(false); break;
            case 3: s = new SJF_Scheduler(true); break;
            case 4: s = new Priority_Scheduler(false); break;
            case 5: s = new Priority_Scheduler(true); break;
            case 6: s = new RR_Scheduler(rrQuantum); break;
            case 7: s = new HRRN_Scheduler(); break;
            case 8: {
                auto* mlfq = new MLFQ_Scheduler(mlfqQueues);
                mlfq->configure(mlfqQueues, mlfqQuantums);
                s = mlfq;
                break;
            }
        }
        if (s) schedulerList.emplace_back(s);
    }

    // 设置比较器
    comparator.clearSchedulers();
    comparator.setProcesses(processManager.getProcesses());
    for (auto& s : schedulerList) {
        comparator.addScheduler(s.get());
    }

    // 执行对比
    cout << "\n  正在执行各算法调度...\n";
    comparator.runAll();

    // 显示各算法详细结果
    cout << "\n  是否查看各算法详细结果? (0=否, 1=是): ";
    int showAll;
    cin >> showAll;
    if (showAll == 1) {
        comparator.printAllResults();
    }

    // 显示对比表
    comparator.printComparisonTable();

    // 显示柱状图
    cout << "\n  ┌── 性能指标柱状图 ──┐\n";
    comparator.printBarChart("平均周转时间");
    comparator.printBarChart("平均等待时间");
    comparator.printBarChart("平均响应时间");

    // 显示推荐
    comparator.printRecommendation();

    pauseScreen();
}

// ======================================================================
// 算法说明
// ======================================================================
void showAlgorithmInfo() {
    clearScreen();
    cout << R"(
  ╔══════════════════════════════════════════════════════════════════════╗
  ║                  进程调度算法原理说明                              ║
  ╚══════════════════════════════════════════════════════════════════════╝

  ┌─── 1. 先来先服务 (FCFS - First Come First Served) ───┐
  │ 原理: 按进程到达就绪队列的先后顺序分配 CPU              │
  │ 类型: 非抢占式                                         │
  │ 优点: 实现简单、公平                                   │
  │ 缺点: 平均等待时间长、对短作业不利(护航效应)           │
  │ 适用: 批处理系统                                       │
  └────────────────────────────────────────────────────────┘

  ┌─── 2. 短作业优先 (SJF - Shortest Job First) ───┐
  │ 原理: 选择就绪队列中服务时间最短的进程优先执行           │
  │ 类型: 非抢占式                                         │
  │ 优点: 平均等待时间最短(可证明最优)                     │
  │ 缺点: 长作业可能饥饿、需预知服务时间                   │
  │ 适用: 批处理系统                                       │
  └────────────────────────────────────────────────────────┘

  ┌─── 3. 最短剩余时间优先 (SRTF) ───┐
  │ 原理: SJF 的抢占版本，新进程到达时比较剩余时间          │
  │ 类型: 抢占式                                           │
  │ 优点: 平均等待时间更短                                 │
  │ 缺点: 频繁上下文切换、长作业饥饿更严重                 │
  └────────────────────────────────────────────────────────┘

  ┌─── 4. 优先级调度 (Priority Scheduling) ───┐
  │ 原理: 为每个进程设定优先级，优先级高者先执行             │
  │ 类型: 非抢占式 / 抢占式                                │
  │ 优点: 灵活、适应性强                                   │
  │ 缺点: 低优先级进程饥饿（可用老化机制解决）             │
  │ 老化: 等待进程随时间推移优先级逐渐提升                 │
  │ 适用: 实时系统、通用系统                               │
  └────────────────────────────────────────────────────────┘

  ┌─── 5. 时间片轮转 (RR - Round Robin) ───┐
  │ 原理: 所有就绪进程按 FIFO 排列，每个进程执行一个时间片   │
  │       时间片用完未完成则放到队尾等候                       │
  │ 类型: 抢占式                                           │
  │ 优点: 公平、响应时间好                                 │
  │ 缺点: 时间片过大退化为 FCFS，过小则上下文切换开销大     │
  │ 适用: 分时系统                                         │
  └────────────────────────────────────────────────────────┘

  ┌─── 6. 高响应比优先 (HRRN) ───┐
  │ 原理: 响应比 R = (等待时间+服务时间)/服务时间           │
  │       选择响应比最大的进程执行                           │
  │ 类型: 非抢占式                                         │
  │ 优点: 兼顾长短作业、不会饥饿                           │
  │ 缺点: 每次调度需计算响应比，开销略大                   │
  │ 适用: 批处理系统                                       │
  └────────────────────────────────────────────────────────┘

  ┌─── 7. 多级反馈队列 (MLFQ) ───┐
  │ 原理: 设置多个优先级不同的就绪队列，每级时间片不同       │
  │       新进程进入最高级队列，未完成则逐级降级             │
  │       高级队列为空时才调度低级队列                       │
  │ 类型: 抢占式                                           │
  │ 优点: 综合性能好，自动适应进程类型                     │
  │ 缺点: 实现复杂、需要多个参数调优                       │
  │ 防饿: 定期将所有进程提升到最高级队列(Priority Boost)   │
  │ 适用: 通用操作系统（如 Linux CFS 的前身）              │
  └────────────────────────────────────────────────────────┘

  ┌─── 性能评价指标说明 ───┐
  │ 周转时间 = 完成时间 - 到达时间                         │
  │ 带权周转时间 = 周转时间 / 服务时间                     │
  │ 等待时间 = 周转时间 - 服务时间                         │
  │ 响应时间 = 首次运行时间 - 到达时间                     │
  │ CPU利用率 = CPU忙碌时间 / 总时间 × 100%               │
  │ 吞吐量 = 完成进程数 / 总时间                           │
  └────────────────────────────────────────────────────────┘
)";

    pauseScreen();
}

// ======================================================================
// 演示模式
// ======================================================================
void runDemoMode() {
    clearScreen();
    cout << "\n  ═══════ 进入演示模式 ═══════\n";
    cout << "\n  选择演示用例:\n";
    cout << "  [1] 经典5进程示例\n";
    cout << "  [2] 同时到达进程\n";
    cout << "  [3] 较大规模进程集(10个)\n";
    cout << "  [4] 长短作业混合\n";
    cout << "  [5] 间隔到达进程\n";

    int preset = readInt("  请选择: ", 1, 5);
    processManager.loadPresetProcesses(preset);
    processManager.displayProcesses();

    cout << "\n  即将对以上进程运行全部调度算法并比较...\n";

    // 创建所有调度器
    FCFS_Scheduler fcfs;
    SJF_Scheduler sjf(false);
    SJF_Scheduler srtf(true);
    Priority_Scheduler priNonPreempt(false);
    Priority_Scheduler priPreempt(true);
    RR_Scheduler rr2(2);
    RR_Scheduler rr4(4);
    HRRN_Scheduler hrrn;
    MLFQ_Scheduler mlfq(3);
    mlfq.configure(3, {1, 2, 4});

    vector<Scheduler*> allSchedulers = {
        &fcfs, &sjf, &srtf, &priNonPreempt, &priPreempt,
        &rr2, &rr4, &hrrn, &mlfq
    };

    // 设置比较器
    comparator.clearSchedulers();
    comparator.setProcesses(processManager.getProcesses());
    for (auto* s : allSchedulers) {
        comparator.addScheduler(s);
    }

    // 执行所有算法
    comparator.runAll();

    // 逐个显示结果
    for (auto* s : allSchedulers) {
        s->printResults();
        s->printGanttChart();
    }

    // 显示综合对比
    comparator.printComparisonTable();

    // 柱状图
    comparator.printBarChart("平均周转时间");
    comparator.printBarChart("平均等待时间");
    comparator.printBarChart("平均响应时间");
    comparator.printBarChart("CPU利用率");

    // 推荐
    comparator.printRecommendation();

    pauseScreen();
}

// ======================================================================
// 时间片大小影响分析（RR 特有功能）
// ======================================================================
void analyzeTimeQuantumEffect() {
    if (!processManager.hasProcesses()) {
        cout << "\n  ✗ 当前没有进程，请先添加进程。\n";
        return;
    }

    cout << "\n  ═══ 时间片大小对 RR 算法性能的影响分析 ═══\n\n";

    vector<int> quantums = {1, 2, 3, 4, 5, 8, 10, 15, 20};
    vector<AlgorithmMetrics> rrResults;

    for (int q : quantums) {
        RR_Scheduler rr(q);
        rr.setProcesses(processManager.getProcesses());
        rr.schedule();

        AlgorithmMetrics m;
        m.name = "RR(q=" + to_string(q) + ")";
        m.avgTurnaroundTime = rr.getAvgTurnaroundTime();
        m.avgWeightedTurnaround = rr.getAvgWeightedTurnaround();
        m.avgWaitingTime = rr.getAvgWaitingTime();
        m.avgResponseTime = rr.getAvgResponseTime();
        m.cpuUtilization = rr.getCpuUtilization();
        m.throughput = rr.getThroughput();
        rrResults.push_back(m);
    }

    // 打印结果表
    cout << "  时间片 │ 平均周转 │ 平均等待 │ 平均响应 │ 带权周转\n";
    cout << "  ───────┼──────────┼──────────┼──────────┼─────────\n";
    for (const auto& r : rrResults) {
        cout << "  " << setw(6) << r.name.substr(3, r.name.length() - 4)
             << " │ " << setw(8) << fixed << setprecision(2) << r.avgTurnaroundTime
             << " │ " << setw(8) << r.avgWaitingTime
             << " │ " << setw(8) << r.avgResponseTime
             << " │ " << setw(8) << r.avgWeightedTurnaround
             << "\n";
    }

    // 趋势图
    cout << "\n  平均响应时间随时间片变化趋势:\n";
    double maxResp = 0;
    for (const auto& r : rrResults)
        maxResp = max(maxResp, r.avgResponseTime);

    for (const auto& r : rrResults) {
        int barLen = (maxResp > 0) ?
            static_cast<int>((r.avgResponseTime / maxResp) * 30) : 0;
        barLen = max(barLen, 0);
        cout << "  " << setw(10) << r.name << " │";
        for (int i = 0; i < barLen; i++) cout << "▓";
        cout << " " << fixed << setprecision(2) << r.avgResponseTime << "\n";
    }

    pauseScreen();
}

// ======================================================================
// 导出 Web 可视化报告
// ======================================================================
void exportWebVisualization() {
    if (!processManager.hasProcesses()) {
        cout << "\n  ✗ 当前没有进程，请先在进程管理中添加进程。\n";
        pauseScreen();
        return;
    }

    processManager.displayProcesses();

    cout << "\n  将对当前进程运行全部算法并生成 Web 可视化报告...\n\n";

    // 创建全部调度器
    FCFS_Scheduler fcfs;
    SJF_Scheduler sjf(false);
    SJF_Scheduler srtf(true);
    Priority_Scheduler priNP(false);
    Priority_Scheduler priP(true);
    RR_Scheduler rr2(2);
    RR_Scheduler rr4(4);
    HRRN_Scheduler hrrn;
    MLFQ_Scheduler mlfq(3);
    mlfq.configure(3, {1, 2, 4});

    vector<Scheduler*> allSchedulers = {
        &fcfs, &sjf, &srtf, &priNP, &priP,
        &rr2, &rr4, &hrrn, &mlfq
    };

    // 执行所有算法
    for (auto* s : allSchedulers) {
        s->setProcesses(processManager.getProcesses());
        s->schedule();
    }

    // 导出 JSON 并打开浏览器
    string jsonPath = "web/schedule_data.json";
    string htmlPath = "web/index.html";
    JsonExporter::exportAndOpen(allSchedulers, processManager.getProcesses(),
                                jsonPath, htmlPath);

    pauseScreen();
}

// ======================================================================
// 主程序入口
// ======================================================================
int main() {
    // 设置控制台编码（Windows UTF-8）
#ifdef _WIN32
    system("chcp 65001 >nul 2>&1");
#endif

    showBanner();

    while (true) {
        showMainMenu();
        int choice = readInt("  请选择功能: ", 0, 7);

        switch (choice) {
            case 1:
                processManagementMenu();
                break;
            case 2:
                runSingleAlgorithm();
                break;
            case 3:
                runComparison();
                break;
            case 4:
                showAlgorithmInfo();
                break;
            case 5:
                runDemoMode();
                break;
            case 6:
                analyzeTimeQuantumEffect();
                break;
            case 7:
                exportWebVisualization();
                break;
            case 0:
                cout << "\n  感谢使用进程调度算法模拟系统！再见！\n\n";
                return 0;
        }
    }

    return 0;
}
