#include "PtuFile.hpp"

// Constructors
// ============

PtuFile::PtuFile(char* filename) :
    fFilename(filename)
{
}

// Public member functions
// =======================

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

// Getter functions
// ----------------


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

FILE* PtuFile::GetFilePointer()
{
    return fFile;
}

// Open & close file

bool PtuFile::OpenPtuFile()
{
    if((fFile = std::fopen(fFilename, "rb")) == nullptr){
        std::cerr << "Error:\tInput file cannot be opened." << "\n";
        ClosePtuFile();
        exit(1);
        return false;
    } else {
        std::cout << "Successfully opened ptu file." << "\n";
        return true;
    }
}

bool PtuFile::ClosePtuFile()
{
    std::fclose(fFile);
}