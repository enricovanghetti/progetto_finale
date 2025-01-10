
#include "../include/Device.h"

void ManualDevice::toggle() {
    isOn = !isOn;
}

double ManualDevice::calculateConsumption(double hours) const {
    return isOn ? powerConsumption * hours : 0.0;
}

void ManualDevice::update(int currentTime) {
    (void)currentTime; //sooprime il waring per parametro inutilizzato
    // No updates for manual devices
}

void ManualDevice::printStatus() const {
    std::cout << "Device: " << name << " (Manual) - Status: "
              << (isOn ? "ON" : "OFF") << "\n";
}

void FCDevice::toggle() {
    isOn = !isOn;
    if (isOn) {
        std::cout << "[Device] " << name << " accesa. Si spegnerà dopo " << cycleDuration / 60 << " ore.\n";
    }
}

double FCDevice::calculateConsumption(double hours) const {
    return isOn ? powerConsumption * std::min(hours, cycleDuration / 60) : 0.0;
}

void FCDevice::update(int currentTime) {
    if (isOn && startTime >= 0 && currentTime - startTime >= cycleDuration) {
        isOn = false;
        std::cout << "[Device] " << name << " spento automaticamente dopo il ciclo prefissato.\n";
    }
}

void FCDevice::setStartTime(int time) {
    startTime = time;
}

void FCDevice::printStatus() const {
    std::cout << "Device: " << name << " (Fixed Cycle) - Status: "
              << (isOn ? "ON" : "OFF") << "\n";
}
