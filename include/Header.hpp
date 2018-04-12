#ifndef __HEADER_HPP__
#define __HEADER_HPP__

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <time.h>

#include "Logger.hpp"

// some important Tag Idents (TTagHead.Ident) that we will need to read the most common content of a PTU file
// check the output of this program and consult the tag dictionary if you need more
#define TTTRTagNumRecords  "TTResult_NumberOfRecords"  // Number of TTTR Records in the File;
#define TTTRTagTTTRRecType "TTResultFormat_TTTRRecType"
#define TTTRTagRes         "MeasDesc_Resolution"       // Resolution for the Dtime (T3 Only)
#define TTTRTagGlobRes     "MeasDesc_GlobalResolution" // Global Resolution of TimeTag(T2) /NSync (T3)
#define FileTagEnd         "Header_End"                // Always appended as last tag (BLOCKEND)

// TagTypes  (TTagHead.Typ)
#define tyEmpty8      0xFFFF0008
#define tyBool8       0x00000008
#define tyInt8        0x10000008
#define tyBitSet64    0x11000008
#define tyColor8      0x12000008
#define tyFloat8      0x20000008
#define tyFloat8Array 0x2001FFFF
#define tyTDateTime   0x21000008
#define tyAnsiString  0x4001FFFF
#define tyWideString  0x4002FFFF
#define tyBinaryBlob  0xFFFFFFFF


class Header{
private:
    struct TgHd{
    char Ident[32];     // Identifier of the tag
    int Idx;            // Index for multiple tags or -1
    unsigned int Typ;  // Type of tag ty..... see const section
    long long TagValue; // Value of tag.
    } TagHead;

    FILE* fFilePointer;
    char Magic[8];
    char Version[8];
    char Buffer[40];
    int fTimecounter = 0;
    long long fNumOfRecords = -1;
    long long fRecordType = 0;
    double fResolution = 0.0;
    double fGlobalResolution = 0.0;
    long long fIGlobalResolution = 0;
    char* fAnsiBuffer;
    char* fWideBuffer;

    time_t TDateTime_TimeT( double Convertee );

public:
    bool ReadMagic();
    bool ReadVersion();
    bool ReadHeaderTag();
    long long GetRecordType() const;
    long long GetNumberOfRecords() const;
    double GetGlobalResolution() const;
    double GetIGlobalResolution() const;
    void SetFilePointer(FILE* file);
};

#endif