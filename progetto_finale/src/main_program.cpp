#include "../include/Device.h"
#include "../include/DeviceManager.h"
#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <algorithm>

int main() {
    DeviceManager manager(3.5);

    manager.addDevice(std::make_shared<ManualDevice>("Photovoltaic System", 1, -1.5));
    manager.addDevice(std::make_shared<FCDevice>("Washing Machine", 2, -2.0, 1.8333));
    manager.addDevice(std::make_shared<FCDevice>("Dishwasher", 3, -1.5, 3.25));
    manager.addDevice(std::make_shared<ManualDevice>("Heat Pump", 4, -2.0));
    manager.addDevice(std::make_shared<ManualDevice>("Water Heater", 5, -1.0));
    manager.addDevice(std::make_shared<ManualDevice>("Fridge", 6, -0.4));
    manager.addDevice(std::make_shared<FCDevice>("Microwave", 7, -0.8, 0.0333));

    std::string command;
    while (true) {
        std::cout << ">> ";
        std::getline(std::cin, command);
        if (command == "exit") break;

        std::istringstream iss(command);
        std::string action;
        iss >> action;

        if (action == "set") {
            std::string rest;
            std::getline(iss, rest);
            std::istringstream restStream(rest);
            
            std::string firstWord;
            restStream >> firstWord;

            // Prima controlla se è un comando "set time"
            if (firstWord == "time") {
                std::string time;
                restStream >> time;
                if (!time.empty()) {
                    manager.setTime(time);
                } else {
                    std::cout << "[Error] Formato tempo non valido. Usa HH:MM\n";
                }
                continue;
            }

            // Se non è "time", procedi con la logica dei dispositivi
            std::string name = firstWord;
            std::string state;
            std::string word;

            // Accumula il resto del nome del dispositivo
            while (restStream >> word) {
                if (word == "on" || word == "off") {
                    state = word;
                    break;
                }
                name += " " + word;
            }

            // Trasforma il nome in minuscolo per confronto (case insensitive)
            std::string nameLower = name;
            std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

            // Trova il dispositivo originale corrispondente
            std::string originalName;
            for (const auto& device : {"Photovoltaic System", "Washing Machine", "Dishwasher", 
                                     "Heat Pump", "Water Heater", "Fridge", "Microwave"}) {
                std::string lowerDevice = device;
                std::transform(lowerDevice.begin(), lowerDevice.end(), lowerDevice.begin(), ::tolower);
                if (lowerDevice == nameLower) {
                    originalName = device;
                    break;
                }
            }

            if (originalName.empty()) {
                std::cout << "[Error] Dispositivo non trovato: " << name << "\n";
                continue;
            }

            if (state == "on") {
                // Gestione accensione dispositivo
                std::string at;
                restStream >> at;
                if (at == "at") {
                    std::string time;
                    restStream >> time;
                    int hours, minutes;
                    char sep;
                    std::istringstream timeStream(time);
                    if (timeStream >> hours >> sep >> minutes) {
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
        } else if (action == "show") {
            manager.printConsumption();
        } else {
            std::cout << "[Error] Comando non riconosciuto: " << action << "\n";
        }
    }

    return 0;
}
