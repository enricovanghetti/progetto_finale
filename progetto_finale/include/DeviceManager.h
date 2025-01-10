#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <vector>
#include <map>
#include <memory>
#include <string>
#include "Device.h"

class DeviceManager {
private:
    std::vector<std::shared_ptr<Device>> devices;
    std::map<int, std::shared_ptr<Device>> activeDevices;
    std::map<int, double> deviceEnergyConsumption;
    double maxPowerLimit;
    int currentTime;

    std::string getCurrentTimeStamp() const;
    std::string formatSpecificTime(int minutes) const;
    void initializeDeviceConsumption();
    std::shared_ptr<Device> findDevice(const std::string& deviceName) const;
    void updateDeviceConsumption();

public:
    explicit DeviceManager(double maxPowerLimit);

    void addDevice(const std::shared_ptr<Device>& device);
    void toggleDevice(const std::string& deviceName, int startAt = -1);
    void checkPowerConsumption();
    void setTime(const std::string& time);
    void printConsumption() const;
    void printDeviceConsumption(const std::string& deviceName) const;
    void resetTime();
    void resetTimers();
    void resetAll();
    std::string formatTime() const;
};

#endif
