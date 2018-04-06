#ifndef __HEADER_HPP__
#define __HEADER_HPP__

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include "Logger.hpp"

class Header{
private:
    FILE* fFilePointer;
    char Magic[8];
    char Version[8];

public:
    bool ReadMagic();
    bool ReadVersion();
    void SetFilePointer(FILE* file);
};

#endif