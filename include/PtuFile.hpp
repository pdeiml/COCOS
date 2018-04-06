#ifndef __PTUFILE_HPP__
#define __PTUFILE_HPP__

#include <string>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "Header.hpp"
#include "Logger.hpp"

class PtuFile{
private:
    char* fFilename;
    FILE* fFile;
    Header fHeader;

public:
    PtuFile(char* filename);
    bool ReadHeader();
    bool OpenPtuFile();
    bool ClosePtuFile();
    FILE* GetFilePointer();
};

#endif