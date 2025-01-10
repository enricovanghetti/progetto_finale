#include "../include/DeviceManager.h"
#include <iostream>
#include <sstream>
#include <iomanip>

DeviceManager::DeviceManager(double maxPowerLimit) : maxPowerLimit(maxPowerLimit), currentTime(0) {}

void DeviceManager::addDevice(const std::shared_ptr<Device>& device) {
    devices.push_back(device);
    deviceRunningTime[device->getId()] = 0.0;  // Inizializza il tempo di esecuzione
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
            // Aggiorna il tempo di esecuzione per i dispositivi attivi
            deviceRunningTime[pair.first] += 1.0/60.0; // Converte in ore
        }
    }
    std::cout << "[Time] Impostato orario a " << formatTime() << "\n";
}

double DeviceManager::getDeviceRunningTime(int deviceId) const {
    auto it = deviceRunningTime.find(deviceId);
    return (it != deviceRunningTime.end()) ? it->second : 0.0;
}

void DeviceManager::printConsumption() const {
    double totalConsumption = 0.0;
    
    std::cout << "\n=== Report Consumo Energetico (00:00 - " << formatTime() << ") ===\n";
    std::cout << std::fixed << std::setprecision(2);
    
    // Stampa info per ogni dispositivo
    for (const auto& device : devices) {
        double runningHours = getDeviceRunningTime(device->getId());
        double consumption = device->calculateConsumption(runningHours);
        totalConsumption += consumption;
        
        std::cout << device->getName() << ":\n";
        std::cout << "  Stato: " << (device->getStatus() ? "Attivo" : "Inattivo") << "\n";
        std::cout << "  Tempo di funzionamento: " << runningHours << " ore\n";
        std::cout << "  Energia " << (consumption < 0 ? "prodotta: " : "consumata: ")
                  << std::abs(consumption) << " kWh\n";
    }
    
    std::cout << "\nBilancio energetico totale: ";
    if (totalConsumption < 0) {
        std::cout << "Produzione netta di " << -totalConsumption << " kWh\n";
    } else if (totalConsumption > 0) {
        std::cout << "Consumo netto di " << totalConsumption << " kWh\n";
    } else {
        std::cout << "Bilancio neutro (0 kWh)\n";
    }
    std::cout << "==========================================\n\n";
}

std::string DeviceManager::formatTime() const {
    int hours = currentTime / 60;
    int minutes = currentTime % 60;
    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << hours << ":"
        << std::setw(2) << std::setfill('0') << minutes;
    return oss.str();
}
