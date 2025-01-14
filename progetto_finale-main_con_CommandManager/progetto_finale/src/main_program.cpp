#include "../include/Device.h"
#include "../include/DeviceManager.h"
#include "../include/CommandManager.h"
#include <iostream>
#include <memory>
#include <string>

int main() {
    DeviceManager deviceManager(3.5);//potenza massima impostata a 3.5kW (varia all'accensione di 'Impianto fotovoltaico')
    //dispositivi ad accensione e spegnimento manuale
    deviceManager.addDevice(std::make_shared<ManualDevice>("Impianto fotovoltaico", 1, -1.5));
    deviceManager.addDevice(std::make_shared<ManualDevice>("Pompa di calore con termostato", 4, 2.0));
    deviceManager.addDevice(std::make_shared<ManualDevice>("Scaldabagno", 5, 1.0));
    deviceManager.addDevice(std::make_shared<ManualDevice>("Frigorifero", 6, 0.4));
    deviceManager.addDevice(std::make_shared<FCDevice>("Tapparelle elettriche", 10, 0.3, 1));
    //dispositivi a spegnimento automatico
    deviceManager.addDevice(std::make_shared<FCDevice>("Lavatrice", 2, 2.0, 110));
    deviceManager.addDevice(std::make_shared<FCDevice>("Lavastoviglie", 3, 1.5, 195));
    deviceManager.addDevice(std::make_shared<FCDevice>("Forno a microonde", 7, 0.8, 2));
    deviceManager.addDevice(std::make_shared<FCDevice>("Asciugatrice", 9, 1.8, 60));
    deviceManager.addDevice(std::make_shared<FCDevice>("Televisore", 8, 0.2, 60));

    CommandManager commandManager(deviceManager);
    std::string command;
    //chiamata a CommandManager fino al termine della stringa in input
    while (true) {
        std::cout << ">> ";
        std::getline(std::cin, command);
        
        if (!commandManager.executeCommand(command)) {
            break;
        }
    }
    return 0;
}