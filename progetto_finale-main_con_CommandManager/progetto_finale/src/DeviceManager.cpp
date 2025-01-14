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
//controlla se c'è abbastanza spazio rimanente in termini di potenza prima di aggiungere un nuovo device allo stack dei dispositvi accesi
bool DeviceManager::checkPowerForDevice(const std::shared_ptr<Device>& deviceToAdd) {
    if (isCheckingPower) return true;
    isCheckingPower = true;
    //somma il consumo istantaneo del dispositvo che si vuole accendere a quello dei dispositivi già accesi (già inseriti nello stack)
    double totalConsumption = deviceToAdd->powerConsumption;
    for (const auto& pair : activeDevices) {
        if (pair.second != deviceToAdd) {
            totalConsumption += pair.second->getPowerConsumption();
        }
    }
    //se la somma da un risultato maggiore rispetto al massimo consentito (3.5kW) inizia a rimuovere con logica LIFO i dispositivi già accesi per "fargli spazio"
    if (totalConsumption > maxPowerLimit) {
        std::cout << "[" << getCurrentTimeStamp() << "] Warning: potenza massima superata (" 
                  << std::fixed << std::setprecision(1) << totalConsumption << "kW)" << std::endl;
        
        //spegne dispositivi più recenti finché non c'è spazio sufficiente
        auto it = deviceActivationOrder.end();
        while (totalConsumption > maxPowerLimit && it != deviceActivationOrder.begin()) {
            --it;//dal più recente al meno recente (LIFO)
            auto device = *it;
            //si assicura che non vengano spenti il dispositivo che si ha intenzione di accendere e l'impianto fotovoltaico
            if (device != deviceToAdd && device->getName() != "Impianto fotovoltaico") {
                device->toggle();
                std::cout << "[" << getCurrentTimeStamp() << "] Il dispositivo '" 
                          << device->getName() << "' è stato spento automaticamente" << std::endl;
                activeDevices.erase(device->getId());
                it = deviceActivationOrder.erase(it);
                //ricalcola il consumo totale
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
//gestisce l'accensione e lo spegnimento dei vari dispositivi
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
                    //gestisce i dispitivi a spegnimento automatico (FCDevice)
                    if (auto* fc = dynamic_cast<FCDevice*>(device.get())) {
                        if (fc->getCycleDuration() > 0) {
                            fc->setStartTime(currentTime);
                            std::cout << "[" << getCurrentTimeStamp() << "] Il dispositivo '"
                                      << device->getName() << "' si è acceso e si spegnerà automaticamente dopo "
                                      << (static_cast<int>(fc->getCycleDuration()) / 60) << " ore e " << (static_cast<int>(fc->getCycleDuration()) % 60) << " minuti\n";

                        } 
                    //gestisce i dispositivi a spegnimento manuale
                    } else {
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
    //stampa l'erroe di dispositvo non trovato e stampa tutti i dospitivi disponibili
    std::cout << "[Error] Dispositivo non trovato: " << deviceName << "\n";
    std::cout << "Dispositivi disponibili:\n";
    for (const auto& device : devices) {
        std::cout << "- " << device->getName() << "\n";
    }
}
//controlla l'attuale consumo istantaneo di energia
void DeviceManager::checkPowerConsumption() {
    if (isCheckingPower) return;
    isCheckingPower = true;
    double totalConsumption = 0.0;
    for (const auto& pair : activeDevices) {
        totalConsumption += pair.second->getPowerConsumption();
    }
    //messaggio di errore in caso di potenza massima superata
    if (totalConsumption > maxPowerLimit) {
        std::cout << "[" << getCurrentTimeStamp() << "] Warning: potenza massima superata (" 
                  << std::fixed << std::setprecision(1) << totalConsumption << "kW)" << std::endl;
    }
    isCheckingPower = false;
}

//gestisce l'ora
void DeviceManager::setTime(const std::string& time) {
    int hours, minutes;
    char colon;
    std::istringstream iss(time);
    //si assciura che l'orario sia scirtto in un formato accettabile dal programma
    if (iss >> hours >> colon >> minutes && colon == ':' && hours >= 0 && hours < 24 && minutes >= 0 && minutes < 60) {
        std::cout << "[" << getCurrentTimeStamp() << "] L'orario attuale è " << formatSpecificTime(currentTime) << std::endl;
        int newTime = hours * 60 + minutes;
        //messaggio di errore in caso di tentativo di impostare l'ora ad un orario minore rispetto a quello attuale
        if (newTime < currentTime) {
            std::cout << "[Error] Non è possibile impostare un orario precedente a quello attuale.\n";
            return;
        }
        //se l'ora è maggiore di quella attuale (non può essere minore per controllo precedente e non può essere uguale per controllo in questa if)
        if (newTime != currentTime) {
            for (int t = currentTime; t <= newTime; ++t) {
                updateDeviceConsumption(t);
                for (const auto& device : devices) {
                    //gestisce dispositivi manuali con timer
                    device->update(t);
                    //gestisce i dispositivi automatici
                    if (auto* fc = dynamic_cast<FCDevice*>(device.get())) {
                        if (device->getStatus()) {
                            int startTime = fc->getStartTime();
                            if (startTime >= 0) {
                                int elapsedMinutes = t - startTime;
                                if (elapsedMinutes >= fc->getCycleDuration() && device->getStatus()) {
                                    device->toggle();
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
                                    fc->setStartTime(-1);
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
//mostra il consumo energetico
void DeviceManager::showConsumption(const std::string& deviceName) const {
    //di tutti i dispositivi
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
    } 
    //del dispostivo di cui si è specificato il nome
    else {
        printDeviceConsumption(deviceName);
    }
}
//stampa il consumo del singolo device
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
//rm, rimuove il timer di un dispositvo specificato
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
//reset dell'ora, la riporta alle 00:00
void DeviceManager::resetTime() {
    currentTime = 0;
    lastUpdateTime = 0;
    deviceActivationOrder.clear();
    activeDevices.clear();
    initializeDeviceConsumption();
    std::cout << "[00:00] Ora reimpostata a 00:00" << std::endl;
}
//rimuove i timer da tutti i dispostivi
void DeviceManager::resetTimers() {
    for (const auto& device : devices) {
        device->clearTimer();
    }
    std::cout << "[" << getCurrentTimeStamp() << "] Timer cancellati" << std::endl;
}
//riporta tutto allo stato inizale --> tutti i dispositvi spenti e senza timer, ora 00:00, reset dei consumi
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
//coverte i minuti in ore:minuti
std::string DeviceManager::formatSpecificTime(int minutes) const {
    int hours = minutes / 60;
    int mins = minutes % 60;
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << hours << ":" << std::setfill('0') << std::setw(2) << mins;
    return oss.str();
}
//aggiorna il consumo dei dispotivi
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
//inizializza il conusmo di tutti i dispositvi
void DeviceManager::initializeDeviceConsumption() {
    deviceEnergyConsumption.clear();
    for (const auto& device : devices) {
        deviceEnergyConsumption[device->getId()] = 0.0;
    }
}
