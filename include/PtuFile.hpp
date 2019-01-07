#ifndef __PTUFILE_HPP__
#define __PTUFILE_HPP__

#include <string>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "Header.hpp"

class PtuFile{
private:
    char* fFilename;
    FILE* fFile;
    Header fHeader;

public:
    PtuFile(char* filename);
    bool ReadHeader();

    // Getter functions
    long long GetRecordType() const;
    long long GetNumberOfRecords() const;
    double GetGlobalResolution() const;
    double GetIGlobalResolution() const;
    FILE* GetFilePointer();

    // Open & close file
    bool OpenPtuFile();
    bool ClosePtuFile();
};

#endif