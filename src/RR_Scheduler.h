/*======================================================================
 * 文件: RR_Scheduler.h
 * 描述: 时间片轮转(Round Robin)调度算法实现
 * 算法原理：
 *   为就绪队列中的每个进程分配一个固定长度的时间片(quantum)。
 *   进程在时间片内执行，时间片用完后：
 *   - 若进程已完成，则标记终止。
 *   - 若未完成，将其移至就绪队列末尾等待下次调度。
 *   特点：抢占式，公平，响应时间好，适用于分时系统。
 *   时间片选择：过大退化为FCFS，过小上下文切换开销大。
 *======================================================================*/
#ifndef RR_SCHEDULER_H
#define RR_SCHEDULER_H

#include "Scheduler.h"

class RR_Scheduler : public Scheduler {
private:
    int timeQuantum;  // 时间片大小

public:
    RR_Scheduler(int quantum = 2)
        : Scheduler("时间片轮转(RR, q=" + std::to_string(quantum) + ")"),
          timeQuantum(quantum) {}

    void setTimeQuantum(int quantum) {
        if (quantum <= 0) quantum = 1;
        timeQuantum = quantum;
        algorithmName = "时间片轮转(RR, q=" + std::to_string(quantum) + ")";
    }

    int getTimeQuantum() const { return timeQuantum; }

    void schedule() override {
        if (processes.empty()) return;

        ProcessQueue readyQueue;
        currentTime = 0;
        ganttChart.clear();
        int completed = 0;
        int n = static_cast<int>(processes.size());

        // 按到达时间排序
        std::sort(processes.begin(), processes.end(),
            [](const PCB& a, const PCB& b) {
                if (a.arrivalTime == b.arrivalTime) return a.pid < b.pid;
                return a.arrivalTime < b.arrivalTime;
            });

        int nextAdmitIndex = 0;

        // 将初始时刻已到达的进程加入就绪队列
        while (nextAdmitIndex < n &&
               processes[nextAdmitIndex].arrivalTime <= currentTime) {
            processes[nextAdmitIndex].state = ProcessState::READY;
            readyQueue.enqueue(&processes[nextAdmitIndex]);
            nextAdmitIndex++;
        }

        // 主调度循环
        while (completed < n) {
            if (readyQueue.empty()) {
                // CPU 空闲，跳到下一个到达时间
                if (nextAdmitIndex < n) {
                    int nextArrival = processes[nextAdmitIndex].arrivalTime;
                    recordIdleEvent(currentTime, nextArrival);
                    currentTime = nextArrival;

                    while (nextAdmitIndex < n &&
                           processes[nextAdmitIndex].arrivalTime <= currentTime) {
                        processes[nextAdmitIndex].state = ProcessState::READY;
                        readyQueue.enqueue(&processes[nextAdmitIndex]);
                        nextAdmitIndex++;
                    }
                    continue;
                }
                break;
            }

            // 取出队列头部的进程
            PCB* current = readyQueue.dequeue();
            current->state = ProcessState::RUNNING;

            if (!current->hasStarted) {
                current->startTime = currentTime;
                current->hasStarted = true;
            }

            // 计算实际执行时间（取时间片和剩余时间的较小值）
            int execTime = std::min(timeQuantum, current->remainingTime);
            int startExec = currentTime;
            currentTime += execTime;
            current->remainingTime -= execTime;

            recordEvent(current->name, startExec, currentTime);

            // 在时间片执行期间到达的新进程先加入就绪队列
            // 这保证了新到达的进程排在被轮转进程之前是错误的
            // 正确的 RR: 新进程加入后，当前进程如果未完成才排到队尾
            while (nextAdmitIndex < n &&
                   processes[nextAdmitIndex].arrivalTime <= currentTime) {
                processes[nextAdmitIndex].state = ProcessState::READY;
                readyQueue.enqueue(&processes[nextAdmitIndex]);
                nextAdmitIndex++;
            }

            if (current->remainingTime == 0) {
                // 进程已完成
                current->completionTime = currentTime;
                current->state = ProcessState::TERMINATED;
                current->calculateMetrics();
                completed++;
            } else {
                // 时间片用完但未完成，放回队尾
                current->state = ProcessState::READY;
                readyQueue.enqueue(current);
            }
        }
    }
};

#endif // RR_SCHEDULER_H
