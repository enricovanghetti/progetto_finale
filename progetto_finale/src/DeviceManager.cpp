#include "../include/DeviceManager.h"
#include <iostream>
#include <sstream>
#include <iomanip>

DeviceManager::DeviceManager(double maxPowerLimit) 
    : maxPowerLimit(maxPowerLimit), currentTime(0) {
    initializeDeviceConsumption();
}

void DeviceManager::initializeDeviceConsumption() {
    for (const auto& device : devices) {
        deviceEnergyConsumption[device->getId()] = 0.0;
    }
}

std::string DeviceManager::getCurrentTimeStamp() const {
    return "[" + formatTime() + "] ";
}

std::string DeviceManager::formatSpecificTime(int minutes) const {
    int hours = (minutes / 60) % 24;
    int mins = minutes % 60;
    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << hours << ":"
        << std::setw(2) << std::setfill('0') << mins;
    return oss.str();
}

void DeviceManager::addDevice(const std::shared_ptr<Device>& device) {
    devices.push_back(device);
    deviceEnergyConsumption[device->getId()] = 0.0;
}

std::shared_ptr<Device> DeviceManager::findDevice(const std::string& deviceName) const {
    for (const auto& device : devices) {
        if (device->getName() == deviceName) {
            return device;
        }
    }
    return nullptr;
}

void DeviceManager::updateDeviceConsumption() {
    for (const auto& pair : activeDevices) {
        auto device = pair.second;
        if (device->getStatus()) {
            deviceEnergyConsumption[device->getId()] += 
                device->getPowerConsumption() * (1.0/60.0);
        }
    }
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
                    std::cout << getCurrentTimeStamp() << "Impostato un timer per il dispositivo '"
                             << deviceName << "' dalle " << formatTime() << " alle "
                             << formatSpecificTime(startAt + static_cast<int>(fcDevice->getCycleDuration())) << "\n";
                    return;
                }
            }

            device->toggle();
            if (device->getStatus()) {
                activeDevices[device->getId()] = device;
                std::cout << getCurrentTimeStamp() << "Il dispositivo '" << deviceName << "' si è acceso\n";
                auto fcDevice = dynamic_cast<FCDevice*>(device.get());
                if (fcDevice) {
                    fcDevice->setStartTime(currentTime);
                }
            } else {
                activeDevices.erase(device->getId());
                std::cout << getCurrentTimeStamp() << "Il dispositivo '" << deviceName << "' si è spento\n";
            }
            return;
        }
    }
    std::cout << getCurrentTimeStamp() << "Error: Dispositivo '" << deviceName << "' non trovato\n";
}

void DeviceManager::checkPowerConsumption() {
    double totalPower = 0;
    for (const auto& pair : activeDevices) {
        totalPower += pair.second->getPowerConsumption();
    }

    if (totalPower > maxPowerLimit) {
        std::cout << getCurrentTimeStamp() << "Warning: Consumo totale supera il limite di " 
                 << maxPowerLimit << " kW!\n";
        for (auto it = activeDevices.rbegin(); it != activeDevices.rend(); ++it) {
            if (totalPower <= maxPowerLimit) break;
            totalPower -= it->second->getPowerConsumption();
            it->second->toggle();
            std::cout << getCurrentTimeStamp() << "Il dispositivo '" << it->second->getName() 
                     << "' è stato spento per rientrare nel limite\n";
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
        updateDeviceConsumption();
        for (const auto& pair : activeDevices) {
            pair.second->update(currentTime);
        }
    }
    std::cout << getCurrentTimeStamp() << "L'orario attuale è " << formatTime() << "\n";
}

void DeviceManager::resetTime() {
    currentTime = 0;
    for (const auto& device : devices) {
        if (device->getStatus()) {
            device->toggle();
        }
    }
    activeDevices.clear();
    initializeDeviceConsumption();
    std::cout << "[00:00] L'orario attuale è 00:00\n";
}

void DeviceManager::resetTimers() {
    for (const auto& device : devices) {
        auto fcDevice = dynamic_cast<FCDevice*>(device.get());
        if (fcDevice && fcDevice->hasTimer()) {
            std::cout << getCurrentTimeStamp() << "Rimosso il timer dal dispositivo '" 
                     << device->getName() << "'\n";
            fcDevice->clearTimer();
        }
    }
}

void DeviceManager::resetAll() {
    currentTime = 0;
    activeDevices.clear();
    for (const auto& device : devices) {
        if (device->getStatus()) {
            device->toggle();
        }
        auto fcDevice = dynamic_cast<FCDevice*>(device.get());
        if (fcDevice) {
            fcDevice->clearTimer();
