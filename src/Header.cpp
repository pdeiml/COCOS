#include "Header.hpp"
#include "Logger.hpp"

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
    }
}

void Header::SetFilePointer(FILE* file)
{
    fFilePointer = file;
}