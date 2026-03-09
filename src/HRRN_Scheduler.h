/*======================================================================
 * 文件: HRRN_Scheduler.h
 * 描述: 高响应比优先(HRRN)调度算法实现
 * 算法原理：
 *   综合考虑等待时间和服务时间，选择响应比最高的进程执行。
 *   响应比 = (等待时间 + 服务时间) / 服务时间
 *   特点：非抢占式，较好地平衡了长短作业，避免了饥饿问题。
 *         等待时间越长，响应比越大，越容易被选中。
 *         短作业因服务时间短，响应比天然较高。
 *======================================================================*/
#ifndef HRRN_SCHEDULER_H
#define HRRN_SCHEDULER_H

#include "Scheduler.h"

class HRRN_Scheduler : public Scheduler {
public:
    HRRN_Scheduler() : Scheduler("高响应比优先(HRRN)") {}

    // 计算进程在 currentTime 时刻的响应比
    static double calculateResponseRatio(const PCB* p, int currentTime) {
        int waitingTime = currentTime - p->arrivalTime;
        if (p->burstTime == 0) return 0.0;
        return static_cast<double>(waitingTime + p->burstTime) / p->burstTime;
    }

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
                return a.arrivalTime < b.arrivalTime;
            });

        int nextAdmitIndex = 0;

        while (completed < n) {
            // 加入已到达的进程
            while (nextAdmitIndex < n &&
                   processes[nextAdmitIndex].arrivalTime <= currentTime) {
                processes[nextAdmitIndex].state = ProcessState::READY;
                readyQueue.enqueue(&processes[nextAdmitIndex]);
                nextAdmitIndex++;
            }

            if (readyQueue.empty()) {
                if (nextAdmitIndex < n) {
                    int nextArrival = processes[nextAdmitIndex].arrivalTime;
                    recordIdleEvent(currentTime, nextArrival);
                    currentTime = nextArrival;
                    continue;
                }
                break;
            }

            // 选择响应比最高的进程
            PCB* selected = nullptr;
            double maxRatio = -1.0;

            for (auto* p : readyQueue.getQueue()) {
                double ratio = calculateResponseRatio(p, currentTime);
                if (ratio > maxRatio) {
                    maxRatio = ratio;
                    selected = p;
                } else if (std::abs(ratio - maxRatio) < 1e-9 &&
                           p->arrivalTime < selected->arrivalTime) {
                    selected = p;
                }
            }

            readyQueue.remove(selected);

            selected->state = ProcessState::RUNNING;
            if (!selected->hasStarted) {
                selected->startTime = currentTime;
                selected->hasStarted = true;
            }

            int startExec = currentTime;
            currentTime += selected->burstTime;
            selected->remainingTime = 0;

            recordEvent(selected->name, startExec, currentTime);

            selected->completionTime = currentTime;
            selected->state = ProcessState::TERMINATED;
            selected->calculateMetrics();
            completed++;

            // 执行期间到达的进程加入队列
            while (nextAdmitIndex < n &&
                   processes[nextAdmitIndex].arrivalTime <= currentTime) {
                processes[nextAdmitIndex].state = ProcessState::READY;
                readyQueue.enqueue(&processes[nextAdmitIndex]);
                nextAdmitIndex++;
            }
        }
    }
};

#endif // HRRN_SCHEDULER_H
