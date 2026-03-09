/*======================================================================
 * 文件: SJF_Scheduler.h
 * 描述: 短作业优先(SJF)调度算法实现，包含非抢占式和抢占式(SRTF)两种模式
 * 算法原理：
 *   非抢占SJF: 选择就绪队列中服务时间最短的进程先执行，一旦执行不被中断。
 *   抢占SRTF:  每当有新进程到达时，比较剩余时间，选择最短的执行。
 *   特点：平均等待时间最短（最优），但可能导致长作业"饥饿"。
 *======================================================================*/
#ifndef SJF_SCHEDULER_H
#define SJF_SCHEDULER_H

#include "Scheduler.h"

class SJF_Scheduler : public Scheduler {
private:
    bool preemptive; // 是否为抢占式

public:
    SJF_Scheduler(bool isPreemptive = false)
        : Scheduler(isPreemptive ? "最短剩余时间优先(SRTF)" : "短作业优先(SJF)"),
          preemptive(isPreemptive) {}

    void setPreemptive(bool isPreemptive) {
        preemptive = isPreemptive;
        algorithmName = isPreemptive ? "最短剩余时间优先(SRTF)" : "短作业优先(SJF)";
    }

    void schedule() override {
        if (preemptive) {
            schedulePreemptive();
        } else {
            scheduleNonPreemptive();
        }
    }

private:
    // ------ 非抢占式 SJF ------
    void scheduleNonPreemptive() {
        if (processes.empty()) return;

        ProcessQueue readyQueue;
        currentTime = 0;
        ganttChart.clear();
        int completed = 0;
        int n = static_cast<int>(processes.size());

        // 按到达时间排序
        std::sort(processes.begin(), processes.end(),
            [](const PCB& a, const PCB& b) {
                return a.arrivalTime < b.arrivalTime;
            });

        int nextAdmitIndex = 0;

        while (completed < n) {
            // 加入已到达进程
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

            // 从就绪队列中选择服务时间最短的进程
            readyQueue.sortByBurstTime();
            PCB* current = readyQueue.dequeue();

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

            // 执行期间新到达的进程加入队列
            while (nextAdmitIndex < n &&
                   processes[nextAdmitIndex].arrivalTime <= currentTime) {
                processes[nextAdmitIndex].state = ProcessState::READY;
                readyQueue.enqueue(&processes[nextAdmitIndex]);
                nextAdmitIndex++;
            }
        }
    }

    // ------ 抢占式 SRTF ------
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
            // 加入已到达进程
            while (nextAdmitIndex < n &&
                   processes[nextAdmitIndex].arrivalTime <= currentTime) {
                processes[nextAdmitIndex].state = ProcessState::READY;
                readyQueue.enqueue(&processes[nextAdmitIndex]);
                nextAdmitIndex++;
            }

            // 如果当前有进程在运行，也放回就绪队列比较
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

            // 选择剩余时间最短的进程
            PCB* shortest = readyQueue.findShortestRemaining();
            readyQueue.remove(shortest);

            // 记录首次运行
            if (!shortest->hasStarted) {
                shortest->startTime = currentTime;
                shortest->hasStarted = true;
            }
            shortest->state = ProcessState::RUNNING;

            // 找出下一个事件时间点（新进程到达或当前进程完成）
            int nextEventTime = currentTime + shortest->remainingTime;
            if (nextAdmitIndex < n) {
                nextEventTime = std::min(nextEventTime,
                                         processes[nextAdmitIndex].arrivalTime);
            }

            int execDuration = nextEventTime - currentTime;
            recordEvent(shortest->name, currentTime, nextEventTime);
            shortest->remainingTime -= execDuration;
            currentTime = nextEventTime;

            if (shortest->remainingTime == 0) {
                shortest->completionTime = currentTime;
                shortest->state = ProcessState::TERMINATED;
                shortest->calculateMetrics();
                completed++;
                currentProcess = nullptr;
            } else {
                currentProcess = shortest;
            }
        }
    }
};

#endif // SJF_SCHEDULER_H
