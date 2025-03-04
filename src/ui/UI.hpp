//+++ UI MAIN +++
#ifndef UI_HPP
#define UI_HPP

#include <iostream>
#include <vector>
#include <filesystem>
#include <termios.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <cmath>
#include <unordered_set>
#include "msgscr.hpp"

using namespace std;
namespace fs = std::filesystem;

namespace cuistd {

    class UI {
        private: 

        string indicator = "•"; // 指示器符号
        string folderIcon = "[D]"; // 文件夹图标
        string fileIcon = "[F]"; // 文件图标
        vector < pair < string,
        bool >> fileList; // 当前路径下的文件和文件夹列表（bool 表示是否为文件夹）
        int currentIndex = 0; // 当前选中的索引
        string currentPath = "/"; // 当前所在路径
        string errorMessage = ""; // 错误提示信息
        bool terminalMode = false; // 是否处于终端模式
        int currentPage = 0; // 当前页码
        int pageSize = 10; // 每页显示的文件数量
        unordered_set < string > lastFileSet; // 上一次的文件集，用于检测变化
        thread fileWatcherThread; // 文件监视线程
        bool stopFileWatcher = false; // 停止文件监视线程的标志

        // 截断文件名并添加省略号
        string truncateFileName(const string & name) {
            if (name.length() > 15) {
                return name.substr(0, 15) + "...";
            }
            return name;
        }

        // 检查文件列表是否有变化
        bool checkFileListChanged() {
            unordered_set < string > currentFileSet;
            for (const auto & entry: fs::directory_iterator(currentPath)) {
                currentFileSet.insert(entry.path().filename().string());
            }

            if (currentFileSet != lastFileSet) {
                lastFileSet = currentFileSet;
                return true;
            }
            return false;
        }

        // 文件监视线程函数
        void fileWatcher() {
            while (!stopFileWatcher) {
                this_thread::sleep_for(chrono::seconds(1)); // 每秒检查一次
                if (checkFileListChanged()) {
                    loadFiles(currentPath);
                    render();
                }
            }
        }

        public:
            // 加载当前路径下的文件和文件夹
            void loadFiles(const string & path) {
                try {
                    currentPath = path;
                    fileList.clear();
                    lastFileSet.clear();

                    for (const auto & entry: fs::directory_iterator(path)) {
                        bool isDir = fs::is_directory(entry.path());
                        fileList.push_back({
                            entry.path().filename().string(),
                            isDir
                        });
                        lastFileSet.insert(entry.path().filename().string());
                    }

                    // 重置当前页码为第一页
                    currentPage = 0;
                    currentIndex = 0;
                } catch (const exception & e) {
                    errorMessage = "Error: You do not have permission to access this folder.";
                    this -> showErrorForSeconds(3);

                    string parentPath = fs::path(currentPath).parent_path().string();
                    if (!parentPath.empty()) {
                        loadFiles(parentPath);
                    }
                }
            }

        // 渲染界面
        void render() {
            system("clear");

            cout << "Current Path: " << currentPath << endl;
            cout << "----------------------------------------" << endl;

            // 计算当前页的起始和结束索引
            int start = currentPage * pageSize;
            int end = min(start + pageSize, static_cast < int > (fileList.size()));

            // 显示文件列表
            for (int i = start; i < end; ++i) {
                if (i == currentIndex) {
                    cout << indicator << " ";
                } else {
                    cout << "  ";
                }

                if (fileList[i].second) // 如果是文件夹
                {
                    cout << folderIcon << " ";
                } else // 如果是文件
                {
                    cout << fileIcon << " ";
                }

                cout << truncateFileName(fileList[i].first) << endl;
            }
            cout << "----------------------------------------" << endl;

            // 计算总页数
            int totalPages = (int) ceil(fileList.size() / (double) pageSize);
            cout << "PAGE[" << currentPage + 1 << "/" << totalPages << "]" << endl;

            cout << "  ENTER - Enter folder" << endl;
            cout << "  LEFT  - Go to parent folder" << endl;
            cout << "  UP/DOWN - Navigate" << endl;
            cout << "  d     - Delete selected file/folder" << endl;
            cout << "  s after enter- Open terminal" << endl;
            cout << "  PageUp - Previous page" << endl;
            cout << "  PageDn - Next page" << endl;
            cout << "  Home - First page" << endl;
            cout << "  End - Last page\nCLI UI 1B3" << endl;

            // 显示错误信息（如果有）
            if (!errorMessage.empty()) {
                cout << "Error: " << errorMessage << endl;
            }
        }

        // 获取单个字符输入（非阻塞）
        char getch() {
            struct termios oldt, newt;
            tcgetattr(STDIN_FILENO, & oldt);
            newt = oldt;
            newt.c_lflag &= ~(ICANON | ECHO);
            tcsetattr(STDIN_FILENO, TCSANOW, & newt);

            char ch = getchar();

            tcsetattr(STDIN_FILENO, TCSANOW, & oldt);
            return ch;
        }

        // 显示错误信息并延迟清除
        void showErrorForSeconds(int seconds) {
            thread t([this, seconds]() {
                this_thread::sleep_for(chrono::seconds(seconds));
                errorMessage.clear();
            });
            t.detach();
        }

        // 处理键盘输入
        void handleInput() {
            while (true) {
                char ch = getch();
                if (terminalMode) {
                    cout << "\nTerminal Mode (Input exit to exit):" << endl;
                    chdir(currentPath.c_str());
                    system("$SHELL");
                    terminalMode = false;
                    render();
                    continue;
                }

                if (ch == '\033') {
                    getch();
                    char arrow = getch();
                    if (arrow == 'A') // Up
                    {
                        int start = currentPage * pageSize;
                        if (currentIndex > start) {
                            currentIndex--;
                        }
                    } else if (arrow == 'B') // Down
                    {
                        int start = currentPage * pageSize;
                        int end = min(start + pageSize, static_cast < int > (fileList.size()));
                        if (currentIndex < end - 1) {
                            currentIndex++;
                        }
                    } else if (arrow == 'D') // Left
                    {
                        string parentPath = fs::path(currentPath).parent_path().string();
                        if (!parentPath.empty()) {
                            loadFiles(parentPath);
                        }
                    } else if (arrow == '5') // PageUp
                    {
                        if (!fileList.empty()) // 检查文件列表是否为空
                        {
                            currentPage = max(0, currentPage - 1);
                            int start = currentPage * pageSize;
                            currentIndex = start; // 指示器指向当前页的第一个
                        }
                    } else if (arrow == '6') // PageDown
                    {
                        if (!fileList.empty()) // 检查文件列表是否为空
                        {
                            int totalPages = (int) ceil(fileList.size() / (double) pageSize);
                            currentPage = min(totalPages - 1, currentPage + 1);
                            int start = currentPage * pageSize;
                            currentIndex = start; // 指示器指向当前页的第一个
                        }
                    } else if (arrow == 'H') // Home
                    {
                        currentPage = 0;
                        currentIndex = 0;
                    } else if (arrow == 'F') // End
                    {
                        int totalPages = (int) ceil(fileList.size() / (double) pageSize);
                        currentPage = totalPages - 1;
                        currentIndex = fileList.size() - 1;
                    }
                } else if (ch == '\n' || ch == '\r') {
                    if (!fileList.empty()) // 检查文件列表是否为空
                    {
                        const auto & [name, isDir] = fileList[currentIndex];
                        string selectedPath = currentPath + "/" + name;
                        if (isDir) {
                            try {
                                loadFiles(selectedPath);
                            } catch (const exception & e) {
                                errorMessage = "Error: You do not have permission to access this folder.";
                                this -> showErrorForSeconds(3);

                                string parentPath = fs::path(currentPath).parent_path().string();
                                if (!parentPath.empty()) {
                                    loadFiles(parentPath);
                                }
                            }
                        }
                    }
                } else if (ch == 'd') {
                    if (!fileList.empty()) // 检查文件列表是否为空
                    {
                        const auto & [name, isDir] = fileList[currentIndex];
                        string selectedPath = currentPath + "/" + name;

                        cout << "Are you sure you want to delete '" << name << "'? (Y/N): ";
                        char confirm = getch();
                        if (confirm == 'Y' || confirm == 'y') {
                            try {
                                if (isDir) {
                                    fs::remove_all(selectedPath);
                                } else {
                                    fs::remove(selectedPath);
                                }
                                loadFiles(currentPath);
                            } catch (const exception & e) {
                                errorMessage = "Error: You do not have permission to delete this file/folder.";
                                this -> showErrorForSeconds(3);
                            }
                        }
                    }
                } else if (ch == 's') {
                    terminalMode = true;
                }
                else if (ch == 'q') {
                    MsgScr msgScr;
                    msgScr.display();
                }

                render();
            }
        }

        void run(const string & path) //入口
        {
            loadFiles(path);
            fileWatcherThread = thread( & UI::fileWatcher, this);
            render();
            handleInput();
            stopFileWatcher = true;
            if (fileWatcherThread.joinable()) {
                fileWatcherThread.join();
            }
        }
    };
}
#endif