#include <iostream>
#include <thread>
#include <fstream>
#include <mutex>
#include <algorithm>
#include <iterator>
#include <vector>
#include <deque>
#include <condition_variable>
using namespace std;
const int N = 1000000;
mutex mut;
bool processing = true;
condition_variable cond;
bool notified = false;
void mergef(const char *file1, const char *file2)
{
    unique_lock<mutex> lock(mut);
    while (true)
    {
        while (!notified)
        {
            cond.wait(lock);
            if (!processing) return;
        }    
        deque<uint64_t> dfile2;
        uint64_t buf1;
        bool pres1 = false;
        uint64_t buf2;
        bool pres2 = false;
        ifstream f1(file1,ios::binary);
        ifstream f2(file2,ios::binary);
        uint64_t temp;
        while (!f2.eof())
        {

            f2.read((char*)&temp,sizeof(uint64_t));
            dfile2.push_back(temp);
        }
        f2.close();
        ofstream res(file2, ios::trunc|ios::binary);
        while (!f1.eof() && !f2.eof())
        {
            if (!pres1)
            {
                f1.read((char*)&buf1,sizeof(uint64_t));
                pres1 = true;
            }
            if (!pres2)
            {
                buf2 = dfile2[0];
                dfile2.pop_front();
                pres2 = true;
            }
            if (buf1 < buf2)
            {
                res.write((char*)&buf1,sizeof(uint64_t));
                pres1 = false;
            }
            else
            {
                res.write((char*)&buf2,sizeof(uint64_t));
                pres2 = false;
            }
        }
        if (pres1)
        {
            res.write((char*)&buf1,sizeof(uint64_t));
            while (!f1.eof())
            {
                f1.read((char*)&buf1,sizeof(uint64_t));
                res.write((char*)&buf1,sizeof(uint64_t));
            }
        }
        else if (pres2)
        {
            res.write((char*)&buf2,sizeof(uint64_t));
            while (!f2.eof())
            {
                buf2 = dfile2[0];
                dfile2.pop_front();
                res.write((char*)&buf2,sizeof(uint64_t));
            }
        }
        dfile2.clear();
        f1.close();
        res.close();
        remove(file1);
        rename(file2, file1);
        notified = false;
        cond.notify_one();
    }
}

void readf(const char * filename)
{
    ifstream file(filename, ios::binary);
    vector<uint64_t>nums;
    size_t wasread;
    uint64_t temp;
    unique_lock<mutex> lock(mut);
    while (true)
    {
        while (notified)
            cond.wait(lock);
        if (file.eof())
        {
            processing = false;
            cond.notify_one();
            break;
        }
        ofstream ofile("temp.dat", ios::trunc|ios::binary);
        for (wasread = 0;wasread<N-2 && !file.eof();wasread++)
        {
            file.read((char*)&temp,sizeof(temp));
            nums.push_back(temp);
        }
        sort(nums.begin(),nums.end());
        for (size_t i = 0;i<=wasread;i++)
        {
            ofile.write((char*)&(nums[i]),sizeof(uint64_t));
        }
        nums.clear();
        ofile.close();
        notified = true;
        cond.notify_one();
    }
    file.close();
}

int main(int argc, char **argv)
{
    if (argc != 2)
        throw "no proper file";
    thread t1(readf, argv[1]);
    thread t2(mergef,"res.dat", "temp.dat");
    t1.join();
    t2.join();
    return 0;
}