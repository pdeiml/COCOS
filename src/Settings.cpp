#include "Settings.hpp"

Settings::Settings(){
    fSettingsMap.insert(std::make_pair("is", 0));
    fSettingsMap.insert(std::make_pair("ie", 0));
    fSettingsMap.insert(std::make_pair("sl", 0));
    fSettingsMap.insert(std::make_pair("ts", -250e3));
    fSettingsMap.insert(std::make_pair("te", 250e3));
    fSettingsMap.insert(std::make_pair("nb", 2000));
    fSettingsMap.insert(std::make_pair("cm", 0));
}

void Settings::PrintSettings()
{
    std::string change;
    std::cout << "Current Settings:\n#################\n";
    std::cout << "Start evaluation time [is]:\t" << fSettingsMap["is"] << " s\n";
    std::cout << "End evaluation time [ie]:\t" << fSettingsMap["ie"] << " s\n";
    std::cout << "Set input limitation [sl]:\t" << fSettingsMap["sl"] << "\n";
    std::cout << "Start time difference [tb]:\t" << fSettingsMap["tb"] << " ps\n";
    std::cout << "End time difference [te]:\t" << fSettingsMap["te"] << " ps\n";
    std::cout << "Number of bins [nb]:\t\t" << fSettingsMap["nb"] << "\n";
    std::cout << "Calibration mode [cm]:\t\t" << fSettingsMap["cm"] << "\n";
    std::cout << "\nDo you want to change something (c for continue)? ";
    std::cin >> change;
    ChangeSettings(change);
}

void Settings::ChangeSettings(std::string change)
{
    if (change == "is"){
        long long starteval;
        std::cout << "Enter new start evaluation time [s]: ";
        std::cin >> starteval;
        SetStartEvalTime(starteval);
    } else if (change == "ie"){
        long long endeval;
        std::cout << "Enter new start evaluation time [s]: ";
        std::cin >> endeval;
        SetEndEvalTime(endeval);
    } else if (change == "sl"){
        bool inputlimit;
        std::cout << "Enter 0 (=false) or 1 (=true): ";
        std::cin >> inputlimit;
        SetTimeLimitation((bool)inputlimit);
    } else if (change == "tb"){
        int taubegin;
        std::cout << "Enter new start time difference [ns]: ";
        std::cin >> taubegin;
        SetTauBegin(taubegin);
    } else if (change == "te"){
        int tauend;
        std::cout << "Enter new start time difference [ns]: ";
        std::cin >> tauend;
        SetTauEnd(tauend);
    } else if (change == "nb"){
        int numofbins;
        std::cout << "Enter new number of bins: ";
        std::cin >> numofbins;
        SetNumberOfBins(numofbins);
    } else if (change == "cm"){
        int calibrationmode;
        std::cout << "Enter 1 (=read), 2 (=write) or 0 (=none): ";
        std::cin >> calibrationmode;
        SetCalibrationMode(calibrationmode);
    } else if (change == "c"){
    } else {
        std::cout << "Wrong parameter!" << std::endl;
    }
    PrintSettings();
}

int Settings::GetNumberOfBins() const
{
    return (int)(fSettingsMap.find("nb") -> second);
}

int Settings::GetTauBegin() const
{
    return (int)(fSettingsMap.find("tb") -> second);
}

int Settings::GetTauEnd() const
{
    return (int)(fSettingsMap.find("te") -> second);
}

int Settings::GetCalibrationMode() const
{
    return (int)(fSettingsMap.find("cm") -> second);
}

long long Settings::GetStartEvalTime() const
{
    return (long long)(fSettingsMap.find("is") -> second);
}

long long Settings::GetEndEvalTime() const
{
    return (long long)(fSettingsMap.find("ie") -> second);
}

bool Settings::GetTimeLimitation() const
{
    return (bool)(fSettingsMap.find("sl") -> second);
}

void Settings::SetCalibrationMode(int calibrationmode)
{
    fSettingsMap["cm"] = calibrationmode;
}

void Settings::SetStartEvalTime(long long starteval)
{
    fSettingsMap["is"] = starteval;
}

void Settings::SetEndEvalTime(long long endeval)
{
    fSettingsMap["ie"] = endeval;
}

void Settings::SetTimeLimitation(bool inputlimit)
{
    fSettingsMap["sl"] = inputlimit;
}

void Settings::SetTauBegin(int taubegin)
{
    fSettingsMap["tb"] = taubegin;
}

void Settings::SetTauEnd(int tauend)
{
    fSettingsMap["te"] = tauend;
}

void Settings::SetNumberOfBins(int numofbins)
{
    fSettingsMap["nb"] = numofbins;
}
