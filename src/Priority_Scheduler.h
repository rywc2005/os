/*======================================================================
 * 文件: Priority_Scheduler.h
 * 描述: 优先级调度算法实现，支持非抢占式和抢占式两种模式
 * 算法原理：
 *   为每个进程赋予一个优先级数值（本系统中数值越小优先级越高）。
 *   - 非抢占式: 选择就绪队列中优先级最高的进程执行，直到完成。
 *   - 抢占式:   新进程到达时若优先级更高，则立即抢占当前进程。
 *   特点：灵活，但低优先级进程可能饥饿；可通过动态优先级(老化)解决。
 *======================================================================*/
#ifndef PRIORITY_SCHEDULER_H
#define PRIORITY_SCHEDULER_H

#include "Scheduler.h"

class Priority_Scheduler : public Scheduler {
private:
    bool preemptive;    // 是否为抢占式
    bool agingEnabled;  // 是否启用老化机制

public:
    Priority_Scheduler(bool isPreemptive = false, bool aging = false)
        : Scheduler(""), preemptive(isPreemptive), agingEnabled(aging) {
        updateName();
    }

    void setPreemptive(bool isPreemptive) {
        preemptive = isPreemptive;
        updateName();
    }

    void setAging(bool aging) {
        agingEnabled = aging;
        updateName();
    }

    void schedule() override {
        if (preemptive) {
            schedulePreemptive();
        } else {
            scheduleNonPreemptive();
        }
    }

private:
    void updateName() {
        algorithmName = preemptive ? "抢占式优先级调度" : "非抢占式优先级调度";
        if (agingEnabled) {
            algorithmName += "(含老化机制)";
        }
    }

    // 执行老化：增加就绪队列中等待进程的优先级
    void applyAging(ProcessQueue& readyQueue) {
        if (!agingEnabled) return;
        for (auto* p : readyQueue.getQueue()) {
            if (p->state == ProcessState::READY && p->priority > 0) {
                p->priority--; // 等待时优先级提高（数值减小）
            }
        }
    }

    // ------ 非抢占式优先级调度 ------
    void scheduleNonPreemptive() {
        if (processes.empty()) return;

        ProcessQueue readyQueue;
        currentTime = 0;
        ganttChart.clear();
        int completed = 0;
        int n = static_cast<int>(processes.size());

        std::sort(processes.begin(), processes.end(),
            [](const PCB& a, const PCB& b) {
                return a.arrivalTime < b.arrivalTime;
            });

        int nextAdmitIndex = 0;

        while (completed < n) {
            while (nextAdmitIndex < n &&
                   processes[nextAdmitIndex].arrivalTime <= currentTime) {
                processes[nextAdmitIndex].state = ProcessState::READY;
                readyQueue.enqueue(&processes[nextAdmitIndex]);
                nextAdmitIndex++;
            }

            if (readyQueue.empty()) {
                int nextArrival = processes[nextAdmitIndex].arrivalTime;
                recordIdleEvent(currentTime, nextArrival);
                currentTime = nextArrival;
                continue;
            }

            // 应用老化机制
            applyAging(readyQueue);

            // 选择优先级最高的进程
            PCB* current = readyQueue.findHighestPriority();
            readyQueue.remove(current);

            current->state = ProcessState::RUNNING;
            if (!current->hasStarted) {
                current->startTime = currentTime;
                current->hasStarted = true;
            }

            int startExec = currentTime;
            currentTime += current->burstTime;
            current->remainingTime = 0;

            recordEvent(current->name, startExec, currentTime);

            current->completionTime = currentTime;
            current->state = ProcessState::TERMINATED;
            current->calculateMetrics();
            completed++;

            while (nextAdmitIndex < n &&
                   processes[nextAdmitIndex].arrivalTime <= currentTime) {
                processes[nextAdmitIndex].state = ProcessState::READY;
                readyQueue.enqueue(&processes[nextAdmitIndex]);
                nextAdmitIndex++;
            }
        }
    }

    // ------ 抢占式优先级调度 ------
    void schedulePreemptive() {
        if (processes.empty()) return;

        ProcessQueue readyQueue;
        currentTime = 0;
        ganttChart.clear();
        int completed = 0;
        int n = static_cast<int>(processes.size());

        std::sort(processes.begin(), processes.end(),
            [](const PCB& a, const PCB& b) {
                return a.arrivalTime < b.arrivalTime;
            });

        int nextAdmitIndex = 0;
        PCB* currentProcess = nullptr;

        while (completed < n) {
            while (nextAdmitIndex < n &&
                   processes[nextAdmitIndex].arrivalTime <= currentTime) {
                processes[nextAdmitIndex].state = ProcessState::READY;
                readyQueue.enqueue(&processes[nextAdmitIndex]);
                nextAdmitIndex++;
            }

            // 当前进程放回就绪队列参与比较
            if (currentProcess != nullptr &&
                currentProcess->state == ProcessState::RUNNING) {
                currentProcess->state = ProcessState::READY;
                readyQueue.enqueue(currentProcess);
            }

            if (readyQueue.empty()) {
                if (nextAdmitIndex < n) {
                    int nextArrival = processes[nextAdmitIndex].arrivalTime;
                    recordIdleEvent(currentTime, nextArrival);
                    currentTime = nextArrival;
                    currentProcess = nullptr;
                    continue;
                }
                break;
            }

            // 应用老化并选择最高优先级进程
            applyAging(readyQueue);
            PCB* highest = readyQueue.findHighestPriority();
            readyQueue.remove(highest);

            if (!highest->hasStarted) {
                highest->startTime = currentTime;
                highest->hasStarted = true;
            }
            highest->state = ProcessState::RUNNING;

            // 确定执行到下一个事件时间
            int nextEventTime = currentTime + highest->remainingTime;
            if (nextAdmitIndex < n) {
                nextEventTime = std::min(nextEventTime,
                                         processes[nextAdmitIndex].arrivalTime);
            }

            int execDuration = nextEventTime - currentTime;
            recordEvent(highest->name, currentTime, nextEventTime);
            highest->remainingTime -= execDuration;
            currentTime = nextEventTime;

            if (highest->remainingTime == 0) {
                highest->completionTime = currentTime;
                highest->state = ProcessState::TERMINATED;
                highest->calculateMetrics();
                completed++;
                currentProcess = nullptr;
            } else {
                currentProcess = highest;
            }
        }
    }
};

#endif // PRIORITY_SCHEDULER_H
