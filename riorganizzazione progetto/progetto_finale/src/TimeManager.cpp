#include "TimeManager.h"
#include <iostream>

void TimeManager::setTime(const std::string& time) {
    int hours, minutes;
    char colon;
    std::istringstream iss(time);
    
    if (iss >> hours >> colon >> minutes && colon == ':' && 
        hours >= 0 && hours < 24 && minutes >= 0 && minutes < 60) {
        
        int newTime = hours * 60 + minutes;
        if (newTime < currentTime) {
            std::cout << "[Error] Non è possibile impostare un orario precedente a quello attuale.\n";
            return;
        }
        currentTime = newTime;
    } else {
        std::cout << "[Error] Formato tempo non valido. Usa HH:MM" << std::endl;
    }
}

void TimeManager::resetTime() {
    currentTime = 0;
    lastUpdateTime = 0;
}

std::string TimeManager::formatTime() const {
    return formatSpecificTime(currentTime);
}

std::string TimeManager::getCurrentTimeStamp() const {
    return formatSpecificTime(currentTime);
}

std::string TimeManager::formatSpecificTime(int minutes) const {
    int hours = minutes / 60;
    int mins = minutes % 60;
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << hours << ":"
        << std::setfill('0') << std::setw(2) << mins;
    return oss.str();
}