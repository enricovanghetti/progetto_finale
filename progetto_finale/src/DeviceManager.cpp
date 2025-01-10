#include "../include/DeviceManager.h"
#include <iostream>
#include <sstream>
#include <iomanip>

DeviceManager::DeviceManager(double maxPowerLimit) 
    : maxPowerLimit(maxPowerLimit), currentTime(0) {
    initializeDeviceConsumption();
}

void DeviceManager::initializeDeviceConsumption() {
    deviceEnergyConsumption.clear();
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

void DeviceManager::toggleDevice(const std::string& deviceName, int startAt) {
    auto device = findDevice(deviceName);
    if (!device) {
        std::cout << getCurrentTimeStamp() << "Error: Dispositivo '" << deviceName << "' non trovato\n";
        return;
    }

    if (startAt != -1) {
        auto fcDevice = dynamic_cast<FCDevice*>(device.get());
        if (fcDevice) {
            fcDevice->setTimer(startAt);
            activeDevices[device->getId()] = device;
            std::cout << getCurrentTimeStamp() << "Impostato timer per il dispositivo '"
                     << deviceName << "': accensione alle " << formatSpecificTime(startAt) << "\n";
            return;
        }
    }

    device->toggle();
    if (device->getStatus()) {
        activeDevices[device->getId()] = device;
        std::cout << getCurrentTimeStamp() << "Il dispositivo '" << deviceName << "' si è acceso\n";
    } else {
        activeDevices.erase(device->getId());
        std::cout << getCurrentTimeStamp() << "Il dispositivo '" << deviceName << "' si è spento\n";
    }
}

void DeviceManager::updateDeviceConsumption() {
    for (const auto& pair : activeDevices) {
        auto device = pair.second;
        if (device->getStatus()) {
            deviceEnergyConsumption[device->getId()] += 
                device->getPowerConsumption() * (1.0/60.0); // Consumo per minuto in kWh
        }
    }
}

void DeviceManager::checkPowerConsumption() {
    double totalPower = 0.0;
    for (const auto& pair : activeDevices) {
        totalPower += pair.second->getPowerConsumption();
    }

    if (totalPower > maxPowerLimit) {
        std::cout << getCurrentTimeStamp() << "Warning: Consumo totale supera il limite di " 
                 << maxPowerLimit << " kW!\n";
        std::vector<std::shared_ptr<Device>> toTurnOff;
        for (const auto& pair : activeDevices) {
            if (totalPower <= maxPowerLimit) break;
            auto device = pair.second;
            if (device->getPowerConsumption() > 0) {
                totalPower -= device->getPowerConsumption();
                toTurnOff.push_back(device);
            }
        }
        for (auto& device : toTurnOff) {
            device->toggle();
            activeDevices.erase(device->getId());
            std::cout << getCurrentTimeStamp() << "Il dispositivo '" << device->getName() 
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
        if (device->hasTimer()) {
            device->clearTimer();
            std::cout << getCurrentTimeStamp() << "Rimosso il timer dal dispositivo '" 
                     << device->getName() << "'\n";
        }
    }
}

void DeviceManager::resetAll() {
    resetTime();
    resetTimers();
}

void DeviceManager::printDeviceConsumption(const std::string& deviceName) const {
    auto device = findDevice(deviceName);
    if (!device) {
        std::cout << getCurrentTimeStamp() << "Error: Dispositivo non trovato: " << deviceName << "\n";
        return;
    }

    double consumption = deviceEnergyConsumption.at(device->getId());
    std::cout << getCurrentTimeStamp() << "Il dispositivo '" << deviceName 
              << "' ha attualmente " << (consumption < 0 ? "prodotto " : "consumato ")
              << std::fixed << std::setprecision(2) << std::abs(consumption) << " kWh\n";
}

void DeviceManager::printConsumption() const {
    double totalProduction = 0.0;
    double totalConsumption = 0.0;

    for (const auto& device : devices) {
        double consumption = deviceEnergyConsumption.at(device->getId());
        if (consumption < 0) {
            totalProduction += -consumption;
        } else {
            totalConsumption += consumption;
        }
    }

    std::cout << getCurrentTimeStamp() << "Attualmente il sistema ha prodotto " 
              << std::fixed << std::setprecision(2) << totalProduction 
              << " kWh e consumato " << totalConsumption << " kWh.\n";
    std::cout << "Nello specifico:\n";

    for (const auto& device : devices) {
        double consumption = deviceEnergyConsumption.at(device->getId());
        std::cout << "- Il dispositivo '" << device->getName() << "' ha "
                  << (consumption < 0 ? "prodotto " : "consumato ")
                  << std::fixed << std::setprecision(2) << std::abs(consumption) << " kWh\n";
    }
}

std::string DeviceManager::formatTime() const {
    return formatSpecificTime(currentTime);
}
