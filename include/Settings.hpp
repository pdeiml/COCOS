#ifndef __SETTINGS_HPP__
#define __SETTINGS_HPP__

#include <string>
#include <iostream>
#include <map>

class Settings{
private:
    std::map<std::string, long long> fSettingsMap;

public:
    Settings();

    void PrintSettings();
    void ChangeSettings(std::string change);

    int GetNumberOfBins() const;
    int GetTauBegin() const;
    int GetTauEnd() const;
    int GetCalibrationMode() const;
    long long GetStartEvalTime() const;
    long long GetEndEvalTime() const;
    bool GetTimeLimitation() const;

    void SetStartEvalTime(long long starteval);
    void SetEndEvalTime(long long endeval);
    void SetTimeLimitation(bool inputlimit);
    void SetTauBegin(int taubegin);
    void SetTauEnd(int tauend);
    void SetCalibrationMode(int calibrationmode);
    void SetNumberOfBins(int numberofbins);
};

#endif