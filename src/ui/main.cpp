//CLI UI
//By XYLCS/XIT and LosefDevLab
//Open Source License by GPL V3.0
//Free-Creator Standard by FREE-CREATOR-SHOW(《自由创作者创作精神宣言（XYLCS原版）》)
/*
1.创作应该是自由性的。
2.创作需要改正，此处指的改正，不只是创作过程中出现的错误，而是整体上的不良氛围或错误氛围、思想。
3.创作需要不断创新。
4.创作不限年龄，不限阶级，不限政治，人人平等。
5.创作应该保持协作。
6.创作应该保持永不停歇。
*/
//Des.:A UI in CLI by ASCII Art

//+++ MAIN +++
#include <iostream>
#include "UI.hpp"
#include "yourmain.cpp"

using namespace std;
using namespace cuistd;
using namespace youruistd;

extern bool isOther = false;

int main()
{
    cout << "Welcome to CLIUI" << endl;//TODO:记得改版本号
    cout << "Version 1 - 3\n        Press enter run UI" << endl;
    if (cin.get() == '\n' && !isOther)
    {
        UI ui;
        ui.run("/");
    }
    else
    {
        youruiMain uiyours;
        uiyours.run();
    }
    return 0;
}