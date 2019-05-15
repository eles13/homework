#include <iostream>
#include <thread>
#include <fstream>
#include <mutex>
#include <algorithm>
#include <iterator>
#include <vector>
#include <deque>
#include <condition_variable>
#include <exception>
#include <atomic>
using namespace std;
const int N = 1000000;
atomic<int> state{0};
void mergef(const char *file1, const char *file2)
{
    while (true)
    {
        if (state == 0) continue;
        if(state<0) return;
        deque<uint64_t> dfile2;
        uint64_t buf1;
        bool pres1 = false;
        uint64_t buf2;
        bool pres2 = false;
        ifstream f1;
        f1.open(file1,ios::binary);
        if (!f1.good()) throw invalid_argument("");
        ifstream f2;
        f2.open(file2,ios::binary);
        if (!f2.good()) throw invalid_argument("");
        uint64_t temp;
        while (!f2.eof())
        {

            f2.read((char*)&temp,sizeof(uint64_t));
            dfile2.push_back(temp);
        }
        f2.close();
        ofstream res;
        res.open(file2, ios::trunc|ios::binary);
        if (!res.good()) throw invalid_argument("");
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
        state = 0;
    }
}

void readf(const char * filename)
{
    ifstream file;
    file.open(filename, ios::binary);
    if (!file.good()) throw invalid_argument("");
    vector<uint64_t>nums;
    size_t wasread;
    uint64_t temp;
    while (true)
    {   
        if (state == 1) continue;
        if (file.eof())
        {
            processing = false;
            state = -1;
            break;
        }
        ofstream ofile;
        ofile.open("temp.dat", ios::trunc|ios::binary);
        if (!ofile.good()) throw invaild_argument("");
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
        state = 1;
    }
    file.close();
}

int main(int argc, char **argv)
{
    try
    {
        thread t1(readf, "numbers.dat");
        thread t2(mergef,"res.dat", "temp.dat");
        t1.join();
        t2.join();
    }
    catch(invalid_argument& ex)
    {
        cout<<"file hasn't been opened"<<endl;
    }
    return 0;
}
