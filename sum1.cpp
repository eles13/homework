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
  int n;
  int m;
  cin>>n>>m;
  int mat[n][m];
  for(int i=0;i<n;i++)
  {
   for(int g=0;g<m; g++)
    mat[i][g]=rand()%100;
  }
    long sum=0;
    Timer t;
    for(int i=0;i<n;i++)
    {
     for(int g=0;g<m; g++)
     sum+=mat[i][g];
    }
    cout<<sum<<endl;

    return 0;
}
