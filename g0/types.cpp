#include "g0/types.h"

//pushers
#include <cstdlib>
#include <cstdio>
//pushers






/////////////////////////////////////////////////////////
stringExtended& stringExtended::operator+=(char *charStr)
{
     push_charStr(charStr);
     return *this;
}

stringExtended& stringExtended::operator+=(const char *charStr)
{
     push_constCharStr(charStr);
     return *this;
}

stringExtended& stringExtended::operator+=(std::string value)
{
     push_constCharStr(value.c_str());
     return *this;
}


stringExtended& stringExtended::operator+=(stringExtended &strEx)
{
    push_strEx(strEx);
    return *this;
}

stringExtended& stringExtended::operator+=(ul &ulongValue)
{
    push_ul(ulongValue);
     return *this;
}

stringExtended& stringExtended::operator+=(long &longValue)
{
    push_long(longValue);
     return *this;
}

stringExtended& stringExtended::operator+=(double &doubleValue)
{
    push_double(doubleValue);
    return *this;
}

//stringExtended& operator+= em prime object?

stringExtended& stringExtended::operator<<(int iValue)
{
    push_int(iValue);
    return *this;
}

stringExtended& stringExtended::operator<<(char *charStr)
{
    push_charStr(charStr);
    return *this;
}

stringExtended& stringExtended::operator<<(const char *charStr)
{
    push_constCharStr(charStr);
    return *this;
}

stringExtended& stringExtended::operator<<(std::string value)
{
    push_constCharStr(value.c_str());
    return *this;
}




stringExtended& stringExtended::operator<<(stringExtended &strEx)
{
    push_strEx(strEx);
    return *this;
}

stringExtended& stringExtended::operator<<(ul &ulongValue)
{
    push_ul(ulongValue);
   return *this;
}

stringExtended& stringExtended::operator<<(long &longValue)
{
    push_long(longValue);
    return *this;
}

stringExtended& stringExtended::operator<<(double &doubleValue)
{
    push_double(doubleValue);
    return *this;
}

stringExtended& stringExtended::operator=(const char *charStr)
{
    inside.clear();
    push_constCharStr(charStr);
    return *this;
}

stringExtended& stringExtended::operator=(std::string value)
{
    inside.clear();
    push_constCharStr(value.c_str());
    return *this;
}


stringExtended& stringExtended::operator=(char *charStr)
{
    inside.clear();
    push_charStr(charStr);
    return *this;
}

stringExtended& stringExtended::operator=(int intValue)
{
    inside.clear();
    push_int(intValue);
    return *this;
}

stringExtended& stringExtended::operator=(stringExtended &strEx)
{
    inside.clear();
    push_strEx(strEx);
    return *this;
}
stringExtended& stringExtended::operator=(ul &ulongValue)
{
    inside.clear();
    push_ul(ulongValue);
    return *this;
}

stringExtended& stringExtended::operator=(long &longValue)
{
    inside.clear();
    push_long(longValue);
    return *this;
}

stringExtended& stringExtended::operator=(double &doubleValue)
{
    inside.clear();
    push_double(doubleValue);
    return *this;
}
//AUTO SEPPARATORS??
void stringExtended::push_charStr(char *charStr)
{
    inside += charStr;
}

void stringExtended::push_constCharStr(const char *charStr)
{
    inside += charStr;
}


void stringExtended::push_int(int&iValue)
{
    char bufer[32]; //hmm

    std::string intValue;
    sprintf(bufer,"%d",iValue);
    intValue = bufer;

    inside += intValue;
}

void stringExtended::push_ul(ul& ulongValue)
{
    char bufer[32]; //hmm

    std::string intValue;
    sprintf(bufer,"%llu",ulongValue); //u
    intValue = bufer;

    inside += intValue;
}

void stringExtended::push_long(long& longValue)
{
    char bufer[32];

    std::string intValue;
    sprintf(bufer,"%ld",longValue);
    intValue = bufer;

    inside += intValue;
}

void stringExtended::push_double(double&doubleValue)
{
    char bufer[32];

    std::string fValue;
    sprintf(bufer,"%f",doubleValue);
    fValue = bufer;

    inside += fValue;
}

void stringExtended::push_strEx(stringExtended& strEx)
{
    inside += strEx.c_str();
}
