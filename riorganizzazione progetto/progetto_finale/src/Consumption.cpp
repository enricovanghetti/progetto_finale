#include "ConsumptionManager.h"
#include <iostream>
#include <iomanip>

void ConsumptionManager::initializeDeviceConsumption() {
    deviceEnergyConsumption.clear();
    for (const auto& device : devices) {
        deviceEnergyConsumption[device->getId()] = 0.0;
    }
}

void ConsumptionManager::updateDeviceConsumption(int currentTime, int lastUpdateTime) {
    double hours = (currentTime - lastUpdateTime) / 60.0;
    if (hours > 0) {
        for (const auto& device : devices) {
            if (device->getStatus()) {
                auto it = deviceEnergyConsumption.find(device->getId());
                if (it != deviceEnergyConsumption.end()) {
                    it->second += device->calculateConsumption(hours);
                }
            }
        }
    }
}

void ConsumptionManager::showConsumption(const std::string& deviceName, const std::string& currentTimeStr) const {
    if (deviceName.empty()) {
        std::cout << "[" << currentTimeStr << "] Consumo energetico:" << std::endl;
        double totalConsumption = 0.0;
        
        for (const auto& device : devices) {
            auto it = deviceEnergyConsumption.find(device->getId());
            if (it != deviceEnergyConsumption.end()) {
                std::cout << device->getName() << ": " 
                          << std::fixed << std::setprecision(2) << it->second << " kWh" << std::endl;
                totalConsumption += it->second;
            }
        }
        
        std::cout << "Consumo totale: " << std::fixed << std::setprecision(2) 
                  << totalConsumption << " kWh" << std::endl;
    } else {
        printDeviceConsumption(deviceName, currentTimeStr);
    }
}

bool ConsumptionManager::checkPowerForDevice(const std::shared_ptr<Device>& deviceToAdd, const std::string& currentTimeStr) {
    if (isCheckingPower) return true;
    isCheckingPower = true;

    double totalConsumption = deviceToAdd->powerConsumption;
    for (const auto& pair : activeDevices) {
        if (pair.second != deviceToAdd) {
            totalConsumption += pair.second->getPowerConsumption();
        }
    }

    if (totalConsumption > maxPowerLimit) {
        std::cout << "[" << currentTimeStr << "] Warning: potenza massima superata (" 
                  << std::fixed << std::setprecision(1) << totalConsumption << "kW)" << std::endl;
        
        auto it = deviceActivationOrder.end();
        while (totalConsumption > maxPowerLimit && it != deviceActivationOrder.begin()) {
            --it;
            auto device = *it;
            if (device != deviceToAdd && device->getName() != "Impianto fotovoltaico") {
                device->toggle();
                activeDevices.erase(device->getId());
                it = deviceActivationOrder.erase(it);

                totalConsumption = deviceToAdd->powerConsumption;
                for (const auto& pair : activeDevices) {
                    if (pair.second != deviceToAdd) {
                        totalConsumption += pair.second->getPowerConsumption();
                    }
                }
            }
        }
    }

    isCheckingPower = false;
    return totalConsumption <= maxPowerLimit;
}

void ConsumptionManager::checkPowerConsumption(const std::string& currentTimeStr) {
    if (isCheckingPower) return;
    isCheckingPower = true;

    double totalConsumption = 0.0;
    for (const auto& pair : activeDevices) {
        totalConsumption += pair.second->getPowerConsumption();
    }

    if (totalConsumption > maxPowerLimit) {
        std::cout << "[" << currentTimeStr << "] Warning: potenza massima superata (" 
                  << std::fixed << std::setprecision(1) << totalConsumption << "kW)" << std::endl;
    }

    isCheckingPower = false;
}

void ConsumptionManager::addDevice(const std::shared_ptr<Device>& device) {
    devices.push_back(device);
    deviceEnergyConsumption[device->getId()] = 0.0;
}

void ConsumptionManager::clearActivationData() {
    activeDevices.clear();
    deviceActivationOrder.clear();
}

void ConsumptionManager::printDeviceConsumption(const std::string& deviceName, const std::string& currentTimeStr) const {
    for (const auto& device : devices) {
        if (device->getName() == deviceName) {
            auto it = deviceEnergyConsumption.find(device->getId());
            if (it != deviceEnergyConsumption.end()) {
                std::cout << "[" << currentTimeStr << "] Consumo " << device->getName() 
                          << ": " << std::fixed << std::setprecision(2) << it->second << " kWh" << std::endl;
                return;
            }
        }
    }
    std::cout << "[Error] Dispositivo non trovato: " << deviceName << std::endl;
}