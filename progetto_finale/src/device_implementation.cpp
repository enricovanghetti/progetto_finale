#include "../include/Device.h"

// ManualDevice implementations
void ManualDevice::toggle() {
    isOn = !isOn;
}

double ManualDevice::calculateConsumption(double hours) const {
    return isOn ? (powerConsumption * hours) : 0.0;
}

void ManualDevice::update(int currentTime) {
    if (hasTimerSet) {
        if (currentTime >= scheduledStartTime && (scheduledStopTime == -1 || currentTime < scheduledStopTime)) {
            if (!isOn) {
                isOn = true;
                std::cout << "[" << formatTime(currentTime) << "] Il dispositivo '"
                          << name << "' si è acceso automaticamente\n";
            }
        } else {
            if (isOn) {
                isOn = false;
                std::cout << "[" << formatTime(currentTime) << "] Il dispositivo '"
                          << name << "' si è spento automaticamente\n";
            }
        }
    }
}


void ManualDevice::printStatus() const {
    std::cout << "Device: " << name << " (Manual) - Status: "
              << (isOn ? "ON" : "OFF");
    if (hasTimerSet) {
        std::cout << " - Timer: " << formatTime(scheduledStartTime);
        if (scheduledStopTime != -1) {
            std::cout << " - " << formatTime(scheduledStopTime);
        }
    }
    std::cout << "\n";
}

void ManualDevice::setTimer(int start, int stop) {
    hasTimerSet = true;
    scheduledStartTime = start;
    scheduledStopTime = stop;
}

void ManualDevice::clearTimer() {
    hasTimerSet = false;
    scheduledStartTime = -1;
    scheduledStopTime = -1;
}

int ManualDevice::getScheduledStartTime() const {
    return scheduledStartTime;
}

int ManualDevice::getScheduledStopTime() const {
    return scheduledStopTime;
}

// FCDevice implementations
void FCDevice::toggle() {
    isOn = !isOn;
    if (isOn && !hasTimerSet) {
        startTime = -1;
    }
}

double FCDevice::calculateConsumption(double hours) const {
    return isOn ? (powerConsumption * std::min(hours, cycleDuration / 60.0)) : 0.0;
}

void FCDevice::update(int currentTime) {
    if (!isOn && hasTimerSet && currentTime >= scheduledStartTime) {
        isOn = true;
        startTime = currentTime;
        std::cout << "[" << formatTime(currentTime) << "] Il dispositivo '"
                  << name << "' si è acceso automaticamente\n";
    }
    else if (isOn) {
        if (startTime >= 0 && currentTime - startTime >= cycleDuration) {
            isOn = false;
            std::cout << "[" << formatTime(currentTime) << "] Il dispositivo '"
                      << name << "' si è spento dopo il ciclo prefissato\n";
            clearTimer();
        }
    }
}

void FCDevice::setStartTime(int time) {
    startTime = time;
}

void FCDevice::printStatus() const {
    std::cout << "Device: " << name << " (Fixed Cycle) - Status: "
              << (isOn ? "ON" : "OFF");
    if (hasTimerSet) {
        std::cout << " - Timer: " << formatTime(scheduledStartTime);
    }
    std::cout << "\n";
}

void FCDevice::setTimer(int start, int stop) {
    (void)stop; // Ignoriamo lo stop time per i dispositivi FC
    hasTimerSet = true;
    scheduledStartTime = start;
    startTime = -1;
}

void FCDevice::clearTimer() {
    hasTimerSet = false;
    scheduledStartTime = -1;
    startTime = -1;
}

int FCDevice::getScheduledStartTime() const {
    return scheduledStartTime;
}

