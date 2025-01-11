#include "../include/DeviceManager.h"
#include <iomanip>
#include <iostream>
#include <sstream>

DeviceManager::DeviceManager(double maxPowerLimit) 
: maxPowerLimit(maxPowerLimit), currentTime(0), lastUpdateTime(0) {
    initializeDeviceConsumption();
}

void DeviceManager::addDevice(const std::shared_ptr<Device>& device) {
    devices.push_back(device);
    deviceEnergyConsumption[device->getId()] = 0.0;
}

void DeviceManager::toggleDevice(const std::string& deviceName, int startAt) {
    std::string searchName = toLowercase(deviceName);
    for (const auto& device : devices) {
        if (toLowercase(device->getName()) == searchName) {
            if (startAt >= 0) {
                device->setTimer(startAt);
                return;
            }

            bool wasOn = device->getStatus();
            device->toggle();

            if (device->getStatus()) {
                activeDevices[device->getId()] = device;
                deviceActivationOrder.push_back(device);
                std::cout << "[" << getCurrentTimeStamp() << "] Il dispositivo '" 
                          << device->getName() << "' si è acceso" << std::endl;
            } else {
                if (wasOn) {
                    updateDeviceConsumption();
                }
                activeDevices.erase(device->getId());
                auto it = std::find_if(deviceActivationOrder.begin(), deviceActivationOrder.end(),
                                       [&device](const std::shared_ptr<Device>& d) { return d->getId() == device->getId(); });
                if (it != deviceActivationOrder.end()) {
                    deviceActivationOrder.erase(it);
                }
                std::cout << "[" << getCurrentTimeStamp() << "] Il dispositivo '" 
                          << device->getName() << "' si è spento" << std::endl;
            }
            return;
        }
    }
    std::cout << "[Error] Dispositivo non trovato: " << deviceName << "\n";
    std::cout << "Dispositivi disponibili:\n";
    for (const auto& device : devices) {
        std::cout << "- " << device->getName() << "\n";
    }
}

void DeviceManager::checkPowerConsumption() {
    double totalConsumption = 0.0;
    for (const auto& pair : activeDevices) {
        totalConsumption += pair.second->getPowerConsumption();
    }

    if (totalConsumption > maxPowerLimit) {
        std::cout << "[" << getCurrentTimeStamp() << "] Warning: potenza massima superata (" 
                  << std::fixed << std::setprecision(1) << totalConsumption << "kW)" << std::endl;

        for (auto it = deviceActivationOrder.rbegin() + 1; it != deviceActivationOrder.rend(); ++it) {
            if ((*it)->getName() != "photovoltaic system") {
                (*it)->toggle();
                std::cout << "[" << getCurrentTimeStamp() << "] Il dispositivo '" 
                          << (*it)->getName() << "' è stato spento automaticamente" << std::endl;

                activeDevices.erase((*it)->getId());
                auto orderIt = std::find(deviceActivationOrder.begin(), deviceActivationOrder.end(), *it);
                if (orderIt != deviceActivationOrder.end()) {
                    deviceActivationOrder.erase(orderIt);
                }

                totalConsumption = 0.0;
                for (const auto& pair : activeDevices) {
                    totalConsumption += pair.second->getPowerConsumption();
                }

                if (totalConsumption <= maxPowerLimit) {
                    break;
                }
            }
        }
    }
}

void DeviceManager::setTime(const std::string& time) {
    int hours, minutes;
    char colon;
    std::istringstream iss(time);

    if (iss >> hours >> colon >> minutes && colon == ':' && 
        hours >= 0 && hours < 24 && minutes >= 0 && minutes < 60) {
        int newTime = hours * 60 + minutes;

        if (newTime > currentTime) {
            updateDeviceConsumption(); // Aggiorna il consumo fino ad ora
            currentTime = newTime; // Imposta il nuovo tempo

            for (const auto& device : devices) {
                device->update(newTime);
                if (device->getStatus()) {
                    activeDevices[device->getId()] = device;
                    if (std::find(deviceActivationOrder.begin(), deviceActivationOrder.end(), device) 
                        == deviceActivationOrder.end()) {
                        deviceActivationOrder.push_back(device);
                    }
                } else {
                    auto it = activeDevices.find(device->getId());
                    if (it != activeDevices.end()) {
                        activeDevices.erase(it);
                    }
                    auto orderIt = std::find(deviceActivationOrder.begin(), deviceActivationOrder.end(), device);
                    if (orderIt != deviceActivationOrder.end()) {
                        deviceActivationOrder.erase(orderIt);
                    }
                }
            }

            lastUpdateTime = newTime; // Aggiorna lastUpdateTime alla fine
        } else {
            currentTime = newTime;
            lastUpdateTime = newTime;
        }

        std::cout << "[" << getCurrentTimeStamp() << "] Ora impostata: " << time << std::endl;
    } else {
        std::cout << "[Error] Formato tempo non valido. Usa HH:MM" << std::endl;
    }
}

void DeviceManager::printConsumption() {
    updateDeviceConsumption(); // Aggiorna i consumi prima di mostrarli

    std::cout << "[" << getCurrentTimeStamp() << "] Consumo energetico:" << std::endl;
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
}

void DeviceManager::printDeviceConsumption(const std::string& deviceName) {
    updateDeviceConsumption(); // Aggiorna i consumi prima di mostrarli

    std::string searchName = toLowercase(deviceName);
    for (const auto& device : devices) {
        if (toLowercase(device->getName()) == searchName) {
            auto it = deviceEnergyConsumption.find(device->getId());
            if (it != deviceEnergyConsumption.end()) {
                std::cout << "[" << getCurrentTimeStamp() << "] Consumo " << device->getName() 
                          << ": " << std::fixed << std::setprecision(2) << it->second << " kWh" << std::endl;
                return;
            }
        }
    }
    std::cout << "[Error] Dispositivo non trovato: " << deviceName << std::endl;
    std::cout << "Dispositivi disponibili:" << std::endl;
    for (const auto& device : devices) {
        std::cout << "- " << device->getName() << "\n";
    }
}

void DeviceManager::resetTime() {
    currentTime = 0;
    lastUpdateTime = 0;
    deviceActivationOrder.clear();
    activeDevices.clear();
    initializeDeviceConsumption();
    std::cout << "[00:00] Ora reimpostata a 00:00" << std::endl;
}

void DeviceManager::resetTimers() {
    for (const auto& device : devices) {
        device->clearTimer();
    }
    std::cout << "[" << getCurrentTimeStamp() << "] Timer cancellati" << std::endl;
}

void DeviceManager::resetAll() {
    currentTime = 0;
    lastUpdateTime = 0;
    deviceActivationOrder.clear();
    activeDevices.clear();
    initializeDeviceConsumption();
    for (const auto& device : devices) {
        device->clearTimer();
        if (device->getStatus()) {
            device->toggle();
        }
    }
    std::cout << "[00:00] Sistema reimpostato" << std::endl;
}

std::string DeviceManager::formatTime() const {
    return formatSpecificTime(currentTime);
}

std::shared_ptr<Device> DeviceManager::findDevice(const std::string& deviceName) const {
    std::string searchName = toLowercase(deviceName);
    for (const auto& device : devices) {
        if (toLowercase(device->getName()) == searchName) {
            return device;
        }
    }
    return nullptr;
}

std::string DeviceManager::getCurrentTimeStamp() const {
    return formatSpecificTime(currentTime);
}

std::string DeviceManager::formatSpecificTime(int minutes) const {
    int hours = minutes / 60;
    int mins = minutes % 60;
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << hours << ":" 
        << std::setfill('0') << std::setw(2) << mins;
    return oss.str();
}

void DeviceManager::updateDeviceConsumption() {
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
    lastUpdateTime = currentTime;
}

void DeviceManager::initializeDeviceConsumption() {
    deviceEnergyConsumption.clear();
    for (const auto& device : devices) {
        deviceEnergyConsumption[device->getId()] = 0.0;
    }
}
