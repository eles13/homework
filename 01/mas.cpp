#include "numbers.dat"
#include <iostream>
#include <vector>
using namespace std;

bool test(const int num)
{
  if (num == 1)
    return false;
  bool sim = true;
  if (num == 2)
    return true;
  if ((num & 1) == 0)
    return false;
  if ((num > 5) && (num % 5 == 0))
    return false;
  int temp = num;
  int sum = 0;
  while (temp > 0)
  {
    sum += temp % 10;
    temp = int(temp / 10);
  }
  if ((num > 3) && (sum % 3 == 0))
    return false;
  for (int i = 3; i <= num / 2; i += 2)
  {
    if (num % i == 0)
    {
      sim = false;
      break;
    }
  }
  return sim;
}

int binsearch(const vector<int>& Data, const int size, const int left, const int right, const int node)
{
  if (left == right)
    return left;
  if (node < Data[0])
    return 0;
  int middle = int((right + left) / 2);
  if (Data[middle] < node)
  {
    if (middle + 1 < size)
    {
      if (node < Data[middle + 1])
        return middle + 1;
    }
    return binsearch(Data, size, middle, right, node);
  }
  else if (Data[middle] > node)
  {
    //cout<<'r'<<endl;
    if (middle > 0)
    {
      if (node > Data[middle - 1])
        return middle;
    }
    return binsearch(Data, size, left, middle, node);
  }
  else
    return middle;
}

int main(int argc, char **argv)
{
  if ((argc == 1) || (((argc - 1) & 1) != 0))
  {
    //cout<<-1<<endl;
    return -1;
  }
  vector<int> simnums;
  if (argc > 3)
  {
    for (int i = 0; i < Size; i++)
    {
      if (test(Data[i]))
      {
        simnums.push_back(Data[i]);
      }
    }
  }
  int count = 0;
  for (int gcnt = 0; gcnt < argc / 2; gcnt++)
  {
    int fnum = atoi(argv[gcnt * 2 + 1]);
    int lnum = atoi(argv[gcnt * 2 + 2]);
    int fpos = -1;
    int lpos = -1;
    if (fnum * lnum == 0)
    {
      return -1;
    }
    if (fnum > lnum)
    {
      cout << 0 << endl;
      return 0;
    }
    int i = 0;
    for (i = 0; i < Size; i++)
    {
      if (Data[i] == fnum)
      {
        fpos = i;
        break;
      }
    }
    if (i == Size)
    {
      cout << 0 << endl;
      return 0;
    }
    for (; i < Size; i++)
    {
      if (Data[i] == lnum)
      {
        lpos = i;
      }
    }
    if (lpos == -1)
    {
      cout << 0 << endl;
      return 0;
    }
    count = 0;
    i = 0;

    if (argc > 3)
    {
      i = binsearch(simnums, simnums.size(), 0, simnums.size(), fnum);
      for (; unsigned(i) < simnums.size(); i++)
      {
        if ((simnums[i] >= fnum) && (simnums[i] <= lnum))
        {
          count++;
          if (simnums[i] > lnum)
            break;
        }
      }
    }
    else
    {
      for (i = fpos; i <= lpos; i++)
      {
        if (test(Data[i]))
        {
          count++;
        }
      }
    }
    cout << count << endl;
  }
  return 0;
}
