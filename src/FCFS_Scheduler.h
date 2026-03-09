/*======================================================================
 * 文件: FCFS_Scheduler.h
 * 描述: 先来先服务(FCFS)调度算法实现
 * 算法原理：
 *   按照进程到达就绪队列的先后顺序来分配 CPU。
 *   特点：非抢占式，实现简单，对长作业有利，对短作业不利。
 *   适用场景：批处理系统。
 *======================================================================*/
#ifndef FCFS_SCHEDULER_H
#define FCFS_SCHEDULER_H

#include "Scheduler.h"

class FCFS_Scheduler : public Scheduler {
public:
    FCFS_Scheduler() : Scheduler("先来先服务(FCFS)") {}

    void schedule() override {
        if (processes.empty()) return;

        ProcessQueue readyQueue;
        currentTime = 0;
        ganttChart.clear();

        // 按到达时间排序进程
        std::sort(processes.begin(), processes.end(),
            [](const PCB& a, const PCB& b) {
                if (a.arrivalTime == b.arrivalTime) return a.pid < b.pid;
                return a.arrivalTime < b.arrivalTime;
            });

        int completed = 0;
        int n = static_cast<int>(processes.size());
        int nextAdmitIndex = 0;

        // 主调度循环
        while (completed < n) {
            // 将当前时间之前到达的进程加入就绪队列
            while (nextAdmitIndex < n &&
                   processes[nextAdmitIndex].arrivalTime <= currentTime) {
                processes[nextAdmitIndex].state = ProcessState::READY;
                readyQueue.enqueue(&processes[nextAdmitIndex]);
                nextAdmitIndex++;
            }

            if (readyQueue.empty()) {
                // CPU 空闲：跳到下一个进程到达的时间
                int nextArrival = processes[nextAdmitIndex].arrivalTime;
                recordIdleEvent(currentTime, nextArrival);
                currentTime = nextArrival;
                continue;
            }

            // FCFS: 取出队列头部的进程执行
            PCB* current = readyQueue.dequeue();
            current->state = ProcessState::RUNNING;

            // 记录首次运行时间
            if (!current->hasStarted) {
                current->startTime = currentTime;
                current->hasStarted = true;
            }

            // 非抢占：一直执行到完成
            int startExec = currentTime;
            currentTime += current->burstTime;
            current->remainingTime = 0;

            // 记录 Gantt 图事件
            recordEvent(current->name, startExec, currentTime);

            // 进程完成
            current->completionTime = currentTime;
            current->state = ProcessState::TERMINATED;
            current->calculateMetrics();
            completed++;

            // 在进程执行期间到达的进程加入就绪队列
            while (nextAdmitIndex < n &&
                   processes[nextAdmitIndex].arrivalTime <= currentTime) {
                processes[nextAdmitIndex].state = ProcessState::READY;
                readyQueue.enqueue(&processes[nextAdmitIndex]);
                nextAdmitIndex++;
            }
        }
    }
};

#endif // FCFS_SCHEDULER_H
