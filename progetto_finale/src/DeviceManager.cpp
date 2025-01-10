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
                device->getPowerConsumption() * (1.0/60.0); // Consumo per minuto in kWh
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
    std::cout << "[Time] Impostato orario a " << formatTime() << "\n";
}

void DeviceManager::printConsumption() const {
    std::cout << "\n=== Report Consumo Energetico (00:00 - " << formatTime() << ") ===\n";
    double totalConsumption = 0.0;

    for (const auto& device : devices) {
        double consumption = deviceEnergyConsumption.at(device->getId());
        totalConsumption += consumption;
        
        std::cout << device->getName() << ":\n"
                  << "  Stato: " << (device->getStatus() ? "Attivo" : "Inattivo") << "\n"
                  << "  Energia " << (consumption < 0 ? "prodotta: " : "consumata: ")
                  << std::fixed << std::setprecision(2) << std::abs(consumption) << " kWh\n";
    }

    std::cout << "\nBilancio energetico totale: ";
    if (totalConsumption < 0) {
        std::cout << "Produzione netta di " << -totalConsumption << " kWh\n";
    } else {
        std::cout << "Consumo netto di " << totalConsumption << " kWh\n";
    }
    std::cout << "==========================================\n\n";
}

void DeviceManager::printDeviceConsumption(const std::string& deviceName) const {
    auto device = findDevice(deviceName);
    if (!device) {
        std::cout << "[Error] Dispositivo non trovato: " << deviceName << "\n";
        return;
    }

    double consumption = deviceEnergyConsumption.at(device->getId());
    std::cout << "\n=== Consumo " << deviceName << " (00:00 - " << formatTime() << ") ===\n"
              << "Stato: " << (device->getStatus() ? "Attivo" : "Inattivo") << "\n"
              << "Energia " << (consumption < 0 ? "prodotta: " : "consumata: ")
              << std::fixed << std::setprecision(2) << std::abs(consumption) << " kWh\n"
              << "==========================================\n\n";
}

void DeviceManager::resetTime() {
    currentTime = 0;
    for (const auto& device : devices) {
        if (device->getStatus()) {
            device->toggle(); // Spegne il dispositivo
        }
    }
    activeDevices.clear();
    initializeDeviceConsumption();
    std::cout << "[System] Tempo resettato a 00:00\n";
}

void DeviceManager::resetTimers() {
    deviceTimers.clear();
    std::cout << "[System] Timer rimossi\n";
}

void DeviceManager::resetAll() {
    currentTime = 0;
    activeDevices.clear();
    deviceTimers.clear();
    initializeDeviceConsumption();
    for (const auto& device : devices) {
        if (device->getStatus()) {
            device->toggle(); // Spegne il dispositivo
        }
    }
    std::cout << "[System] Sistema resettato completamente\n";
}
