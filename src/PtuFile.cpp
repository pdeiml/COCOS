#include "PtuFile.hpp"

PtuFile::PtuFile(char* filename) :
    fFilename(filename)
{
}

bool PtuFile::ReadHeader()
{
    fHeader.SetFilePointer(fFile);
    if(!(fHeader.ReadMagic())){
        return false;
    }
    if(!(fHeader.ReadVersion())){
        return false;
    }
}

bool PtuFile::OpenPtuFile()
{
    if((fFile = std::fopen(fFilename, "rb")) == nullptr){
        GERROR << "Input file cannot be opened.";
        ClosePtuFile();
        return false;
    } else {
        GINFO << "Successfully opened ptu file.";
        return true;
    }
}

FILE* PtuFile::GetFilePointer()
{
    return fFile;
}

bool PtuFile::ClosePtuFile()
{
    std::fclose(fFile);
}