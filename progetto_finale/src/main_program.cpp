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

    const std::vector<std::string> deviceNames = {
        "Photovoltaic System", "Washing Machine", "Dishwasher",
        "Heat Pump", "Water Heater", "Fridge", "Microwave"
    };

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

            std::string rest;
            std::getline(iss, rest);
            std::istringstream restStream(rest);
            
            std::string name = secondWord;
            std::string state;
            std::string word;

            while (restStream >> word) {
                if (word == "on" || word == "off") {
                    state = word;
                    break;
                }
                if (!name.empty()) name += " ";
                name += word;
            }

            std::string nameLower = toLower(name);
            std::string originalName;
            
            for (const auto& deviceName : deviceNames) {
                if (toLower(deviceName) == nameLower) {
                    originalName = deviceName;
                    break;
                }
            }

            if (originalName.empty()) {
                std::cout << "[Error] Dispositivo non trovato: " << name << "\n";
                continue;
            }

            if (state == "on") {
                std::string at;
                restStream >> at;
                if (at == "at") {
                    std::string timeStr;
                    restStream >> timeStr;
                    int hours, minutes;
                    if (parseTime(timeStr, hours, minutes)) {
                        manager.toggleDevice(originalName, hours * 60 + minutes);
                    } else {
                        std::cout << "[Error] Formato tempo non valido per 'at'. Usa HH:MM\n";
                    }
                } else {
                    manager.toggleDevice(originalName);
                }
                manager.checkPowerConsumption();
            } else if (state == "off") {
                manager.toggleDevice(originalName);
                manager.checkPowerConsumption();
            } else {
                std::cout << "[Error] Stato non valido: usa 'on' o 'off'\n";
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
