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
    fHeader.ReadHeaderTag();
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

long long PtuFile::GetRecordType() const
{
    fHeader.GetRecordType();
}

long long PtuFile::GetNumberOfRecords() const
{
    fHeader.GetNumberOfRecords();
}

double PtuFile::GetGlobalResolution() const
{
    fHeader.GetGlobalResolution();
}

double PtuFile::GetIGlobalResolution() const
{
    fHeader.GetIGlobalResolution();
}

bool PtuFile::ClosePtuFile()
{
    std::fclose(fFile);
}