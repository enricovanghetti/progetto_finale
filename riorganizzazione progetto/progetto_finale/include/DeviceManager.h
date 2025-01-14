#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <vector>
#include <memory>
#include <string>
#include "Device.h"
#include "TimeManager.h"
#include "ConsumptionManager.h"

class DeviceManager {
private:
    std::vector<std::shared_ptr<Device>> devices;
    TimeManager timeManager;
    ConsumptionManager consumptionManager;

public:
    explicit DeviceManager(double maxPowerLimit);
    void addDevice(const std::shared_ptr<Device>& device);
    void toggleDevice(const std::string& deviceName, int startAt = -1);
    void setTime(const std::string& time);
    void showConsumption(const std::string& deviceName = "");
    void resetTime();
    void resetTimers();
    void resetAll();
    void removeDeviceTimer(const std::string& deviceName);
    std::string formatTime() const;
    std::shared_ptr<Device> findDevice(const std::string& deviceName) const;
    const std::vector<std::shared_ptr<Device>>& getDevices() const { return devices; }

private:
    static std::string toLowercase(const std::string& str);
};

#endif