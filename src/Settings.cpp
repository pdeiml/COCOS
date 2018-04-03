#include "Settings.hpp"

void Settings::PrintSettings()
{
    std::string change;
    std::cout << "Current Settings:\n#################\n";
    std::cout << "Start evaluation time [is]:\t" << fStartEvalTime << " s\n";
    std::cout << "End evaluation time [ie]:\t" << fEndEvalTime << " s\n";
    std::cout << "Set input limitation [sl]:\t" << fTimeLimitation << "\n";
    std::cout << "Start time difference [ts]:\t" << fTauBegin << " ps\n";
    std::cout << "End time difference [te]:\t" << fTauEnd << " ps\n";
    std::cout << "Number of bins [nb]:\t\t" << fNumberOfBins << "\n";
    std::cout << "Calibration mode [cm]:\t\t" << fCalibrationMode << "\n";
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
    } else if (change == "ts"){
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
        std::string calibrationmode;
        std::cout << "Enter r (=read), w (=write) or n (=none)";
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
    return fNumberOfBins;
}

int Settings::GetTauBegin() const
{
    return fTauBegin;
}

int Settings::GetTauEnd() const
{
    return fTauEnd;
}

std::string Settings::GetCalibrationMode() const
{
    return fCalibrationMode;
}

long long Settings::GetStarEvalTime() const
{
    return fStartEvalTime;
}

long long Settings::GetEndEvalTime() const
{
    return fEndEvalTime;
}

bool Settings::GetTimeLimitation() const
{
    return fTimeLimitation;
}

void Settings::SetCalibrationMode(std::string calibrationmode)
{
    fCalibrationMode = calibrationmode;
}

void Settings::SetStartEvalTime(long long starteval)
{
    fStartEvalTime = starteval;
}

void Settings::SetEndEvalTime(long long endeval)
{
    fEndEvalTime = endeval;
}

void Settings::SetTimeLimitation(bool inputlimit)
{
    fTimeLimitation = inputlimit;
}

void Settings::SetTauBegin(int taubegin)
{
    fTauBegin = taubegin;
}

void Settings::SetTauEnd(int tauend)
{
    fTauBegin = tauend;
}

void Settings::SetNumberOfBins(int numofbins)
{
    fNumberOfBins = numofbins;
}
