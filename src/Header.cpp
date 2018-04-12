#include "Header.hpp"
#include "Logger.hpp"

// Private member functions
// ========================

time_t Header::TDateTime_TimeT(double Convertee)
{
    const int EpochDiff = 25569; // days between 30/12/1899 and 01/01/1970
    const int SecsInDay = 86400; // number of seconds in a day
    time_t Result((long)(((Convertee) - EpochDiff) * SecsInDay));
    return Result;
}

// Public member functions
// =======================

// Reader functions
// ----------------

bool Header::ReadMagic()
{
    int Result = fread(&Magic, 1, sizeof(Magic), fFilePointer);
    if (Result != sizeof(Magic)){
        GERROR << "Reading magic of ptu file failed.";
        return false;
    } else {
        if(strncmp(Magic, "PQTTTR", 6)){
            GERROR << "Wrong beginning of ptu header, this is not a PTU file!";
            return false;
        } else {
            GDEBUG << "Magic: " << Magic;
            return true;
        }
    }
}

bool Header::ReadVersion()
{
    int Result = fread(&Version, 1, sizeof(Version), fFilePointer);
    if (Result != sizeof(Version)){
        GERROR << "Reading version of ptu file failed.";
        return false;
    } else {
        GDEBUG << "Version: " << Version;
        return true;
    }
}

bool Header::ReadHeaderTag()
{
    do{
        int Result = fread( &TagHead, 1, sizeof(TagHead), fFilePointer);
        if( Result != sizeof(TagHead)){
            GERROR << "Incomplete ptu file.";
            return false;
        }
        strcpy(Buffer, TagHead.Ident);
        if( TagHead.Idx > -1){
            sprintf( Buffer, "%s(%d)", TagHead.Ident, TagHead.Idx );
        }
        switch(TagHead.Typ){
            case tyEmpty8:
                break;
            case tyBool8:
                break;
            case tyInt8:
                fTimecounter ++;
                if( fTimecounter == 35 ){
                    GINFO << "Adjusted measurement time:\t" << 1e03 * TagHead.TagValue << " s";
                }
                if( strcmp( TagHead.Ident, TTTRTagNumRecords ) == 0){ // Number of rescords
                    fNumOfRecords = TagHead.TagValue;
                    GDEBUG << "Number of records:\t" << fNumOfRecords;
                }
                if( strcmp( TagHead.Ident, TTTRTagTTTRRecType) == 0 ){ // TTTR RecordType
                    fRecordType = TagHead.TagValue;
                }
                break;
            case tyBitSet64:
                break;
            case tyColor8:
                break;
            case tyFloat8:
                if (strcmp(TagHead.Ident, TTTRTagRes)==0){ // Resolution for TCSPC-Decay
                    fResolution = *(double*)&(TagHead.TagValue);
                }
                if (strcmp(TagHead.Ident, TTTRTagGlobRes)==0){ // Global resolution for timetag
                    fGlobalResolution = *(double*)&(TagHead.TagValue); // in ns
                    fIGlobalResolution = 1e12 * fGlobalResolution;
                }
                break;
            case tyFloat8Array:
                break;
            case tyTDateTime:
                time_t CreateTime;
                CreateTime = TDateTime_TimeT(*((double*)&(TagHead.TagValue)));
                break;
            case tyAnsiString:
                fAnsiBuffer = (char*)calloc((size_t)TagHead.TagValue,1);
                Result = fread(fAnsiBuffer, 1, (size_t)TagHead.TagValue, fFilePointer);
                if( Result != TagHead.TagValue){
                    GERROR << "Incomplete File.";
                    return false;
                }
                free (fAnsiBuffer);
                break;
            case tyWideString:
            fWideBuffer = (char*)calloc((size_t)TagHead.TagValue, 1);
                Result = fread(fWideBuffer, 1, (size_t)TagHead.TagValue, fFilePointer);
                if( Result != TagHead.TagValue){
                    GERROR << "Incomplete file.";
                    return false;
                }
                free( fWideBuffer );
                break;
            case tyBinaryBlob:
                break;
            default:
                GERROR << "Illegal type identifier found! Broken file?";
                return false;
        }
    } while (( strncmp( TagHead.Ident, FileTagEnd, sizeof( FileTagEnd ))));
}

// Getter functions
// ----------------

long long Header::GetRecordType() const
{
    return fRecordType;
}

long long Header::GetNumberOfRecords() const
{
    return fNumOfRecords;
}

double Header::GetGlobalResolution() const
{
    return fGlobalResolution;
}

double Header::GetIGlobalResolution() const
{
    return fIGlobalResolution;
}

// Setter functions
// ----------------

void Header::SetFilePointer(FILE* file)
{
    fFilePointer = file;
}