#include <iostream>
#pragma once
#include <sstream>
#include <string>
#include <type_traits>
#include <typeinfo>
using namespace std;
enum class Error
{
    NoError,
    CorruptedArchive
};

class Serializer
{
    ostream &out_;
    static constexpr char Separator = ' ';

  public:
    explicit Serializer(ostream &out)
        : out_(out)
    {
    }

    template <class T>
    Error save(T &object)
    {
        return object.serialize(*this);
    }

    template <class... ArgsT>
    Error operator()(ArgsT... args)
    {
        return process(args...);
    }

  private:
    template <class T>
    Error process(T &&val)
    {
        if (*(typeid(T).name()) == 'b')
        {
            out_ << (val == true ? "true" : "false");
        }
        else if (*(typeid(T).name()) == 'm')
        {
            out_ << val;
        }
        else
            return Error::CorruptedArchive;
        return Error::NoError;
    }
    template <class T, class... Args>
    Error process(T &&val, Args &&... args)
    {
        if (*(typeid(T).name()) == 'b')
        {
            out_ << (val == true ? "true" : "false") << Separator;
        }
        else if (*(typeid(T).name()) == 'm')
        {
            out_ << val << Separator;
        }
        else
            return Error::CorruptedArchive;
        return process(forward<Args>(args)...);
    }
};

class Deserializer
{
    istream &in_;
    static constexpr char Separator = ' ';

  public:
    explicit Deserializer(istream &in)
        : in_(in)
    {
    }

    template <class T>
    Error load(T &object)
    {
        //cout<<"call"<<endl;
        return object.serialize(*this);
    }

    template <class... ArgsT>
    Error operator()(ArgsT &&... args)
    {
        return process(forward<ArgsT>(args)...);
    }

  private:
    // process использует variadic templates
    template <class T>
    Error process(T &&val)
    {
        string s;
        in_ >> s;
        if (s == "")
            return Error::CorruptedArchive;
        if (checkBool(val))
        {
            if ((s == "true") || (s == "false"))
            {
                val = (s == "true" ? true : false);
                return Error::NoError;
            }
        }
        else if (checkType(val))
        {
            stringstream st(s);
            if (s.rfind("-", 0) == 0)
                return Error::CorruptedArchive;
            st >> val;
            return Error::NoError;
        }
        return Error::CorruptedArchive;
    }

    template <class T, class... Args>
    Error process(T &&val, Args &&... args)
    {
        string s;
        in_ >> s;
        if (s == "")
            return Error::CorruptedArchive;
        if (checkBool(val))
        {
            if ((s == "true") || (s == "false"))
            {
                val = (s == "true" ? true : false);
            }
            else
            {
                return Error::CorruptedArchive;
            }
        }
        else if (checkType(val))
        {
            stringstream st(s);
            st >> val;
        }
        else
        {
            return Error::CorruptedArchive;
        }
        return process(forward<Args>(args)...);
    }

  public:
    template <typename T>
    bool checkType(T &val)
    {
        if (is_same<uint64_t, T>::value)
            return true;
        else
            return false;
    }

    template <typename T>
    bool checkBool(T &val)
    {
        if (is_same<bool, T>::value)
            return true;
        else
            return false;
    }
};
