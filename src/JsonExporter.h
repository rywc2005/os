/*======================================================================
 * 文件: JsonExporter.h
 * 描述: JSON 数据导出模块
 * 说明: 将调度结果序列化为 JSON 格式，供 Web 前端可视化读取。
 *       手动拼接 JSON 字符串，不引入第三方库。
 *======================================================================*/
#ifndef JSON_EXPORTER_H
#define JSON_EXPORTER_H

#include "Scheduler.h"
#include "AlgorithmComparator.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdlib>

class JsonExporter {
private:
    // 转义 JSON 字符串中的特殊字符
    static std::string escapeJson(const std::string& s) {
        std::string out;
        out.reserve(s.size() + 10);
        for (char c : s) {
            switch (c) {
                case '"':  out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\n': out += "\\n";  break;
                case '\r': out += "\\r";  break;
                case '\t': out += "\\t";  break;
                default:   out += c;
            }
        }
        return out;
    }

    // 格式化浮点数为字符串（保留2位小数）
    static std::string fmtDouble(double val, int precision = 2) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(precision) << val;
        return oss.str();
    }

public:
    /**
     * 导出全部算法比较结果为 JSON 文件
     * @param schedulers  调度器指针列表（已执行 schedule()）
     * @param processes   原始进程列表
     * @param filepath    输出 JSON 文件路径
     */
    static bool exportAll(
        const std::vector<Scheduler*>& schedulers,
        const std::vector<PCB>& originalProcesses,
        const std::string& filepath)
    {
        std::ofstream out(filepath);
        if (!out.is_open()) {
            std::cout << "  ✗ 无法创建文件: " << filepath << "\n";
            return false;
        }

        out << "{\n";

        // ======== 1. 原始进程列表 ========
        out << "  \"processes\": [\n";
        for (size_t i = 0; i < originalProcesses.size(); i++) {
            const auto& p = originalProcesses[i];
            out << "    {"
                << "\"pid\":" << p.pid << ","
                << "\"name\":\"" << escapeJson(p.name) << "\","
                << "\"arrivalTime\":" << p.arrivalTime << ","
                << "\"burstTime\":" << p.burstTime << ","
                << "\"priority\":" << p.priority
                << "}";
            if (i + 1 < originalProcesses.size()) out << ",";
            out << "\n";
        }
        out << "  ],\n";

        // ======== 2. 各算法结果 ========
        out << "  \"algorithms\": [\n";
        for (size_t a = 0; a < schedulers.size(); a++) {
            const auto* sched = schedulers[a];
            const auto& procs = sched->getProcesses();
            const auto& gantt = sched->getGanttChart();

            out << "    {\n";
            out << "      \"name\": \"" << escapeJson(sched->getName()) << "\",\n";

            // 统计指标
            out << "      \"metrics\": {\n";
            out << "        \"avgTurnaroundTime\": " << fmtDouble(sched->getAvgTurnaroundTime()) << ",\n";
            out << "        \"avgWeightedTurnaround\": " << fmtDouble(sched->getAvgWeightedTurnaround()) << ",\n";
            out << "        \"avgWaitingTime\": " << fmtDouble(sched->getAvgWaitingTime()) << ",\n";
            out << "        \"avgResponseTime\": " << fmtDouble(sched->getAvgResponseTime()) << ",\n";
            out << "        \"cpuUtilization\": " << fmtDouble(sched->getCpuUtilization(), 1) << ",\n";
            out << "        \"throughput\": " << fmtDouble(sched->getThroughput(), 3) << "\n";
            out << "      },\n";

            // 每个进程的调度结果
            out << "      \"results\": [\n";
            for (size_t i = 0; i < procs.size(); i++) {
                const auto& p = procs[i];
                out << "        {"
                    << "\"pid\":" << p.pid << ","
                    << "\"name\":\"" << escapeJson(p.name) << "\","
                    << "\"arrivalTime\":" << p.arrivalTime << ","
                    << "\"burstTime\":" << p.burstTime << ","
                    << "\"priority\":" << p.priority << ","
                    << "\"startTime\":" << p.startTime << ","
                    << "\"completionTime\":" << p.completionTime << ","
                    << "\"turnaroundTime\":" << p.turnaroundTime << ","
                    << "\"weightedTurnaround\":" << fmtDouble(p.weightedTurnaround) << ","
                    << "\"waitingTime\":" << p.waitingTime << ","
                    << "\"responseTime\":" << p.responseTime
                    << "}";
                if (i + 1 < procs.size()) out << ",";
                out << "\n";
            }
            out << "      ],\n";

            // Gantt 图事件
            out << "      \"ganttChart\": [\n";
            for (size_t i = 0; i < gantt.size(); i++) {
                const auto& e = gantt[i];
                out << "        {"
                    << "\"process\":\"" << escapeJson(e.processName) << "\","
                    << "\"start\":" << e.startTime << ","
                    << "\"end\":" << e.endTime
                    << "}";
                if (i + 1 < gantt.size()) out << ",";
                out << "\n";
            }
            out << "      ]\n";

            out << "    }";
            if (a + 1 < schedulers.size()) out << ",";
            out << "\n";
        }
        out << "  ]\n";

        out << "}\n";
        out.close();
        return true;
    }

    /**
     * 导出并自动在浏览器中打开 Web 页面
     */
    static void exportAndOpen(
        const std::vector<Scheduler*>& schedulers,
        const std::vector<PCB>& originalProcesses,
        const std::string& jsonPath,
        const std::string& htmlPath)
    {
        if (exportAll(schedulers, originalProcesses, jsonPath)) {
            std::cout << "  ✓ JSON 数据已导出到: " << jsonPath << "\n";
            std::cout << "  正在打开 Web 可视化页面...\n";
#ifdef _WIN32
            std::string cmd = "start \"\" \"" + htmlPath + "\"";
#else
            std::string cmd = "xdg-open \"" + htmlPath + "\"";
#endif
            system(cmd.c_str());
        }
    }
};

#endif // JSON_EXPORTER_H
