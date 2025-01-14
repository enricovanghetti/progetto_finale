#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <string>
#include <sstream>
#include <iomanip>

class TimeManager {
private:
    int currentTime;
    int lastUpdateTime;

public:
    TimeManager() : currentTime(0), lastUpdateTime(0) {}

    void setTime(const std::string& time);
    void resetTime();
    std::string formatTime() const;
    std::string getCurrentTimeStamp() const;
    std::string formatSpecificTime(int minutes) const;
    int getCurrentTime() const { return currentTime; }
    int getLastUpdateTime() const { return lastUpdateTime; }
    void setLastUpdateTime(int time) { lastUpdateTime = time; }
};
#endif