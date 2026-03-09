/*======================================================================
 * 文件: AlgorithmComparator.h
 * 描述: 算法比较与分析模块
 * 说明: 对同一组进程分别执行多种调度算法，横向比较各算法的性能指标，
 *       生成对比表格和可视化柱状图，帮助用户理解不同算法的优劣。
 *======================================================================*/
#ifndef ALGORITHM_COMPARATOR_H
#define ALGORITHM_COMPARATOR_H

#include "Scheduler.h"
#include <vector>
#include <memory>
#include <map>

// 算法性能指标结构
struct AlgorithmMetrics {
    std::string name;
    double avgTurnaroundTime;
    double avgWeightedTurnaround;
    double avgWaitingTime;
    double avgResponseTime;
    double cpuUtilization;
    double throughput;
};

class AlgorithmComparator {
private:
    std::vector<PCB> originalProcesses;             // 原始进程列表
    std::vector<AlgorithmMetrics> results;           // 各算法比较结果
    std::vector<Scheduler*> schedulers;              // 调度器指针列表

public:
    AlgorithmComparator() {}

    // 设置进程列表
    void setProcesses(const std::vector<PCB>& procs) {
        originalProcesses = procs;
    }

    // 添加调度算法
    void addScheduler(Scheduler* scheduler) {
        schedulers.push_back(scheduler);
    }

    // 清除调度器列表
    void clearSchedulers() {
        schedulers.clear();
        results.clear();
    }

    // 执行所有算法并收集结果
    void runAll() {
        results.clear();
        for (auto* scheduler : schedulers) {
            scheduler->setProcesses(originalProcesses);
            scheduler->schedule();

            AlgorithmMetrics metrics;
            metrics.name = scheduler->getName();
            metrics.avgTurnaroundTime = scheduler->getAvgTurnaroundTime();
            metrics.avgWeightedTurnaround = scheduler->getAvgWeightedTurnaround();
            metrics.avgWaitingTime = scheduler->getAvgWaitingTime();
            metrics.avgResponseTime = scheduler->getAvgResponseTime();
            metrics.cpuUtilization = scheduler->getCpuUtilization();
            metrics.throughput = scheduler->getThroughput();

            results.push_back(metrics);
        }
    }

    // 打印各算法详细结果
    void printAllResults() {
        for (auto* scheduler : schedulers) {
            scheduler->printResults();
            scheduler->printGanttChart();
            std::cout << "\n";
        }
    }

    // 打印横向比较表
    void printComparisonTable() const {
        if (results.empty()) {
            std::cout << "\n  尚未运行算法比较，请先执行 runAll()。\n";
            return;
        }

        std::cout << "\n";
        std::cout << "╔═══════════════════════════════════════════════════════"
                  << "════════════════════════════════════════════════╗\n";
        std::cout << "║                       "
                  << "多种进程调度算法性能对比表"
                  << "                                     ║\n";
        std::cout << "╠══════════════════════════════╦═══════════╦═══════════"
                  << "══╦═══════════╦═══════════╦══════════╦══════════╣\n";
        std::cout << "║ " << std::left << std::setw(29) << "算法名称"
                  << "║" << std::setw(11) << "平均周转"
                  << "║" << std::setw(13) << "平均带权周转"
                  << "║" << std::setw(11) << "平均等待"
                  << "║" << std::setw(11) << "平均响应"
                  << "║" << std::setw(10) << "CPU利用率"
                  << "║" << std::setw(10) << "吞吐量"
                  << "║\n";
        std::cout << "╠══════════════════════════════╬═══════════╬═══════════"
                  << "══╬═══════════╬═══════════╬══════════╬══════════╣\n";

        for (const auto& r : results) {
            std::cout << "║ " << std::left << std::setw(29) << r.name
                      << "║" << std::setw(11) << std::fixed
                      << std::setprecision(2) << r.avgTurnaroundTime
                      << "║" << std::setw(13) << r.avgWeightedTurnaround
                      << "║" << std::setw(11) << r.avgWaitingTime
                      << "║" << std::setw(11) << r.avgResponseTime
                      << "║" << std::setw(9) << std::setprecision(1)
                      << r.cpuUtilization << "%"
                      << "║" << std::setw(10) << std::setprecision(3)
                      << r.throughput
                      << "║\n";
        }

        std::cout << "╚══════════════════════════════╩═══════════╩═══════════"
                  << "══╩═══════════╩═══════════╩══════════╩══════════╝\n";
    }

    // 打印简易柱状图（控制台文本可视化）
    void printBarChart(const std::string& metricName) const {
        if (results.empty()) return;

        std::cout << "\n┌─── " << metricName << " 对比柱状图 ───┐\n\n";

        // 获取指标数据
        std::vector<std::pair<std::string, double>> data;
        double maxVal = 0;

        for (const auto& r : results) {
            double val = 0;
            if (metricName == "平均周转时间")   val = r.avgTurnaroundTime;
            else if (metricName == "平均等待时间")   val = r.avgWaitingTime;
            else if (metricName == "平均响应时间")   val = r.avgResponseTime;
            else if (metricName == "CPU利用率")     val = r.cpuUtilization;
            else if (metricName == "平均带权周转时间") val = r.avgWeightedTurnaround;
            else val = r.avgTurnaroundTime;

            data.emplace_back(r.name, val);
            maxVal = std::max(maxVal, val);
        }

        // 绘制横向柱状图
        int maxBarWidth = 40;
        for (const auto& d : data) {
            // 算法名（截断显示）
            std::string label = d.first;
            if (label.length() > 20) {
                label = label.substr(0, 18) + "..";
            }
            std::cout << "  " << std::left << std::setw(22) << label << " │";

            int barLen = 0;
            if (maxVal > 0) {
                barLen = static_cast<int>((d.second / maxVal) * maxBarWidth);
            }
            barLen = std::max(barLen, 1);

            // 绘制柱体
            for (int i = 0; i < barLen; i++) {
                std::cout << "█";
            }
            std::cout << " " << std::fixed << std::setprecision(2)
                      << d.second << "\n";
        }
        std::cout << "  " << std::string(22, ' ') << " +";
        for (int i = 0; i < maxBarWidth + 8; i++) std::cout << "-";
        std::cout << "\n";
    }

    // 综合评价与推荐
    void printRecommendation() const {
        if (results.empty()) return;

        std::cout << "\n╔═════════════════════════════════════════════╗\n";
        std::cout << "║           综 合 评 价 与 推 荐              ║\n";
        std::cout << "╚═════════════════════════════════════════════╝\n\n";

        // 找出各指标最优的算法
        int bestTurnaround = 0, bestWaiting = 0, bestResponse = 0;
        int bestUtilization = 0;

        for (int i = 1; i < static_cast<int>(results.size()); i++) {
            if (results[i].avgTurnaroundTime < results[bestTurnaround].avgTurnaroundTime)
                bestTurnaround = i;
            if (results[i].avgWaitingTime < results[bestWaiting].avgWaitingTime)
                bestWaiting = i;
            if (results[i].avgResponseTime < results[bestResponse].avgResponseTime)
                bestResponse = i;
            if (results[i].cpuUtilization > results[bestUtilization].cpuUtilization)
                bestUtilization = i;
        }

        std::cout << "  [最短平均周转时间] " << results[bestTurnaround].name
                  << " (" << std::fixed << std::setprecision(2)
                  << results[bestTurnaround].avgTurnaroundTime << ")\n";
        std::cout << "  [最短平均等待时间] " << results[bestWaiting].name
                  << " (" << results[bestWaiting].avgWaitingTime << ")\n";
        std::cout << "  [最短平均响应时间] " << results[bestResponse].name
                  << " (" << results[bestResponse].avgResponseTime << ")\n";
        std::cout << "  [最高CPU利用率]   " << results[bestUtilization].name
                  << " (" << std::setprecision(1)
                  << results[bestUtilization].cpuUtilization << "%)\n";

        std::cout << "\n  适用场景建议：\n";
        std::cout << "  ● 批处理系统 → 推荐 SJF/HRRN（低周转时间）\n";
        std::cout << "  ● 分时系统   → 推荐 RR/MLFQ（低响应时间）\n";
        std::cout << "  ● 实时系统   → 推荐优先级调度（高优先级任务优先）\n";
        std::cout << "  ● 通用系统   → 推荐 MLFQ（综合平衡）\n\n";
    }

    // 获取比较结果
    const std::vector<AlgorithmMetrics>& getResults() const {
        return results;
    }
};

#endif // ALGORITHM_COMPARATOR_H
