
#include "../include/DeviceManager.h"
#include <iostream>
#include <sstream>
#include <iomanip>

DeviceManager::DeviceManager(double maxPowerLimit) : maxPowerLimit(maxPowerLimit), currentTime(0) {}

void DeviceManager::addDevice(const std::shared_ptr<Device>& device) {
    devices.push_back(device);
}

void DeviceManager::toggleDevice(const std::string& deviceName, int startAt) {
    for (const auto& device : devices) {
        if (device->getName() == deviceName) {
            if (startAt != -1) {
                auto fcDevice = dynamic_cast<FCDevice*>(device.get());
                if (fcDevice) {
                    fcDevice->setStartTime(startAt);
                    fcDevice->toggle();
                    activeDevices[device->getId()] = device;
                    return;
                }
            }

            device->toggle();
            if (device->getStatus()) {
                activeDevices[device->getId()] = device;
                auto fcDevice = dynamic_cast<FCDevice*>(device.get());
                if (fcDevice) {
                    fcDevice->setStartTime(currentTime);
                }
            } else {
                activeDevices.erase(device->getId());
            }
            return;
        }
    }
    std::cout << "[Error] Dispositivo " << deviceName << " non trovato.\n";
}

void DeviceManager::checkPowerConsumption() {
    double totalPower = 0;
    for (const auto& pair : activeDevices) {
        totalPower += pair.second->getPowerConsumption();
    }

    if (totalPower > maxPowerLimit) {
        std::cout << "[Warning] Consumo totale supera il limite di " << maxPowerLimit << " kW!\n";
        for (auto it = activeDevices.rbegin(); it != activeDevices.rend(); ++it) {
            if (totalPower <= maxPowerLimit) break;
            totalPower -= it->second->getPowerConsumption();
            it->second->toggle();
            std::cout << "[Manager] Dispositivo " << it->second->getName() << " spento per rientrare nel limite.\n";
        }
    }
}

void DeviceManager::setTime(const std::string& time) {
    int hours, minutes;
    char sep;
    std::istringstream iss(time);
    iss >> hours >> sep >> minutes;
    int targetTime = hours * 60 + minutes;

    while (currentTime < targetTime) {
        currentTime++;
        for (const auto& pair : activeDevices) {
            pair.second->update(currentTime);
        }
    }
    std::cout << "[Time] Impostato orario a " << formatTime() << "\n";
}

void DeviceManager::printConsumption() const {
    double totalPower = 0;
    for (const auto& pair : activeDevices) {
        totalPower += pair.second->getPowerConsumption();
    }
    std::cout << "[Info] Consumo totale attuale: " << totalPower << " kW\n";
}

std::string DeviceManager::formatTime() const {
    int hours = currentTime / 60;
    int minutes = currentTime % 60;
    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << hours << ":"
        << std::setw(2) << std::setfill('0') << minutes;
    return oss.str();
}
