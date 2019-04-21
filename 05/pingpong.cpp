#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
using namespace std;
const uint maxi = 1000000;
atomic<uint> printed{0};
mutex mut;

void outprint(uint id)
{
    while (true)
    {
        lock_guard<mutex> guard(mut);
        if (printed >= maxi)
            break;
        if (printed % 2 == id)
        {
            cout << (id == 0 ? "ping" : "pong") << endl;
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