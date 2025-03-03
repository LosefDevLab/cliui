#include <iostream>
#include <vector>
#include <filesystem>
#include <termios.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <cstring>

using namespace std;
namespace fs = std::filesystem;

class UI
{
private:
    string indicator = "•";          // 指示器符号
    string folderIcon = "[D]";       // 文件夹图标
    string fileIcon = "[F]";         // 文件图标
    vector<pair<string, bool>> fileList; // 当前路径下的文件和文件夹列表（bool 表示是否为文件夹）
    int currentIndex = 0;            // 当前选中的索引
    string currentPath = "/";        // 当前所在路径
    string errorMessage = "";        // 错误提示信息
    bool terminalMode = false;       // 是否处于终端模式

public:
    // 加载当前路径下的文件和文件夹
    void loadFiles(const string& path)
    {
        try
        {
            currentPath = path;
            fileList.clear();
            for (const auto& entry : fs::directory_iterator(path))
            {
                bool isDir = fs::is_directory(entry.path());
                fileList.push_back({entry.path().filename().string(), isDir});
            }
        }
        catch (const exception& e)
        {
            errorMessage = "Error: You do not have permission to access this folder.";
            this->showErrorForSeconds(3);

            
            string parentPath = fs::path(currentPath).parent_path().string();
            if (!parentPath.empty())
            {
                loadFiles(parentPath);
            }
        }
    }

    // 渲染界面
    void render()
    {
        system("clear");


        cout << "Current Path: " << currentPath << endl;
        cout << "----------------------------------------" << endl;

        // 显示文件列表
        for (size_t i = 0; i < fileList.size(); ++i)
        {
            if (i == currentIndex)
            {
                cout << indicator << " ";
            }
            else
            {
                cout << "  ";
            }

            if (fileList[i].second) // 如果是文件夹
            {
                cout << folderIcon << " ";
            }
            else // 如果是文件
            {
                cout << fileIcon << " ";
            }

            cout << fileList[i].first << endl;
        }
        cout << "----------------------------------------" << endl;
        cout << "  ENTER - Enter folder" << endl;
        cout << "  LEFT  - Go to parent folder" << endl;
        cout << "  UP/DOWN - Navigate" << endl;
        cout << "  d     - Delete selected file/folder" << endl;
        cout << "  s after enter- Open terminal" << endl;

        // 显示错误信息（如果有）
        if (!errorMessage.empty())
        {
            cout << "Error: " << errorMessage << endl;
        }
    }

    // 获取单个字符输入（非阻塞）
    char getch()
    {
        struct termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);

        char ch = getchar();

        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        return ch;
    }

    // 显示错误信息并延迟清除
    void showErrorForSeconds(int seconds)
    {
        thread t([this, seconds]() {
            this_thread::sleep_for(chrono::seconds(seconds));
            errorMessage.clear();
        });
        t.detach();
    }

    // 处理键盘输入
    void handleInput()
    {
        while (true)
        {
            char ch = getch();
            if (terminalMode)
            {
                    cout << "\nTerminal Mode (Input exit to exit):" << endl;
                    chdir(currentPath.c_str());
                    system("$SHELL");
                    terminalMode = false;
                    render();
                    continue;
            }

            if (ch == '\033')
            {
                getch();
                char arrow = getch();
                if (arrow == 'A')
                {
                    currentIndex = (currentIndex > 0) ? currentIndex - 1 : fileList.size() - 1;
                }
                else if (arrow == 'B')
                {
                    currentIndex = (currentIndex + 1) % fileList.size();
                }
                else if (arrow == 'D')
                {
                    string parentPath = fs::path(currentPath).parent_path().string();
                    if (!parentPath.empty())
                    {
                        loadFiles(parentPath);
                    }
                }
            }
            else if (ch == '\n' || ch == '\r')
            {
                const auto& [name, isDir] = fileList[currentIndex];
                string selectedPath = currentPath + "/" + name;
                if (isDir)
                {
                    try
                    {
                        loadFiles(selectedPath);
                    }
                    catch (const exception& e)
                    {
                        errorMessage = "Error: You do not have permission to access this folder.";
                        this->showErrorForSeconds(3);

                        string parentPath = fs::path(currentPath).parent_path().string();
                        if (!parentPath.empty())
                        {
                            loadFiles(parentPath);
                        }
                    }
                }
            }
            else if (ch == 'd')
            {
                const auto& [name, isDir] = fileList[currentIndex];
                string selectedPath = currentPath + "/" + name;

                cout << "Are you sure you want to delete '" << name << "'? (Y/N): ";
                char confirm = getch();
                if (confirm == 'Y' || confirm == 'y')
                {
                    try
                    {
                        if (isDir)
                        {
                            fs::remove_all(selectedPath);
                        }
                        else
                        {
                            fs::remove(selectedPath);
                        }
                        loadFiles(currentPath);
                    }
                    catch (const exception& e)
                    {
                        errorMessage = "Error: You do not have permission to delete this file/folder.";
                        this->showErrorForSeconds(3);
                    }
                }
            }
            else if (ch == 's') 
            {
                terminalMode = true;
            }

            render(); 
        }
    }


    void run(const string& path)//入口
    {
        loadFiles(path);
        render();
        handleInput();
    }
};
