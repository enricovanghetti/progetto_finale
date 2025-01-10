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
    double maxPowerLimit;
    int currentTime;
    std::map<int, double> deviceRunningTime;  // Nuovo: per tracciare il tempo di esecuzione

public:
    explicit DeviceManager(double maxPowerLimit);

    void addDevice(const std::shared_ptr<Device>& device);
    void toggleDevice(const std::string& deviceName, int startAt = -1);
    void checkPowerConsumption();
    void setTime(const std::string& time);
    void printConsumption() const;
    std::string formatTime() const;
    double getDeviceRunningTime(int deviceId) const;  // Nuovo: per ottenere il tempo di esecuzione
};

#endif
