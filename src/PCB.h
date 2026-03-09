/*======================================================================
 * 文件: PCB.h
 * 描述: 进程控制块(PCB)定义 —— 操作系统进程调度的核心数据结构
 * 作者: 操作系统课程设计
 * 说明: PCB 记录进程的全部调度相关信息，包括到达时间、服务时间、
 *       优先级等静态属性以及完成时间、等待时间等动态计算结果。
 *======================================================================*/
#ifndef PCB_H
#define PCB_H

#include <string>
#include <iostream>
#include <iomanip>

// 进程状态枚举
enum class ProcessState {
    NEW,        // 新建
    READY,      // 就绪
    RUNNING,    // 运行
    WAITING,    // 等待/阻塞
    TERMINATED  // 终止
};

// 调度事件记录（用于 Gantt 图绘制）
struct ScheduleEvent {
    std::string processName; // 进程名称（空表示 CPU 空闲）
    int startTime;           // 事件开始时间
    int endTime;             // 事件结束时间

    ScheduleEvent() : processName(""), startTime(0), endTime(0) {}
    ScheduleEvent(const std::string& name, int start, int end)
        : processName(name), startTime(start), endTime(end) {}
};

// 进程控制块
struct PCB {
    // ---- 静态属性（用户输入）----
    int pid;                  // 进程ID
    std::string name;         // 进程名称
    int arrivalTime;          // 到达时间
    int burstTime;            // CPU 区间时间（服务时间）
    int priority;             // 优先级（数值越小优先级越高）

    // ---- 动态属性（调度过程中计算）----
    int remainingTime;        // 剩余服务时间
    int startTime;            // 首次获得 CPU 的时间
    int completionTime;       // 完成时间
    int waitingTime;          // 等待时间
    int turnaroundTime;       // 周转时间
    double weightedTurnaround;// 带权周转时间
    int responseTime;         // 响应时间
    ProcessState state;       // 当前状态
    bool hasStarted;          // 是否已经开始执行

    // ---- 多级反馈队列专用 ----
    int currentQueue;         // 当前所在队列级别

    // 默认构造函数
    PCB()
        : pid(0), name(""), arrivalTime(0), burstTime(0), priority(0),
          remainingTime(0), startTime(-1), completionTime(0),
          waitingTime(0), turnaroundTime(0), weightedTurnaround(0.0),
          responseTime(0), state(ProcessState::NEW), hasStarted(false),
          currentQueue(0) {}

    // 参数化构造函数
    PCB(int id, const std::string& n, int arrival, int burst, int prio)
        : pid(id), name(n), arrivalTime(arrival), burstTime(burst),
          priority(prio), remainingTime(burst), startTime(-1),
          completionTime(0), waitingTime(0), turnaroundTime(0),
          weightedTurnaround(0.0), responseTime(0),
          state(ProcessState::NEW), hasStarted(false), currentQueue(0) {}

    // 计算调度结果指标
    void calculateMetrics() {
        turnaroundTime = completionTime - arrivalTime;
        waitingTime = turnaroundTime - burstTime;
        if (burstTime > 0) {
            weightedTurnaround = static_cast<double>(turnaroundTime) / burstTime;
        }
        responseTime = startTime - arrivalTime;
    }

    // 重置动态属性（切换算法前重置）
    void reset() {
        remainingTime = burstTime;
        startTime = -1;
        completionTime = 0;
        waitingTime = 0;
        turnaroundTime = 0;
        weightedTurnaround = 0.0;
        responseTime = 0;
        state = ProcessState::NEW;
        hasStarted = false;
        currentQueue = 0;
    }

    // 获取状态的字符串表示
    std::string getStateStr() const {
        switch (state) {
            case ProcessState::NEW:        return "新建";
            case ProcessState::READY:      return "就绪";
            case ProcessState::RUNNING:    return "运行";
            case ProcessState::WAITING:    return "等待";
            case ProcessState::TERMINATED: return "终止";
            default:                       return "未知";
        }
    }
};

#endif // PCB_H
