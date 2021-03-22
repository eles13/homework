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
    Error operator()(ArgsT&&... args)
    {
        return process(forward<ArgsT>(args)...);
    }

  private:
    template <class T>
    Error pushit(T& val)
    {
        if (is_same<T, bool>::value)
            out_ << (val == true ? "true" : "false") << Separator;
        else if (is_same<T, uint64_t>::value)
            out_ << val << Separator;
        else
            return Error::CorruptedArchive;
        return Error::NoError;
    }
    template <class T>
    Error process(T &val)
    {
        return pushit(val);
    }
    template <class T, class... Args>
    Error process(T &val, Args &&... args)
    {
        if (pushit(val) == Error::CorruptedArchive)
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
    Error popit(T &val)
    {
        string s;
        in_ >> s;
        if (s == "")
            return Error::CorruptedArchive;
        // cout<<"type bool "<<is_same<T,bool>::value<<endl;
        // cout<<"type int "<<is_same<T,uint64_t>::value<<endl;
        // cout<<s<<endl;
        if (is_same<T, bool>::value)
        {
            if ((s == "true") || (s == "false"))
            {
                val = (s == "true" ? true : false);
                return Error::NoError;
            }
        }
        else if (is_same<T, uint64_t>::value)
        {
            if (s[0] == '-')
                return Error::CorruptedArchive;
            try
            {
                val = stoull(s);
            }
            catch (invalid_argument &ex)
            {
                return Error::CorruptedArchive;
            }
            return Error::NoError;
        }
        return Error::CorruptedArchive;
    }
    template <class T>
    Error process(T &val)
    {
        return popit(val);
    }

    template <class T, class... Args>
    Error process(T &val, Args &&... args)
    {
        if (popit(val) == Error::CorruptedArchive)
            return Error::CorruptedArchive;
        return process(forward<Args>(args)...);
    }
};
