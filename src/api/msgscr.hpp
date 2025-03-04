//+++ Message Box +++
#include <iostream>

using namespace std;

namespace cuistd
{
    class MsgScr
    {
        public:
            void display()
            {
                system("clear");
                cout << title << endl;
                cout << "   ["+Sign+"]" << endl;
                cout << "\n       "+message << endl;
                this_thread::sleep_for(chrono::seconds(seconds));
            }
        private:
            string title = "Message Screen";
            string Sign = "Message";
            string message = "This is a message screen\n        and this is a test message";
            int seconds = 5;
    };
}