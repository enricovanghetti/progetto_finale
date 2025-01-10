#include "../include/Device.h"

std::string formatTime(int minutes) {
    int hours = (minutes / 60) % 24;
    int mins = minutes % 60;
    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << hours << ":"
        << std::setw(2) << std::setfill('0') << mins;
    return oss.str();
}

void ManualDevice::toggle() {
    isOn = !isOn;
}

double ManualDevice::calculateConsumption(double hours) const {
    return isOn ? powerConsumption * hours : 0.0;
}

void ManualDevice::update(int currentTime) {
    (void)currentTime; // sopprime il warning per parametro inutilizzato
}

void ManualDevice::printStatus() const {
    std::cout << "Device: " << name << " (Manual) - Status: "
              << (isOn ? "ON" : "OFF") << "\n";
}

void FCDevice::toggle() {
    isOn = !isOn;
    if (isOn) {
        std::cout << "[Device] " << name << " acceso";
        if (hasTimerSet) {
            std::cout << ". Si spegnerà automaticamente dopo " 
                     << cycleDuration / 60.0 << " ore";
        }
        std::cout << ".\n";
    } else {
        std::cout << "[Device] " << name << " spento.\n";
    }
}

double FCDevice::calculateConsumption(double hours) const {
    if (!isOn) return 0.0;
    return powerConsumption * std::min(hours, cycleDuration / 60.0);
}

void FCDevice::update(int currentTime) {
    if (!isOn && hasTimerSet && currentTime == scheduledStartTime) {
        isOn = true;
        startTime = currentTime;
        std::cout << "[" << formatTime(currentTime) << "] Il dispositivo '" 
                 << name << "' si è acceso automaticamente\n";
    }
    else if (isOn) {
        if (hasTimerSet && currentTime == scheduledEndTime) {
            isOn = false;
            std::cout << "[" << formatTime(currentTime) << "] Il dispositivo '" 
                     << name << "' si è spento automaticamente\n";
        }
        else if (startTime >= 0 && currentTime - startTime >= cycleDuration) {
            isOn = false;
            std::cout << "[" << formatTime(currentTime) << "] Il dispositivo '" 
                     << name << "' si è spento dopo il ciclo prefissato\n";
        }
    }
}

void FCDevice::setStartTime(int time) {
    startTime = time;
}

void FCDevice::clearTimer() {
    hasTimerSet = false;
    scheduledStartTime = -1;
    scheduledEndTime = -1;
}

void FCDevice::setTimer(int start, int end) {
    hasTimerSet = true;
    scheduledStartTime = start;
    scheduledEndTime = end;
}

void FCDevice::printStatus() const {
    std::cout << "Device: " << name << " (Fixed Cycle) - Status: "
              << (isOn ? "ON" : "OFF");
    if (hasTimerSet) {
        std::cout << " - Timer: " << formatTime(scheduledStartTime) 
                 << " - " << formatTime(scheduledEndTime);
    }
    std::cout << "\n";
}
