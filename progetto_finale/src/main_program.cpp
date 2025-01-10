#include "../include/Device.h"
#include "../include/DeviceManager.h"
#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>

// Helper function per convertire una stringa in minuscolo
std::string toLower(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return lower;
}

// Helper function per parsare il tempo
bool parseTime(const std::string& timeStr, int& hours, int& minutes) {
    std::istringstream iss(timeStr);
    char sep;
    if (!(iss >> hours >> sep >> minutes) || sep != ':' || 
        hours < 0 || hours > 23 || minutes < 0 || minutes > 59) {
        return false;
    }
    return true;
}

int main() {
    DeviceManager manager(3.5);

    // Inizializzazione dei dispositivi
    manager.addDevice(std::make_shared<ManualDevice>("photovoltaic system", 1, -1.5));
    manager.addDevice(std::make_shared<FCDevice>("washing machine", 2, 2.0, 1.8333));
    manager.addDevice(std::make_shared<FCDevice>("dishwasher", 3, 1.5, 3.25));
    manager.addDevice(std::make_shared<ManualDevice>("heat pump", 4, 2.0));
    manager.addDevice(std::make_shared<ManualDevice>("water heater", 5, 1.0));
    manager.addDevice(std::make_shared<ManualDevice>("fridge", 6, 0.4));
    manager.addDevice(std::make_shared<FCDevice>("microwave", 7, 0.8, 0.0333));

    std::string command;
    while (true) {
        std::cout << ">> ";
        std::getline(std::cin, command);
        if (command == "exit") break;

        std::istringstream iss(command);
        std::string action;
        iss >> action;

        if (action == "set") {
            std::string secondWord;
            iss >> secondWord;

            if (secondWord.empty()) {
                std::cout << "[Error] Comando 'set' richiede un dispositivo o 'time'. Dispositivi disponibili:\n";
                for (const auto& device : manager.getDevices()) {
                    std::cout << "- " << device->getName() << "\n";
                }
                continue;
            }

            // Gestione del comando "set time"
            if (secondWord == "time") {
                std::string timeStr;
                iss >> timeStr;
                if (!timeStr.empty()) {
                    int hours, minutes;
                    if (parseTime(timeStr, hours, minutes)) {
                        manager.setTime(timeStr);
                    } else {
                        std::cout << "[Error] Formato tempo non valido. Usa HH:MM\n";
                    }
                }
                continue;
            }

            // Verifica che il dispositivo esista prima di procedere
            auto device = manager.findDevice(secondWord);
            if (!device) {
                std::cout << "[Error] Dispositivo '" << secondWord << "' non trovato. Dispositivi disponibili:\n";
                for (const auto& d : manager.getDevices()) {
                    std::cout << "- " << d->getName() << "\n";
                }
                continue;
            }

            // Raccolta delle parti rimanenti del comando
            std::string rest;
            std::getline(iss, rest);
            std::istringstream restStream(rest);
            
            std::string name = secondWord;
            std::string nextWord;
            std::vector<std::string> timeValues;
            bool isTimerCommand = false;

            while (restStream >> nextWord) {
                if (nextWord == "on") {
                    // Controllo se ci sono due orari dopo "on"
                    std::string startTime, stopTime;
                    restStream >> startTime;
                    if (!startTime.empty()) {
                        restStream >> stopTime;
                        if (!stopTime.empty()) {
                            // Caso "set Device on HH:MM HH:MM"
                            int startHours, startMinutes, stopHours, stopMinutes;
                            if (parseTime(startTime, startHours, startMinutes) && 
                                parseTime(stopTime, stopHours, stopMinutes)) {
                                int startTimeMinutes = startHours * 60 + startMinutes;
                                int stopTimeMinutes = stopHours * 60 + stopMinutes;
                                device->setTimer(startTimeMinutes, stopTimeMinutes);
                                std::cout << "[" << manager.formatTime() << "] Impostato un timer per il dispositivo '"
                                        << name << "' dalle " << startTime << " alle " << stopTime << "\n";
                                isTimerCommand = true;
                            }
                        }
                    }
                    // Se non è un comando timer, trattalo come comando on immediato
                    if (!isTimerCommand) {
                        manager.toggleDevice(name);
                        manager.checkPowerConsumption();
                    }
                    break;
                } else if (nextWord == "off") {
                    manager.toggleDevice(name);
                    manager.checkPowerConsumption();
                    break;
                }
                
                // Se è un formato tempo HH:MM, salvalo
                int h, m;
                if (parseTime(nextWord, h, m)) {
                    timeValues.push_back(nextWord);
                } else {
                    if (!name.empty()) name += " ";
                    name += nextWord;
                }
            }

            // Se abbiamo trovato dei valori tempo e non è già stato gestito come timer command
            if (!timeValues.empty() && !isTimerCommand) {
                auto device = manager.findDevice(name);
                if (!device) {
                    std::cout << "[Error] Dispositivo non trovato: " << name << "\n";
                    continue;
                }

                int startHours, startMinutes;
                parseTime(timeValues[0], startHours, startMinutes);
                int startTime = startHours * 60 + startMinutes;

                if (timeValues.size() > 1) {
                    // Caso con orario di stop
                    int stopHours, stopMinutes;
                    parseTime(timeValues[1], stopHours, stopMinutes);
                    int stopTime = stopHours * 60 + stopMinutes;
                    device->setTimer(startTime, stopTime);
                    std::cout << "[" << manager.formatTime() << "] Impostato un timer per il dispositivo '"
                             << name << "' dalle " << timeValues[0] << " alle " << timeValues[1] << "\n";
                } else {
                    // Solo orario di start
                    device->setTimer(startTime);
                    std::cout << "[" << manager.formatTime() << "] Impostato un timer per il dispositivo '"
                             << name << "' alle " << timeValues[0] << "\n";
                }
            }
        }
        else if (action == "show") {
            std::string deviceName;
            std::getline(iss, deviceName);
            if (deviceName.empty()) {
                manager.printConsumption();
            } else {
                deviceName = deviceName.substr(deviceName.find_first_not_of(" \t"));
                if (!deviceName.empty()) {
                    manager.printDeviceConsumption(deviceName);
                }
            }
        }
        else if (action == "reset") {
            std::string resetType;
            iss >> resetType;
            if (resetType == "time") {
                manager.resetTime();
            } else if (resetType == "timers") {
                manager.resetTimers();
            } else if (resetType == "all") {
                manager.resetAll();
            } else {
                std::cout << "[Error] Comando reset non valido. Usa 'time', 'timers' o 'all'\n";
            }
        }
        else {
            std::cout << "[Error] Comando non riconosciuto: " << action << "\n";
        }
    }

    return 0;
}
