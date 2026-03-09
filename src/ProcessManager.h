/*======================================================================
 * 文件: ProcessManager.h
 * 描述: 进程管理模块 —— 负责进程的创建、编辑、删除和持久化
 * 说明: 提供手动输入、随机生成、预设测试用例、文件导入导出等
 *       多种方式管理进程数据，为调度算法提供数据源。
 *======================================================================*/
#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include "PCB.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <random>
#include <ctime>
#include <iomanip>
#include <algorithm>
#include <limits>

class ProcessManager {
private:
    std::vector<PCB> processes;
    int nextPid;

    // 安全的字符串转整数，失败时返回默认值
    static int safeStoi(const std::string& s, int defaultVal) {
        if (s.empty()) return defaultVal;
        try { return std::stoi(s); }
        catch (...) { return defaultVal; }
    }

public:
    ProcessManager() : nextPid(1) {}

    // 获取进程列表
    std::vector<PCB>& getProcesses() { return processes; }
    const std::vector<PCB>& getProcesses() const { return processes; }

    // 清空进程列表
    void clearProcesses() {
        processes.clear();
        nextPid = 1;
    }

    // 手动添加单个进程
    void addProcessManually() {
        PCB p;
        p.pid = nextPid++;

        std::cout << "\n  === 添加新进程 (PID=" << p.pid << ") ===\n";

        std::cout << "  进程名称: ";
        std::cin >> p.name;

        std::cout << "  到达时间 (>=0): ";
        while (!(std::cin >> p.arrivalTime) || p.arrivalTime < 0) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "  输入无效，请输入非负整数: ";
        }

        std::cout << "  服务时间 (>0): ";
        while (!(std::cin >> p.burstTime) || p.burstTime <= 0) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "  输入无效，请输入正整数: ";
        }

        std::cout << "  优先级 (数值越小优先级越高, >=0): ";
        while (!(std::cin >> p.priority) || p.priority < 0) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "  输入无效，请输入非负整数: ";
        }

        p.remainingTime = p.burstTime;
        p.state = ProcessState::NEW;
        processes.push_back(p);

        std::cout << "  ✓ 进程 " << p.name << " 已添加成功！\n";
    }

    // 批量手动输入多个进程
    void addMultipleProcesses() {
        int count;
        std::cout << "\n  请输入要添加的进程数量: ";
        while (!(std::cin >> count) || count <= 0 || count > 100) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "  输入无效，请输入 1~100 的正整数: ";
        }

        std::cout << "\n  请依次输入每个进程的信息：\n";
        std::cout << "  格式: 名称 到达时间 服务时间 优先级\n";
        std::cout << "  示例: P1 0 5 3\n\n";

        for (int i = 0; i < count; i++) {
            PCB p;
            p.pid = nextPid++;
            std::cout << "  进程 " << i + 1 << " (PID=" << p.pid << "): ";

            if (!(std::cin >> p.name >> p.arrivalTime >> p.burstTime >> p.priority)) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "  输入格式错误，请重新输入此进程。\n";
                nextPid--;
                i--;
                continue;
            }

            if (p.arrivalTime < 0) p.arrivalTime = 0;
            if (p.burstTime <= 0) p.burstTime = 1;
            if (p.priority < 0) p.priority = 0;

            p.remainingTime = p.burstTime;
            p.state = ProcessState::NEW;
            processes.push_back(p);
        }

        std::cout << "\n  ✓ 成功添加 " << count << " 个进程！\n";
    }

    // 随机生成进程
    void generateRandomProcesses() {
        int count;
        std::cout << "\n  请输入要生成的进程数量 (1~50): ";
        while (!(std::cin >> count) || count <= 0 || count > 50) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "  输入无效，请输入 1~50 的正整数: ";
        }

        int maxArrival, maxBurst, maxPriority;
        std::cout << "  最大到达时间 (默认20): ";
        std::string input;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::getline(std::cin, input);
        maxArrival = safeStoi(input, 20);

        std::cout << "  最大服务时间 (默认15): ";
        std::getline(std::cin, input);
        maxBurst = safeStoi(input, 15);

        std::cout << "  最大优先级数值 (默认10): ";
        std::getline(std::cin, input);
        maxPriority = safeStoi(input, 10);

        // 使用随机数引擎
        std::mt19937 gen(static_cast<unsigned int>(std::time(nullptr)));
        std::uniform_int_distribution<> arrivalDist(0, maxArrival);
        std::uniform_int_distribution<> burstDist(1, maxBurst);
        std::uniform_int_distribution<> prioDist(0, maxPriority);

        for (int i = 0; i < count; i++) {
            PCB p;
            p.pid = nextPid++;
            p.name = "P" + std::to_string(p.pid);
            p.arrivalTime = arrivalDist(gen);
            p.burstTime = burstDist(gen);
            p.priority = prioDist(gen);
            p.remainingTime = p.burstTime;
            p.state = ProcessState::NEW;
            processes.push_back(p);
        }

        std::cout << "\n  ✓ 成功随机生成 " << count << " 个进程！\n";
    }

    // 预设测试用例
    void loadPresetProcesses(int presetId) {
        clearProcesses();

        switch (presetId) {
            case 1: {
                // 经典教材示例：5个进程
                std::cout << "\n  加载预设用例1: 经典5进程示例\n";
                processes = {
                    PCB(1, "P1", 0, 8, 3),
                    PCB(2, "P2", 1, 4, 1),
                    PCB(3, "P3", 2, 9, 4),
                    PCB(4, "P4", 3, 5, 2),
                    PCB(5, "P5", 4, 2, 5)
                };
                nextPid = 6;
                break;
            }
            case 2: {
                // 同时到达的进程
                std::cout << "\n  加载预设用例2: 同时到达进程\n";
                processes = {
                    PCB(1, "P1", 0, 10, 3),
                    PCB(2, "P2", 0, 5, 1),
                    PCB(3, "P3", 0, 8, 4),
                    PCB(4, "P4", 0, 3, 2),
                    PCB(5, "P5", 0, 6, 5)
                };
                nextPid = 6;
                break;
            }
            case 3: {
                // 大规模进程集
                std::cout << "\n  加载预设用例3: 较大规模进程集(10个进程)\n";
                processes = {
                    PCB(1,  "A",  0,  6, 3),
                    PCB(2,  "B",  1,  3, 5),
                    PCB(3,  "C",  2,  8, 1),
                    PCB(4,  "D",  3,  2, 4),
                    PCB(5,  "E",  5,  4, 2),
                    PCB(6,  "F",  6,  7, 6),
                    PCB(7,  "G",  8,  5, 3),
                    PCB(8,  "H", 10,  1, 7),
                    PCB(9,  "I", 12,  9, 2),
                    PCB(10, "J", 15,  3, 4)
                };
                nextPid = 11;
                break;
            }
            case 4: {
                // 长短作业混合
                std::cout << "\n  加载预设用例4: 长短作业混合\n";
                processes = {
                    PCB(1, "Short1", 0,  1, 3),
                    PCB(2, "Long1",  0, 20, 2),
                    PCB(3, "Short2", 2,  2, 4),
                    PCB(4, "Mid1",   3,  8, 1),
                    PCB(5, "Short3", 5,  1, 5),
                    PCB(6, "Long2",  6, 15, 3),
                    PCB(7, "Short4", 8,  3, 2)
                };
                nextPid = 8;
                break;
            }
            case 5: {
                // 带间隔到达的进程
                std::cout << "\n  加载预设用例5: 间隔到达进程\n";
                processes = {
                    PCB(1, "P1",  0, 4, 2),
                    PCB(2, "P2",  5, 3, 1),
                    PCB(3, "P3", 10, 5, 3),
                    PCB(4, "P4", 15, 2, 4),
                    PCB(5, "P5", 20, 6, 2)
                };
                nextPid = 6;
                break;
            }
            default:
                std::cout << "  无效的预设用例编号！\n";
                return;
        }

        std::cout << "  ✓ 成功加载 " << processes.size() << " 个进程。\n";
    }

    // 从文件导入进程
    bool loadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cout << "  ✗ 无法打开文件: " << filename << "\n";
            return false;
        }

        clearProcesses();
        std::string line;

        // 跳过标题行
        if (std::getline(file, line)) {
            // 检查是否为标题行
            if (line.find("名称") == std::string::npos &&
                line.find("name") == std::string::npos &&
                line.find("Name") == std::string::npos) {
                // 不是标题行，解析为数据
                std::istringstream iss(line);
                PCB p;
                p.pid = nextPid++;
                if (iss >> p.name >> p.arrivalTime >> p.burstTime >> p.priority) {
                    p.remainingTime = p.burstTime;
                    processes.push_back(p);
                }
            }
        }

        // 读取数据行
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            std::istringstream iss(line);
            PCB p;
            p.pid = nextPid++;
            if (iss >> p.name >> p.arrivalTime >> p.burstTime >> p.priority) {
                if (p.arrivalTime < 0) p.arrivalTime = 0;
                if (p.burstTime <= 0) p.burstTime = 1;
                if (p.priority < 0) p.priority = 0;
                p.remainingTime = p.burstTime;
                processes.push_back(p);
            }
        }

        file.close();
        std::cout << "  ✓ 从文件成功导入 " << processes.size() << " 个进程。\n";
        return !processes.empty();
    }

    // 导出进程到文件
    bool saveToFile(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cout << "  ✗ 无法创建文件: " << filename << "\n";
            return false;
        }

        file << "名称\t到达时间\t服务时间\t优先级\n";
        for (const auto& p : processes) {
            file << p.name << "\t" << p.arrivalTime << "\t"
                 << p.burstTime << "\t" << p.priority << "\n";
        }

        file.close();
        std::cout << "  ✓ 进程数据已保存到: " << filename << "\n";
        return true;
    }

    // 显示当前进程列表
    void displayProcesses() const {
        if (processes.empty()) {
            std::cout << "\n  当前没有进程，请先添加或加载进程。\n";
            return;
        }

        std::cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        std::cout << "  ║              当前进程列表 (共 "
                  << std::setw(2) << processes.size()
                  << " 个)                   ║\n";
        std::cout << "  ╠══════╦══════════╦══════════╦══════════╦══════════════╣\n";
        std::cout << "  ║ " << std::left
                  << std::setw(5) << "PID"
                  << "║ " << std::setw(9) << "名称"
                  << "║ " << std::setw(9) << "到达时间"
                  << "║ " << std::setw(9) << "服务时间"
                  << "║ " << std::setw(13) << "优先级"
                  << "║\n";
        std::cout << "  ╠══════╬══════════╬══════════╬══════════╬══════════════╣\n";

        for (const auto& p : processes) {
            std::cout << "  ║ " << std::left
                      << std::setw(5)  << p.pid
                      << "║ " << std::setw(9) << p.name
                      << "║ " << std::setw(9) << p.arrivalTime
                      << "║ " << std::setw(9) << p.burstTime
                      << "║ " << std::setw(13) << p.priority
                      << "║\n";
        }

        std::cout << "  ╚══════╩══════════╩══════════╩══════════╩══════════════╝\n";
    }

    // 删除指定进程
    bool removeProcess(int pid) {
        auto it = std::find_if(processes.begin(), processes.end(),
            [pid](const PCB& p) { return p.pid == pid; });
        if (it != processes.end()) {
            std::cout << "  ✓ 进程 " << it->name << " (PID="
                      << pid << ") 已删除。\n";
            processes.erase(it);
            return true;
        }
        std::cout << "  ✗ 未找到 PID=" << pid << " 的进程。\n";
        return false;
    }

    // 修改指定进程
    void editProcess(int pid) {
        auto it = std::find_if(processes.begin(), processes.end(),
            [pid](const PCB& p) { return p.pid == pid; });
        if (it == processes.end()) {
            std::cout << "  ✗ 未找到 PID=" << pid << " 的进程。\n";
            return;
        }

        PCB& p = *it;
        std::cout << "\n  修改进程 " << p.name << " (PID=" << p.pid << ")\n";
        std::cout << "  当前值: 名称=" << p.name << ", 到达=" << p.arrivalTime
                  << ", 服务=" << p.burstTime << ", 优先级=" << p.priority << "\n";
        std::cout << "  输入新值 (名称 到达时间 服务时间 优先级): ";
        std::cin >> p.name >> p.arrivalTime >> p.burstTime >> p.priority;

        if (p.arrivalTime < 0) p.arrivalTime = 0;
        if (p.burstTime <= 0) p.burstTime = 1;
        if (p.priority < 0) p.priority = 0;
        p.remainingTime = p.burstTime;

        std::cout << "  ✓ 进程信息已更新。\n";
    }

    // 检查是否有进程
    bool hasProcesses() const {
        return !processes.empty();
    }
};

#endif // PROCESS_MANAGER_H
