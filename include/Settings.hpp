#ifndef __SETTINGS_HPP__
#define __SETTINGS_HPP__

#include <string>
#include <iostream>

class Settings{
private:
    long long fStartEvalTime = 0;
    long long fEndEvalTime = 0;
    bool fTimeLimitation = false;
    int fTauBegin = -250e3;
    int fTauEnd = 250e3;
    int fNumberOfBins = 2000;
    std::string fCalibrationMode = "n";

public:
    long long GetStartEvalTime() const 
    {
        return fStartEvalTime;
    }

    void PrintSettings();
    void ChangeSettings(std::string change);

    int GetNumberOfBins() const;
    int GetTauBegin() const;
    int GetTauEnd() const;
    std::string GetCalibrationMode() const;
    long long GetStarEvalTime() const;
    long long GetEndEvalTime() const;
    bool GetTimeLimitation() const;

    void SetStartEvalTime(long long starteval);
    void SetEndEvalTime(long long endeval);
    void SetTimeLimitation(bool inputlimit);
    void SetTauBegin(int taubegin);
    void SetTauEnd(int tauend);
    void SetCalibrationMode(std::string calibrationmode);
    void SetNumberOfBins(int numberofbins);
};

#endif