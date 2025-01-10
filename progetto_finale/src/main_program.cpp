#include "../include/Device.h"
#include "../include/DeviceManager.h"
#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <algorithm>

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
    manager.addDevice(std::make_shared<ManualDevice>("Photovoltaic System", 1, -1.5));
    manager.addDevice(std::make_shared<FCDevice>("Washing Machine", 2, 2.0, 1.8333));
    manager.addDevice(std::make_shared<FCDevice>("Dishwasher", 3, 1.5, 3.25));
    manager.addDevice(std::make_shared<ManualDevice>("Heat Pump", 4, 2.0));
    manager.addDevice(std::make_shared<ManualDevice>("Water Heater", 5, 1.0));
    manager.addDevice(std::make_shared<ManualDevice>("Fridge", 6, 0.4));
    manager.addDevice(std::make_shared<FCDevice>("Microwave", 7, 0.8, 0.0333));

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

            // Raccolta delle parti rimanenti del comando
            std::string rest;
            std::getline(iss, rest);
            std::istringstream restStream(rest);
            
            std::string name = secondWord;
            std::string nextWord;
            std::vector<std::string> timeValues;

            while (restStream >> nextWord) {
                if (nextWord == "on" || nextWord == "off") {
                    // Gestione vecchio formato comando
                    if (nextWord == "on") {
                        std::string at;
                        std::string timeStr;
                        restStream >> at >> timeStr;
                        if (at == "at" && !timeStr.empty()) {
                            int hours, minutes;
                            if (parseTime(timeStr, hours, minutes)) {
                                manager.toggleDevice(name, hours * 60 + minutes);
                            } else {
                                std::cout << "[Error] Formato tempo non valido per 'at'. Usa HH:MM\n";
                            }
                        } else {
                            manager.toggleDevice(name);
                        }
                    } else {
                        manager.toggleDevice(name);
                    }
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

            // Se abbiamo trovato dei valori tempo, trattiamoli come nuovo formato comando
            if (!timeValues.empty()) {
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
                } else {
                    // Solo orario di start
                    device->setTimer(startTime);
                }

                std::cout << "[" << manager.formatTime() << "] Impostato timer per '" << name 
                         << "': accensione alle " << timeValues[0];
                if (timeValues.size() > 1) {
                    std::cout << ", spegnimento alle " << timeValues[1];
                }
                std::cout << "\n";
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
