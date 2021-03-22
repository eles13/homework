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
void mergef(const char *file1, const char *file2, atomic<int>& state)
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
        if (!f1.good()) 
        {
            state = -1;
            cout<<"cannot open "<<file1<<endl;
            return;
        }
        ifstream f2;
        f2.open(file2,ios::binary);
        if (!f2.good()) 
        {
            state = -1;
            cout<<" cannot open "<<file2<<endl;
            return;
        }
        uint64_t temp;
        while (!f2.eof())
        {

            f2.read((char*)&temp,sizeof(uint64_t));
            dfile2.push_back(temp);
        }
        f2.close();
        ofstream res;
        res.open(file2, ios::trunc|ios::binary);
        if (!res.good()) 
        {
            state = -1;
            cout<<"cannot open "<<file2<<endl;
            return;
        }
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

void readf(const char * filename, atomic<int>& state)
{
    const int N = 1000000;
    ifstream file;
    file.open(filename, ios::binary);
    if (!file.good())
    {
        state = -1;
        cout<<"cannot open "<<filename<<endl;
        return;
    }
    vector<uint64_t>nums;
    size_t wasread;
    uint64_t temp;
    while (true)
    {   
        if (state == 1) continue;
        if (file.eof())
        {
            state = -1;
            break;
        }
        ofstream ofile;
        ofile.open("temp.dat", ios::trunc|ios::binary);
        if (!ofile.good() || state == -1)
        {
            state = -1;
            cout<<"cannot open temp.dat"<<endl;
            return;
        }
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
    atomic<int> state{0};
    thread t1(readf, "numbers.dat",ref(state));
    thread t2(mergef,"res.dat", "temp.dat",ref(state));
    t1.join();
    t2.join();
    return 0;
}