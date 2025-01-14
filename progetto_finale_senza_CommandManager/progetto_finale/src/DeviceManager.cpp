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
bool DeviceManager::checkPowerForDevice(const std::shared_ptr<Device>& deviceToAdd) {
    if (isCheckingPower) return true;
    isCheckingPower = true;

    // Calcola il consumo totale considerando la potenza nominale del nuovo dispositivo
    double totalConsumption = deviceToAdd->powerConsumption;
    for (const auto& pair : activeDevices) {
        if (pair.second != deviceToAdd) {
            totalConsumption += pair.second->getPowerConsumption();
        }
    }

    if (totalConsumption > maxPowerLimit) {
        std::cout << "[" << getCurrentTimeStamp() << "] Warning: potenza massima superata (" 
                  << std::fixed << std::setprecision(1) << totalConsumption << "kW)" << std::endl;
        
        // Spegni dispositivi più vecchi finché non c'è spazio sufficiente
        auto it = deviceActivationOrder.end();
        while (totalConsumption > maxPowerLimit && it != deviceActivationOrder.begin()) {
            --it; // Itera dal più recente al meno recente
            auto device = *it;
            // Non spegnere il dispositivo che stiamo cercando di accendere o il sistema fotovoltaico
            if (device != deviceToAdd && device->getName() != "Impianto fotovoltaico") {
                device->toggle();
                std::cout << "[" << getCurrentTimeStamp() << "] Il dispositivo '" 
                          << device->getName() << "' è stato spento automaticamente" << std::endl;
                
                activeDevices.erase(device->getId());
                it = deviceActivationOrder.erase(it); // Erase aggiorna l'iteratore

                // Ricalcola il consumo totale
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

void DeviceManager::toggleDevice(const std::string& deviceName, int startAt) {
    std::string searchName = toLowercase(deviceName);
    for (const auto& device : devices) {
        if (toLowercase(device->getName()) == searchName) {
            if (startAt >= 0) {
                device->setTimer(startAt);
                return;
            }

            if (!device->getStatus()) {
                if (checkPowerForDevice(device)) {
                    device->toggle();
                    activeDevices[device->getId()] = device;
                    deviceActivationOrder.push_back(device);
                    
                    // Gestione dei dispositivi automatici
                    if (auto* fc = dynamic_cast<FCDevice*>(device.get())) {
                        if (fc->getCycleDuration() > 0) {
                            fc->setStartTime(currentTime); // Memorizza il tempo di inizio
                            std::cout << "[" << getCurrentTimeStamp() << "] Il dispositivo '"
                                      << device->getName() << "' si è acceso e si spegnerà automaticamente dopo "
                                      << (static_cast<int>(fc->getCycleDuration()) / 60) << " ore e " << (static_cast<int>(fc->getCycleDuration()) % 60) << " minuti\n";

                        }  
                    } else {
                        // Dispositivo manuale
                        std::cout << "[" << getCurrentTimeStamp() << "] Il dispositivo '"
                                << device->getName() << "' si è acceso\n";
                    }
                }
            } else {
                device->toggle();
                std::cout << "[" << getCurrentTimeStamp() << "] Il dispositivo '"
                        << device->getName() << "' si è spento\n";
                activeDevices.erase(device->getId());
                auto it = std::find(deviceActivationOrder.begin(), deviceActivationOrder.end(), device);
                if (it != deviceActivationOrder.end()) {
                    deviceActivationOrder.erase(it);
                }
                updateDeviceConsumption(currentTime);
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
    if (isCheckingPower) return;
    isCheckingPower = true;

    double totalConsumption = 0.0;
    for (const auto& pair : activeDevices) {
        totalConsumption += pair.second->getPowerConsumption();
    }

    if (totalConsumption > maxPowerLimit) {
        std::cout << "[" << getCurrentTimeStamp() << "] Warning: potenza massima superata (" 
                  << std::fixed << std::setprecision(1) << totalConsumption << "kW)" << std::endl;
    }

    isCheckingPower = false;
}


void DeviceManager::setTime(const std::string& time) {
    int hours, minutes;
    char colon;
    std::istringstream iss(time);
    
    if (iss >> hours >> colon >> minutes && colon == ':' && 
        hours >= 0 && hours < 24 && minutes >= 0 && minutes < 60) {
        
        std::cout << "[" << getCurrentTimeStamp() << "] L'orario attuale è " << formatSpecificTime(currentTime) << std::endl;

        int newTime = hours * 60 + minutes;

        if (newTime < currentTime) {
            std::cout << "[Error] Non è possibile impostare un orario precedente a quello attuale.\n";
            return;
        }

        if (newTime != currentTime) {
            for (int t = currentTime; t <= newTime; ++t) {
                updateDeviceConsumption(t);
                for (const auto& device : devices) {
                    // Gestione dispositivi manuali con timer
                    device->update(t);
                    
                    // Gestione dispositivi automatici
                    if (auto* fc = dynamic_cast<FCDevice*>(device.get())) {
                        if (device->getStatus()) {
                            int startTime = fc->getStartTime();
                            if (startTime >= 0) {
                                int elapsedMinutes = t - startTime;
                                if (elapsedMinutes >= fc->getCycleDuration() && device->getStatus()) {
                                    device->toggle(); // Spegni il dispositivo
                                    activeDevices.erase(device->getId());
                                    auto it = std::find(deviceActivationOrder.begin(), deviceActivationOrder.end(), device);
                                    if (it != deviceActivationOrder.end()) {
                                        deviceActivationOrder.erase(it);
                                    }
                                    int cycleDuration = fc->getCycleDuration();
                                    std::cout << "[" << formatSpecificTime(t) << "] Il dispositivo '"
                                            << device->getName() << "' ha completato il suo ciclo di "
                                            << (cycleDuration / 60) << " ore e " << (cycleDuration % 60)
                                            << " minuti ed è stato spento automaticamente\n";
                                    fc->setStartTime(-1); // Resetta il tempo di inizio
                                }
                            }
                        }
                    }
                }
            }
            currentTime = newTime;

            activeDevices.clear();
            deviceActivationOrder.clear();
            for (const auto& device : devices) {
                if (device->getStatus()) {
                    activeDevices[device->getId()] = device;
                    deviceActivationOrder.push_back(device);
                }
            }
            checkPowerConsumption();
        }

        std::cout << "[" << getCurrentTimeStamp() << "] Ora impostata: " << time << std::endl;
    } else {
        std::cout << "[Error] Formato tempo non valido. Usa HH:MM" << std::endl;
    }
}


void DeviceManager::showConsumption(const std::string& deviceName) const {
    if (deviceName.empty()) {
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
    } else {
        printDeviceConsumption(deviceName);
    }
}

void DeviceManager::printDeviceConsumption(const std::string& deviceName) const {
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
        std::cout << "- " << device->getName() << std::endl;
    }
}

void DeviceManager::removeDeviceTimer(const std::string& deviceName) {
    auto device = findDevice(deviceName);
    if (device) {
        device->clearTimer();
        std::cout << "[" << getCurrentTimeStamp() << "] Timer rimosso per il dispositivo '" 
                  << deviceName << "'" << std::endl;
    } else {
        std::cout << "[Error] Dispositivo non trovato: " << deviceName << std::endl;
        std::cout << "Dispositivi disponibili:" << std::endl;
        for (const auto& d : devices) {
            std::cout << "- " << d->getName() << std::endl;
        }
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

void DeviceManager::updateDeviceConsumption(int newTime) {
    double hours = (newTime - lastUpdateTime) / 60.0;
    
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
    lastUpdateTime = newTime;
}

void DeviceManager::initializeDeviceConsumption() {
    deviceEnergyConsumption.clear();
    for (const auto& device : devices) {
        deviceEnergyConsumption[device->getId()] = 0.0;
    }
}
