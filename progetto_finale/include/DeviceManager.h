#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <vector>
#include <map>
#include <memory>
#include <string>
#include <algorithm>
#include "Device.h"

class DeviceManager {
    std::vector<std::shared_ptr<Device>> devices;
    std::map<int, std::shared_ptr<Device>> activeDevices;
    std::map<int, double> deviceEnergyConsumption;
    std::vector<std::shared_ptr<Device>> deviceActivationOrder;
    double maxPowerLimit;
    int currentTime;
    int lastUpdateTime;

public:
    explicit DeviceManager(double maxPowerLimit);

    void addDevice(const std::shared_ptr<Device>& device);
    void toggleDevice(const std::string& deviceName, int startAt = -1);
    void checkPowerConsumption();
    void setTime(const std::string& time);
    void showConsumption(const std::string& deviceName = "") const; // Modifica qui
    void resetTime();
    void resetTimers();
    void resetAll();
    void removeDeviceTimer(const std::string& deviceName); // Nuovo metodo
    std::string formatTime() const;
    std::shared_ptr<Device> findDevice(const std::string& deviceName) const;
    const std::vector<std::shared_ptr<Device>>& getDevices() const { return devices; }

private:
    std::string getCurrentTimeStamp() const;
    std::string formatSpecificTime(int minutes) const;
    void updateDeviceConsumption();
    void initializeDeviceConsumption();
    void printDeviceConsumption(const std::string& deviceName) const; // Aggiungi la dichiarazione qui
    
    static std::string toLowercase(const std::string& str) {
        std::string lower = str;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        return lower;
    }
};

#endif
