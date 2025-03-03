//+++ MAIN +++
#include <iostream>
#include "UI.hpp"

using namespace std;

int main()
{
    cout << "Welcome to CLIUI" << endl;//TODO:记得改版本号
    cout << "Version 1 - 2\n        Press enter run UI" << endl;
    if (cin.get() == '\n')
    {
        UI ui;
        ui.run("/");
    }
}