/*======================================================================
 * 文件: MLFQ_Scheduler.h
 * 描述: 多级反馈队列(Multilevel Feedback Queue)调度算法实现
 * 算法原理：
 *   设置多个就绪队列（如3级），每个队列有不同的优先级和时间片大小。
 *   - 第 0 级队列优先级最高，时间片最小。
 *   - 新进程首先进入第 0 级队列。
 *   - 若在当前级别的时间片内未完成，则降级到下一级队列。
 *   - 只有当高优先级队列为空时，才调度低优先级队列中的进程。
 *   - 当有新进程到达高优先级队列时，可以抢占低优先级队列中正在执行的进程。
 *   特点：兼顾响应时间和吞吐量，终端型作业短，能在高优先级快速完成；
 *         长批处理作业逐级下降但不会饥饿（可设置升级机制）。
 *======================================================================*/
#ifndef MLFQ_SCHEDULER_H
#define MLFQ_SCHEDULER_H

#include "Scheduler.h"
#include <queue>

class MLFQ_Scheduler : public Scheduler {
private:
    int numQueues;                    // 队列级数
    std::vector<int> timeQuantums;    // 每级队列的时间片大小
    bool boostEnabled;                // 是否启用优先级提升（防饥饿）
    int boostInterval;                // 优先级提升间隔

public:
    MLFQ_Scheduler(int queues = 3)
        : Scheduler("多级反馈队列(MLFQ)"), numQueues(queues),
          boostEnabled(true), boostInterval(20) {
        // 默认时间片：第0级=1, 第1级=2, 第2级=4, ...
        timeQuantums.resize(numQueues);
        for (int i = 0; i < numQueues; i++) {
            timeQuantums[i] = (1 << i); // 2^i
        }
        updateName();
    }

    // 设置队列数量和时间片
    void configure(int queues, const std::vector<int>& quantums) {
        numQueues = queues;
        timeQuantums = quantums;
        if (static_cast<int>(timeQuantums.size()) < numQueues) {
            int lastQ = timeQuantums.empty() ? 1 : timeQuantums.back();
            while (static_cast<int>(timeQuantums.size()) < numQueues) {
                lastQ *= 2;
                timeQuantums.push_back(lastQ);
            }
        }
        updateName();
    }

    void setBoost(bool enabled, int interval = 20) {
        boostEnabled = enabled;
        boostInterval = interval;
    }

    void schedule() override {
        if (processes.empty()) return;

        // 创建多级就绪队列
        std::vector<ProcessQueue> queues(numQueues);
        currentTime = 0;
        ganttChart.clear();
        int completed = 0;
        int n = static_cast<int>(processes.size());
        int lastBoostTime = 0;

        // 按到达时间排序
        std::sort(processes.begin(), processes.end(),
            [](const PCB& a, const PCB& b) {
                return a.arrivalTime < b.arrivalTime;
            });

        int nextAdmitIndex = 0;

        while (completed < n) {
            // 将已到达进程加入第0级队列
            while (nextAdmitIndex < n &&
                   processes[nextAdmitIndex].arrivalTime <= currentTime) {
                processes[nextAdmitIndex].state = ProcessState::READY;
                processes[nextAdmitIndex].currentQueue = 0;
                queues[0].enqueue(&processes[nextAdmitIndex]);
                nextAdmitIndex++;
            }

            // 优先级提升（boost）—— 防止低级队列饥饿
            if (boostEnabled &&
                (currentTime - lastBoostTime) >= boostInterval) {
                boostAllProcesses(queues);
                lastBoostTime = currentTime;
            }

            // 从最高优先级（0级）开始查找非空队列
            int activeQueue = -1;
            for (int i = 0; i < numQueues; i++) {
                if (!queues[i].empty()) {
                    activeQueue = i;
                    break;
                }
            }

            if (activeQueue == -1) {
                // 所有队列为空，CPU 空闲
                if (nextAdmitIndex < n) {
                    int nextArrival = processes[nextAdmitIndex].arrivalTime;
                    recordIdleEvent(currentTime, nextArrival);
                    currentTime = nextArrival;
                    continue;
                }
                break;
            }

            // 从活跃队列取出进程执行
            PCB* current = queues[activeQueue].dequeue();
            current->state = ProcessState::RUNNING;

            if (!current->hasStarted) {
                current->startTime = currentTime;
                current->hasStarted = true;
            }

            // 当前队列的时间片
            int quantum = timeQuantums[activeQueue];
            int execTime = std::min(quantum, current->remainingTime);

            // 检查是否有高优先级进程在执行期间到达（抢占检查）
            int preemptTime = currentTime + execTime;
            if (nextAdmitIndex < n) {
                int nextArr = processes[nextAdmitIndex].arrivalTime;
                // 如果新进程到达时间在时间片内，且会进入更高优先级队列
                if (nextArr < preemptTime && nextArr > currentTime) {
                    // 将执行截断到新进程到达时间点
                    execTime = nextArr - currentTime;
                }
            }

            int startExec = currentTime;
            currentTime += execTime;
            current->remainingTime -= execTime;
            recordEvent(current->name, startExec, currentTime);

            // 在执行期间到达的进程加入第0级队列
            while (nextAdmitIndex < n &&
                   processes[nextAdmitIndex].arrivalTime <= currentTime) {
                processes[nextAdmitIndex].state = ProcessState::READY;
                processes[nextAdmitIndex].currentQueue = 0;
                queues[0].enqueue(&processes[nextAdmitIndex]);
                nextAdmitIndex++;
            }

            if (current->remainingTime == 0) {
                // 进程完成
                current->completionTime = currentTime;
                current->state = ProcessState::TERMINATED;
                current->calculateMetrics();
                completed++;
            } else if (execTime < quantum) {
                // 被抢占（未用完时间片）：保持在当前队列
                current->state = ProcessState::READY;
                queues[activeQueue].enqueue(current);
            } else {
                // 时间片用完：降级到下一级队列
                int nextQueue = std::min(activeQueue + 1, numQueues - 1);
                current->currentQueue = nextQueue;
                current->state = ProcessState::READY;
                queues[nextQueue].enqueue(current);
            }
        }
    }

private:
    void updateName() {
        algorithmName = "多级反馈队列(MLFQ, " +
                        std::to_string(numQueues) + "级, q=";
        for (int i = 0; i < numQueues; i++) {
            if (i > 0) algorithmName += "/";
            algorithmName += std::to_string(timeQuantums[i]);
        }
        algorithmName += ")";
    }

    // 将所有进程提升到最高级队列（防饥饿）
    void boostAllProcesses(std::vector<ProcessQueue>& queues) {
        for (int i = 1; i < numQueues; i++) {
            while (!queues[i].empty()) {
                PCB* p = queues[i].dequeue();
                p->currentQueue = 0;
                queues[0].enqueue(p);
            }
        }
    }
};

#endif // MLFQ_SCHEDULER_H
