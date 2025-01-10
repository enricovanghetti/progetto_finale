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

// Helper function per verificare il formato del tempo
bool isTimeFormat(const std::string& str) {
    int h, m;
    return parseTime(str, h, m);
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
            std::string fullDeviceName;
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

            // Leggi il resto della riga per il nome del dispositivo e il comando
            std::string restOfLine;
            std::getline(iss, restOfLine);
            fullDeviceName = secondWord;

            // Estrai il nome del dispositivo e il comando (on/off)
            std::istringstream restStream(restOfLine);
            std::string word;
            std::vector<std::string> timeValues;
            bool isTimerCommand = false;
            std::string command;

            while (restStream >> word) {
                if (word == "on" || word == "off") {
                    command = word;
                    
                    // Leggi i possibili orari dopo on/off
                    std::string startTime, stopTime;
                    restStream >> startTime;
                    if (!startTime.empty()) {
                        restStream >> stopTime;
                        if (!stopTime.empty()) {
                            int startHours, startMinutes, stopHours, stopMinutes;
                            if (parseTime(startTime, startHours, startMinutes) && 
                                parseTime(stopTime, stopHours, stopMinutes)) {
                                auto device = manager.findDevice(fullDeviceName);
                                if (device) {
                                    int startTimeMinutes = startHours * 60 + startMinutes;
                                    int stopTimeMinutes = stopHours * 60 + stopMinutes;
                                    device->setTimer(startTimeMinutes, stopTimeMinutes);
                                    std::cout << "[" << manager.formatTime() << "] Impostato un timer per il dispositivo '"
                                            << fullDeviceName << "' dalle " << startTime << " alle " << stopTime << "\n";
                                    isTimerCommand = true;
                                }
                            }
                        }
                    }
                    
                    if (!isTimerCommand) {
                        manager.toggleDevice(fullDeviceName);
                        manager.checkPowerConsumption();
                    }
                    break;
                } else if (isTimeFormat(word)) {
                    timeValues.push_back(word);
                } else {
                    if (!fullDeviceName.empty()) fullDeviceName += " ";
                    fullDeviceName += word;
                }
            }

            // Se non è stato trovato un comando on/off, controlla se ci sono orari per il timer
            if (command.empty() && !timeValues.empty() && !isTimerCommand) {
                auto device = manager.findDevice(fullDeviceName);
                if (!device) {
                    std::cout << "[Error] Dispositivo non trovato: " << fullDeviceName << "\n";
                    continue;
                }

                int startHours, startMinutes;
                parseTime(timeValues[0], startHours, startMinutes);
                int startTime = startHours * 60 + startMinutes;

                if (timeValues.size() > 1) {
                    int stopHours, stopMinutes;
                    parseTime(timeValues[1], stopHours, stopMinutes);
                    int stopTime = stopHours * 60 + stopMinutes;
                    device->setTimer(startTime, stopTime);
                    std::cout << "[" << manager.formatTime() << "] Impostato un timer per il dispositivo '"
                             << fullDeviceName << "' dalle " << timeValues[0] << " alle " << timeValues[1] << "\n";
                } else {
                    device->setTimer(startTime);
                    std::cout << "[" << manager.formatTime() << "] Impostato un timer per il dispositivo '"
                             << fullDeviceName << "' alle " << timeValues[0] << "\n";
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
