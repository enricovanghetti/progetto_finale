#ifndef CONSUMPTION_MANAGER_H
#define CONSUMPTION_MANAGER_H

#include <map>
#include <memory>
#include <vector>
#include "Device.h"

class ConsumptionManager {
private:
    std::map<int, double> deviceEnergyConsumption;
    double maxPowerLimit;
    std::vector<std::shared_ptr<Device>> deviceActivationOrder;
    std::map<int, std::shared_ptr<Device>> activeDevices;
    bool isCheckingPower;

public:
    explicit ConsumptionManager(double maxPowerLimit) 
        : maxPowerLimit(maxPowerLimit), isCheckingPower(false) {}

    void initializeDeviceConsumption();
    void updateDeviceConsumption(int currentTime, int lastUpdateTime);
    void showConsumption(const std::string& deviceName, const std::string& currentTimeStr) const;
    bool checkPowerForDevice(const std::shared_ptr<Device>& deviceToAdd, const std::string& currentTimeStr);
    void checkPowerConsumption(const std::string& currentTimeStr);
    
    std::vector<std::shared_ptr<Device>>& getDeviceActivationOrder() { return deviceActivationOrder; }
    const std::map<int, std::shared_ptr<Device>>& getActiveDevices() const { return activeDevices; }
    void addDevice(const std::shared_ptr<Device>& device);
    void clearActivationData();

private:
    void printDeviceConsumption(const std::string& deviceName, const std::string& currentTimeStr) const;
    std::vector<std::shared_ptr<Device>> devices;
};
#endif