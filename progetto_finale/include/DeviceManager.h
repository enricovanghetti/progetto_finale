#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <vector>
#include <map>
#include <memory>
#include <string>
#include "Device.h"

class DeviceManager {
    std::vector<std::shared_ptr<Device>> devices;
    std::map<int, std::shared_ptr<Device>> activeDevices;
    std::map<int, double> deviceEnergyConsumption; // Nuovo: per tracciare il consumo totale
    double maxPowerLimit;
    int currentTime;
    std::map<int, int> deviceTimers; // Nuovo: per gestire i timer

public:
    explicit DeviceManager(double maxPowerLimit);

    void addDevice(const std::shared_ptr<Device>& device);
    void toggleDevice(const std::string& deviceName, int startAt = -1);
    void checkPowerConsumption();
    void setTime(const std::string& time);
    void printConsumption() const;
    void printDeviceConsumption(const std::string& deviceName) const; // Nuovo
    void resetTime(); // Nuovo
    void resetTimers(); // Nuovo
    void resetAll(); // Nuovo
    std::string formatTime() const;
    void updateDeviceConsumption(); // Nuovo

private:
    std::shared_ptr<Device> findDevice(const std::string& deviceName) const; // Nuovo
    void initializeDeviceConsumption(); // Nuovo
};

#endif
