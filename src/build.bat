@echo off
chcp 65001 >nul 2>&1
echo ======================================
echo   进程调度算法模拟系统 - 编译脚本
echo ======================================

set COMPILER=D:\Develop\Dev-Cpp\MinGW64\bin\g++.exe

if not exist "%COMPILER%" (
    echo [错误] 未找到编译器: %COMPILER%
    echo 请修改此脚本中的 COMPILER 路径
    pause
    exit /b 1
)

echo 正在编译...
"%COMPILER%" -std=c++11 -Wall -o scheduler.exe main.cpp

if %ERRORLEVEL%==0 (
    echo [成功] 编译完成！生成 scheduler.exe
    echo 运行命令: scheduler.exe
) else (
    echo [失败] 编译失败，请检查代码错误
)
pause
