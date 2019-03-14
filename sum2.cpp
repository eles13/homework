#include <iostream>
#include <chrono>
using namespace std;
class Timer
{
    using clock_t = std::chrono::high_resolution_clock;
    using microseconds = std::chrono::microseconds;
public:
    Timer()
        : start_(clock_t::now())
    {
    }

    ~Timer()
    {
        const auto finish = clock_t::now();
        const auto us =
            std::chrono::duration_cast<microseconds>
                (finish - start_).count();
        std::cout << us << " us" << std::endl;
    }

private:
    const clock_t::time_point start_;
};
int main()
{
  int maxarray;
  int maxcol;
  cin>>maxarray>>maxcol;
  int mat[maxarray][maxcol];
  for(int array=0;array<maxarray;array++)
  {
   for(int col=0;col<maxcol; col++)
    mat[array][col]=rand()%100;
  }
    long sum=0;
    Timer t;
    for(int col=0;col<maxcol;col++)
    {
     for(int array=0;array<maxarray; array++)
     sum+=mat[array][maxarray];
    }
    cout<<sum<<endl;

    return 0;
}
