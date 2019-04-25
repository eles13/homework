#include <iostream>
#include <thread>
#include <atomic>
using namespace std;
const uint maxi = 1000000;
atomic<uint> printed{0};
void outprint(uint id)
{
    while (true)
    {
        if (printed >= maxi-1)
            break;
        if (printed % 2 == id)
        {
            cout << (id == 0 ? "ping" : "pong") <<endl;
            printed++;
        }
    }
}
int main()
{
    thread thread0(outprint, 0);
    thread thread1(outprint, 1);
    thread0.join();
    thread1.join();
    return 0;
}