#include "DeviceManager.h"
#include <iomanip>
#include <iostream>
#include <sstream>

DeviceManager::DeviceManager(double maxPowerLimit) 
    : consumptionManager(maxPowerLimit) {
}

void DeviceManager::addDevice(const std::shared_ptr<Device>& device) {
    devices.push_back(device);
    consumptionManager.addDevice(device);
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
                if (consumptionManager.checkPowerForDevice(device, timeManager.getCurrentTimeStamp())) {
                    device->toggle();
                    auto& activeDevices = consumptionManager.getActiveDevices();
                    activeDevices[device->getId()] = device;
                    consumptionManager.getDeviceActivationOrder().push_back(device);
                    
                    // Gestione dei dispositivi automatici
                    if (auto* fc = dynamic_cast<FCDevice*>(device.get())) {
                        if (fc->getCycleDuration() > 0) {
                            fc->setStartTime(timeManager.getCurrentTime());
                            std::cout << "[" << timeManager.getCurrentTimeStamp() << "] Il dispositivo '"
                                      << device->getName() << "' si è acceso e si spegnerà automaticamente dopo "
                                      << (static_cast<int>(fc->getCycleDuration()) / 60) << " ore e " 
                                      << (static_cast<int>(fc->getCycleDuration()) % 60) << " minuti\n";
                        }  
                    } else {
                        std::cout << "[" << timeManager.getCurrentTimeStamp() << "] Il dispositivo '"
                                << device->getName() << "' si è acceso\n";
                    }
                }
            } else {
                device->toggle();
                std::cout << "[" << timeManager.getCurrentTimeStamp() << "] Il dispositivo '"
                        << device->getName() << "' si è spento\n";
                
                auto& activeDevices = consumptionManager.getActiveDevices();
                activeDevices.erase(device->getId());
                
                auto& deviceActivationOrder = consumptionManager.getDeviceActivationOrder();
                auto it = std::find(deviceActivationOrder.begin(), deviceActivationOrder.end(), device);
                if (it != deviceActivationOrder.end()) {
                    deviceActivationOrder.erase(it);
                }
                consumptionManager.updateDeviceConsumption(timeManager.getCurrentTime(), timeManager.getLastUpdateTime());
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

void DeviceManager::setTime(const std::string& time) {
    int hours, minutes;
    char colon;
    std::istringstream iss(time);
    
    if (iss >> hours >> colon >> minutes && colon == ':' && 
        hours >= 0 && hours < 24 && minutes >= 0 && minutes < 60) {
        
        std::cout << "[" << timeManager.getCurrentTimeStamp() << "] L'orario attuale è " 
                  << timeManager.formatSpecificTime(timeManager.getCurrentTime()) << std::endl;

        int newTime = hours * 60 + minutes;
        if (newTime < timeManager.getCurrentTime()) {
            std::cout << "[Error] Non è possibile impostare un orario precedente a quello attuale.\n";
            return;
        }

        if (newTime != timeManager.getCurrentTime()) {
            for (int t = timeManager.getCurrentTime(); t <= newTime; ++t) {
                consumptionManager.updateDeviceConsumption(t, timeManager.getLastUpdateTime());
                timeManager.setLastUpdateTime(t);
                
                for (const auto& device : devices) {
                    device->update(t);
                    
                    if (auto* fc = dynamic_cast<FCDevice*>(device.get())) {
                        if (device->getStatus()) {
                            int startTime = fc->getStartTime();
                            if (startTime >= 0) {
                                int elapsedMinutes = t - startTime;
                                if (elapsedMinutes >= fc->getCycleDuration() && device->getStatus()) {
                                    device->toggle();
                                    auto& activeDevices = consumptionManager.getActiveDevices();
                                    activeDevices.erase(device->getId());
                                    
                                    auto& deviceActivationOrder = consumptionManager.getDeviceActivationOrder();
                                    auto it = std::find(deviceActivationOrder.begin(), deviceActivationOrder.end(), device);
                                    if (it != deviceActivationOrder.end()) {
                                        deviceActivationOrder.erase(it);
                                    }
                                    
                                    int cycleDuration = fc->getCycleDuration();
                                    std::cout << "[" << timeManager.formatSpecificTime(t) << "] Il dispositivo '"
                                            << device->getName() << "' ha completato il suo ciclo di "
                                            << (cycleDuration / 60) << " ore e " << (cycleDuration % 60)
                                            << " minuti ed è stato spento automaticamente\n";
                                    fc->setStartTime(-1);
                                }
                            }
                        }
                    }
                }
            }
            
            consumptionManager.clearActivationData();
            for (const auto& device : devices) {
                if (device->getStatus()) {
                    auto& activeDevices = consumptionManager.getActiveDevices();
                    activeDevices[device->getId()] = device;
                    consumptionManager.getDeviceActivationOrder().push_back(device);
                }
            }
            consumptionManager.checkPowerConsumption(timeManager.getCurrentTimeStamp());
        }

        timeManager.setTime(time);
        std::cout << "[" << timeManager.getCurrentTimeStamp() << "] Ora impostata: " << time << std::endl;
    } else {
        std::cout << "[Error] Formato tempo non valido. Usa HH:MM" << std::endl;
    }
}

void DeviceManager::showConsumption(const std::string& deviceName) {
    consumptionManager.showConsumption(deviceName, timeManager.getCurrentTimeStamp());
}

void DeviceManager::resetTime() {
    timeManager.resetTime();
    consumptionManager.clearActivationData();
    consumptionManager.initializeDeviceConsumption();
    std::cout << "[00:00] Ora reimpostata a 00:00" << std::endl;
}

void DeviceManager::resetTimers() {
    for (const auto& device : devices) {
        device->clearTimer();
    }
    std::cout << "[" << timeManager.getCurrentTimeStamp() << "] Timer cancellati" << std::endl;
}

void DeviceManager::resetAll() {
    timeManager.resetTime();
    consumptionManager.clearActivationData();
    consumptionManager.initializeDeviceConsumption();
    for (const auto& device : devices) {
        device->clearTimer();
        if (device->getStatus()) {
            device->toggle();
        }
    }
    std::cout << "[00:00] Sistema reimpostato" << std::endl;
}

void DeviceManager::removeDeviceTimer(const std::string& deviceName) {
    auto device = findDevice(deviceName);
    if (device) {
        device->clearTimer();
        std::cout << "[" << timeManager.getCurrentTimeStamp() << "] Timer rimosso per il dispositivo '" 
                  << deviceName << "'" << std::endl;
    } else {
        std::cout << "[Error] Dispositivo non trovato: " << deviceName << std::endl;
        std::cout << "Dispositivi disponibili:" << std::endl;
        for (const auto& d : devices) {
            std::cout << "- " << d->getName() << std::endl;
        }
    }
}

std::string DeviceManager::formatTime() const {
    return timeManager.formatTime();
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

std::string DeviceManager::toLowercase(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return lower;
}