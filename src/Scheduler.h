/*======================================================================
 * 文件: Scheduler.h
 * 描述: 调度算法抽象基类
 * 说明: 定义所有调度算法的公共接口和通用功能，包括结果展示、
 *       Gantt 图记录等。各具体算法继承此类并实现 schedule() 方法。
 *======================================================================*/
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "PCB.h"
#include "ProcessQueue.h"
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <numeric>
#include <cmath>

class Scheduler {
protected:
    std::string algorithmName;             // 算法名称
    std::vector<PCB> processes;            // 进程列表（副本）
    std::vector<ScheduleEvent> ganttChart; // Gantt 图事件序列
    int currentTime;                       // 当前模拟时钟

    // 将已到达的新进程加入就绪队列
    void admitNewProcesses(ProcessQueue& readyQueue, int time) {
        for (auto& p : processes) {
            if (p.arrivalTime <= time && p.state == ProcessState::NEW) {
                p.state = ProcessState::READY;
                readyQueue.enqueue(&p);
            }
        }
    }

    // 检查是否所有进程都已完成
    bool allCompleted() const {
        for (const auto& p : processes) {
            if (p.state != ProcessState::TERMINATED) return false;
        }
        return true;
    }

    // 获取下一个到达时间（用于 CPU 空闲时跳转）
    int getNextArrivalTime(int afterTime) const {
        int minArrival = INT_MAX;
        for (const auto& p : processes) {
            if (p.arrivalTime > afterTime && p.state == ProcessState::NEW) {
                minArrival = std::min(minArrival, p.arrivalTime);
            }
        }
        return minArrival;
    }

    // 记录 Gantt 图事件（合并连续相同进程的事件）
    void recordEvent(const std::string& processName, int start, int end) {
        if (!ganttChart.empty() &&
            ganttChart.back().processName == processName &&
            ganttChart.back().endTime == start) {
            ganttChart.back().endTime = end;
        } else {
            ganttChart.emplace_back(processName, start, end);
        }
    }

    // 记录 CPU 空闲事件
    void recordIdleEvent(int start, int end) {
        if (start < end) {
            recordEvent("IDLE", start, end);
        }
    }

public:
    Scheduler(const std::string& name) : algorithmName(name), currentTime(0) {}
    virtual ~Scheduler() {}

    // 计算 UTF-8 字符串在终端的显示宽度（CJK字符占2列）
    static int displayWidth(const std::string& s) {
        int width = 0;
        for (size_t i = 0; i < s.size(); ) {
            unsigned char c = s[i];
            if (c < 0x80) { width += 1; i += 1; }
            else if (c < 0xE0) { width += 1; i += 2; }
            else if (c < 0xF0) { width += 2; i += 3; }
            else { width += 2; i += 4; }
        }
        return width;
    }

    // 设置进程列表
    void setProcesses(const std::vector<PCB>& procs) {
        processes = procs;
        // 重置所有进程状态
        for (auto& p : processes) {
            p.reset();
        }
        ganttChart.clear();
        currentTime = 0;
    }

    // 纯虚函数 - 执行调度（由子类实现）
    virtual void schedule() = 0;

    // 获取算法名称
    std::string getName() const { return algorithmName; }

    // 获取 Gantt 图事件序列
    const std::vector<ScheduleEvent>& getGanttChart() const { return ganttChart; }

    // 获取调度后的进程列表
    const std::vector<PCB>& getProcesses() const { return processes; }

    // =========== 结果输出功能 ===========

    // 打印调度结果表格
    void printResults() const {
        std::cout << "\n╔══════════════════════════════════════════════════════"
                  << "══════════════════════════════════════╗\n";
        int titlePad = std::max(1, 72 - displayWidth(algorithmName));
        std::cout << "║           " << algorithmName << " 调度结果"
                  << std::string(titlePad, ' ') << "║\n";
        std::cout << "╠══════════════════════════════════════════════════════"
                  << "══════════════════════════════════════╣\n";

        // 表头
        std::cout << "║ " << std::left
                  << std::setw(6)  << "PID"
                  << std::setw(8)  << "名称"
                  << std::setw(10) << "到达时间"
                  << std::setw(10) << "服务时间"
                  << std::setw(8)  << "优先级"
                  << std::setw(10) << "开始时间"
                  << std::setw(10) << "完成时间"
                  << std::setw(10) << "周转时间"
                  << std::setw(12) << "带权周转"
                  << std::setw(10) << "等待时间"
                  << "║\n";
        std::cout << "╠══════════════════════════════════════════════════════"
                  << "══════════════════════════════════════╣\n";

        // 每行进程数据
        for (const auto& p : processes) {
            std::cout << "║ " << std::left
                      << std::setw(6)  << p.pid
                      << std::setw(8)  << p.name
                      << std::setw(10) << p.arrivalTime
                      << std::setw(10) << p.burstTime
                      << std::setw(8)  << p.priority
                      << std::setw(10) << p.startTime
                      << std::setw(10) << p.completionTime
                      << std::setw(10) << p.turnaroundTime
                      << std::setw(12) << std::fixed << std::setprecision(2)
                                       << p.weightedTurnaround
                      << std::setw(10) << p.waitingTime
                      << "║\n";
        }

        std::cout << "╠══════════════════════════════════════════════════════"
                  << "══════════════════════════════════════╣\n";

        // 统计信息
        double avgTurnaround = getAvgTurnaroundTime();
        double avgWeighted = getAvgWeightedTurnaround();
        double avgWaiting = getAvgWaitingTime();
        double avgResponse = getAvgResponseTime();
        double cpuUtilization = getCpuUtilization();

        std::cout << "║  平均周转时间: " << std::fixed << std::setprecision(2)
                  << avgTurnaround;
        std::cout << "    平均带权周转时间: " << avgWeighted;
        std::cout << "    平均等待时间: " << avgWaiting << std::string(14, ' ') << "║\n";
        std::cout << "║  平均响应时间: " << avgResponse;
        std::cout << "    CPU利用率: " << std::setprecision(1)
                  << cpuUtilization << "%" << std::string(40, ' ') << "║\n";

        std::cout << "╚══════════════════════════════════════════════════════"
                  << "══════════════════════════════════════╝\n";
    }

    // 打印 Gantt 图
    void printGanttChart() const {
        std::cout << "\n┌─── " << algorithmName << " Gantt 图 ───┐\n";
        if (ganttChart.empty()) {
            std::cout << "  (无调度事件)\n";
            return;
        }

        // 第一行：进程名条
        std::cout << " ";
        for (const auto& event : ganttChart) {
            int duration = event.endTime - event.startTime;
            int width = std::max(duration * 3, 3);
            std::string label = event.processName;
            if (label == "IDLE") label = "空闲";

            int padding = width - displayWidth(label);
            int leftPad = padding / 2;
            int rightPad = padding - leftPad;

            std::cout << "|";
            std::cout << std::string(std::max(leftPad, 0), ' ');
            std::cout << label;
            std::cout << std::string(std::max(rightPad, 0), ' ');
        }
        std::cout << "|\n";

        // 第二行：时间边界
        std::cout << " ";
        for (const auto& event : ganttChart) {
            int duration = event.endTime - event.startTime;
            int width = std::max(duration * 3, 3);
            std::string timeStr = std::to_string(event.startTime);
            std::cout << timeStr << std::string(std::max(width + 1 -
                         static_cast<int>(timeStr.length()), 1), ' ');
        }
        // 最后一个结束时间
        if (!ganttChart.empty()) {
            std::cout << ganttChart.back().endTime;
        }
        std::cout << "\n";
    }

    // =========== 统计计算功能 ===========

    double getAvgTurnaroundTime() const {
        if (processes.empty()) return 0.0;
        double sum = 0;
        for (const auto& p : processes) sum += p.turnaroundTime;
        return sum / processes.size();
    }

    double getAvgWeightedTurnaround() const {
        if (processes.empty()) return 0.0;
        double sum = 0;
        for (const auto& p : processes) sum += p.weightedTurnaround;
        return sum / processes.size();
    }

    double getAvgWaitingTime() const {
        if (processes.empty()) return 0.0;
        double sum = 0;
        for (const auto& p : processes) sum += p.waitingTime;
        return sum / processes.size();
    }

    double getAvgResponseTime() const {
        if (processes.empty()) return 0.0;
        double sum = 0;
        for (const auto& p : processes) sum += p.responseTime;
        return sum / processes.size();
    }

    double getCpuUtilization() const {
        if (ganttChart.empty()) return 0.0;
        int totalTime = ganttChart.back().endTime - ganttChart.front().startTime;
        if (totalTime == 0) return 0.0;
        int busyTime = 0;
        for (const auto& event : ganttChart) {
            if (event.processName != "IDLE") {
                busyTime += (event.endTime - event.startTime);
            }
        }
        return (static_cast<double>(busyTime) / totalTime) * 100.0;
    }

    // 获取吞吐量
    double getThroughput() const {
        if (ganttChart.empty()) return 0.0;
        int totalTime = ganttChart.back().endTime - ganttChart.front().startTime;
        if (totalTime == 0) return 0.0;
        return static_cast<double>(processes.size()) / totalTime;
    }
};

#endif // SCHEDULER_H
