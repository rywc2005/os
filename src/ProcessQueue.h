/*======================================================================
 * 文件: ProcessQueue.h
 * 描述: 进程就绪队列管理类
 * 说明: 封装进程队列的增删查改操作，支持按不同策略排序，
 *       为各调度算法提供统一的队列操作接口。
 *======================================================================*/
#ifndef PROCESS_QUEUE_H
#define PROCESS_QUEUE_H

#include "PCB.h"
#include <vector>
#include <algorithm>
#include <functional>

class ProcessQueue {
private:
    std::vector<PCB*> queue;   // 使用指针避免拷贝开销

public:
    ProcessQueue() {}

    // 向队列中加入进程
    void enqueue(PCB* process) {
        queue.push_back(process);
    }

    // 从队列头部取出进程
    PCB* dequeue() {
        if (queue.empty()) return nullptr;
        PCB* front = queue.front();
        queue.erase(queue.begin());
        return front;
    }

    // 查看队列头部进程（不取出）
    PCB* peek() const {
        if (queue.empty()) return nullptr;
        return queue.front();
    }

    // 移除指定进程
    bool remove(PCB* process) {
        auto it = std::find(queue.begin(), queue.end(), process);
        if (it != queue.end()) {
            queue.erase(it);
            return true;
        }
        return false;
    }

    // 判断队列是否为空
    bool empty() const {
        return queue.empty();
    }

    // 获取队列大小
    size_t size() const {
        return queue.size();
    }

    // 清空队列
    void clear() {
        queue.clear();
    }

    // 获取内部 vector 引用（用于遍历）
    std::vector<PCB*>& getQueue() {
        return queue;
    }

    const std::vector<PCB*>& getQueue() const {
        return queue;
    }

    // 按到达时间排序
    void sortByArrivalTime() {
        std::sort(queue.begin(), queue.end(),
            [](const PCB* a, const PCB* b) {
                if (a->arrivalTime == b->arrivalTime)
                    return a->pid < b->pid;
                return a->arrivalTime < b->arrivalTime;
            });
    }

    // 按服务时间排序（短作业优先）
    void sortByBurstTime() {
        std::sort(queue.begin(), queue.end(),
            [](const PCB* a, const PCB* b) {
                if (a->burstTime == b->burstTime)
                    return a->arrivalTime < b->arrivalTime;
                return a->burstTime < b->burstTime;
            });
    }

    // 按剩余时间排序（SRTF）
    void sortByRemainingTime() {
        std::sort(queue.begin(), queue.end(),
            [](const PCB* a, const PCB* b) {
                if (a->remainingTime == b->remainingTime)
                    return a->arrivalTime < b->arrivalTime;
                return a->remainingTime < b->remainingTime;
            });
    }

    // 按优先级排序（数值小优先级高）
    void sortByPriority() {
        std::sort(queue.begin(), queue.end(),
            [](const PCB* a, const PCB* b) {
                if (a->priority == b->priority)
                    return a->arrivalTime < b->arrivalTime;
                return a->priority < b->priority;
            });
    }

    // 通用自定义排序
    void sortBy(std::function<bool(const PCB*, const PCB*)> comparator) {
        std::sort(queue.begin(), queue.end(), comparator);
    }

    // 查找最短剩余时间的进程
    PCB* findShortestRemaining() {
        if (queue.empty()) return nullptr;
        PCB* shortest = queue[0];
        for (auto* p : queue) {
            if (p->remainingTime < shortest->remainingTime) {
                shortest = p;
            } else if (p->remainingTime == shortest->remainingTime
                       && p->arrivalTime < shortest->arrivalTime) {
                shortest = p;
            }
        }
        return shortest;
    }

    // 查找最高优先级的进程
    PCB* findHighestPriority() {
        if (queue.empty()) return nullptr;
        PCB* highest = queue[0];
        for (auto* p : queue) {
            if (p->priority < highest->priority) {
                highest = p;
            } else if (p->priority == highest->priority
                       && p->arrivalTime < highest->arrivalTime) {
                highest = p;
            }
        }
        return highest;
    }

    // 打印队列中所有进程信息
    void printQueue(const std::string& title) const {
        std::cout << "\n--- " << title << " (共 " << queue.size() << " 个进程) ---\n";
        if (queue.empty()) {
            std::cout << "  (空)\n";
            return;
        }
        std::cout << std::left
                  << std::setw(6)  << "PID"
                  << std::setw(10) << "名称"
                  << std::setw(10) << "到达时间"
                  << std::setw(10) << "服务时间"
                  << std::setw(10) << "剩余时间"
                  << std::setw(8)  << "优先级"
                  << std::setw(8)  << "状态"
                  << "\n";
        std::cout << std::string(62, '-') << "\n";
        for (const auto* p : queue) {
            std::cout << std::left
                      << std::setw(6)  << p->pid
                      << std::setw(10) << p->name
                      << std::setw(10) << p->arrivalTime
                      << std::setw(10) << p->burstTime
                      << std::setw(10) << p->remainingTime
                      << std::setw(8)  << p->priority
                      << std::setw(8)  << p->getStateStr()
                      << "\n";
        }
    }
};

#endif // PROCESS_QUEUE_H
