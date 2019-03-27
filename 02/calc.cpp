#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <iterator>
#include <typeinfo>
#include <exception>
#include <stdexcept>
#include <ios>
//#include <ctype>
using namespace std;
bad_exception exc;
vector<string> splitstr(const char *str, const size_t Size)
{
    size_t i = 0;
    vector<string> res;
    string temp = "";
    while (i < Size)
    {
        if (isspace(str[i]))
        {
            if (temp != "")
            {
                res.push_back(temp);
                temp = "";
            }
        }
        else if ((str[i] == '+') || (str[i] == '-') || (str[i] == '*') || (str[i] == '/'))
        {
            if (temp != "")
            {
                res.push_back(temp);
                temp = "";
            }
            string s(1, str[i]);
            res.push_back(s);
        }
        else if ((isdigit(str[i])))
            temp += str[i];
        else
        {
            throw exc ;
        }
        i++;
    }
    if (temp != "")
        res.push_back(temp);
    else
    {
        throw exc ;
    }
    return res;
}

void reducemins(vector<string> &exp, const size_t Size)
{
    int mcounter = 0;
    vector<string>::iterator it = exp.begin();
    if (*it == "-")
    {
        if (isdigit((*(it + 1))[0]))
        {
            exp.erase(it);
            (*it).insert((*it).begin(), '-');
        }
        else
        {
            throw exc ;
        }
    }
    while (it != exp.end())
    {
        if (*it == "-")
        {
            while (!isdigit((*it)[0]))
            {
                mcounter++;
                exp.erase(it);
            }
            if ((mcounter == 1) && (!isdigit((*(it - 1))[0])))
            {
                //exp.erase(it);
                (*it).insert((*it).begin(), '-');
            }
            else if (mcounter % 2 == 1)
                exp.insert(it, "-");
            else
                exp.insert(it, "+");
            mcounter = 0;
        }
        it++;
    }
}

void mult(vector<string> &exp, const size_t Size, vector<string>::iterator it)
{
    int op1 = stoi(*(it - 1));
    int op2 = stoi(*(it + 1));
    exp.erase(it + 1);
    exp.erase(it);
    exp.erase(it - 1);
    exp.insert(it - 1, to_string(op2 * op1));
}

void div(vector<string> &exp, const size_t Size, vector<string>::iterator it)
{
    int op1 = stoi(*(it - 1));
    int op2 = stoi(*(it + 1));
    exp.erase(it + 1);
    exp.erase(it);
    exp.erase(it - 1);
    if (op2 == 0)
    {
        throw exc;
    }
    exp.insert(it - 1, to_string(op1 / op2));
}

void pls(vector<string> &exp, const size_t Size, vector<string>::iterator it)
{
    int op1 = stoi(*(it - 1));
    int op2 = stoi(*(it + 1));
    exp.erase(it + 1);
    exp.erase(it);
    exp.erase(it - 1);
    exp.insert(it - 1, to_string(op2 + op1));
}

void mns(vector<string> &exp, const size_t Size, vector<string>::iterator it)
{
    int op1 = stoi(*(it - 1));
    int op2 = stoi(*(it + 1));
    exp.erase(it + 1);
    exp.erase(it);
    exp.erase(it - 1);
    exp.insert(it - 1, to_string(op1 - op2));
}

int calc(const char *raw, const size_t Size)
{
    try
    {
        vector<string> exp = splitstr(raw, Size);
        reducemins(exp, exp.size());
        vector<string>::iterator it = exp.begin();
        while (it != exp.end())
        {
            if (*it == "*")
            {
                mult(exp, exp.size(), it);
                continue;
            }
            else if (*it == "/")
            {
                div(exp, exp.size(), it);
                continue;
            }
            it++;
        }
        it = exp.begin();
        while (it != exp.end())
        {
            if (*it == "+")
            {
                pls(exp, exp.size(), it);
                continue;
            }
            else if (*it == "-")
            {
                mns(exp, exp.size(), it);
                continue;
            }
            it++;
        }
        return stoi(exp[0]);
    }
    catch (bad_exception& exc)
    {
        throw;
    }
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        cout << "error" << endl;
        return 1;
    }
    try
    {
        cout << calc(argv[1], strlen(argv[1])) << endl;
    }
    catch (bad_exception& exc)
    {
        cout << "error" << endl;
        return 1;
    }
    return 0;
}