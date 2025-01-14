#include "../include/Device.h"
#include "../include/DeviceManager.h"
#include "../include/CommandManager.h"
#include <iostream>
#include <fstream>
#include <memory>
#include <string>

int main() {
    DeviceManager deviceManager(3.5); // Potenza massima impostata a 3.5kW

    // Dispositivi ad accensione e spegnimento manuale
    deviceManager.addDevice(std::make_shared<ManualDevice>("Impianto fotovoltaico", 1, -1.5));
    deviceManager.addDevice(std::make_shared<ManualDevice>("Pompa di calore con termostato", 4, 2.0));
    deviceManager.addDevice(std::make_shared<ManualDevice>("Scaldabagno", 5, 1.0));
    deviceManager.addDevice(std::make_shared<ManualDevice>("Frigorifero", 6, 0.4));
    deviceManager.addDevice(std::make_shared<FCDevice>("Tapparelle elettriche", 10, 0.3, 1));

    // Dispositivi a spegnimento automatico
    deviceManager.addDevice(std::make_shared<FCDevice>("Lavatrice", 2, 2.0, 110));
    deviceManager.addDevice(std::make_shared<FCDevice>("Lavastoviglie", 3, 1.5, 195));
    deviceManager.addDevice(std::make_shared<FCDevice>("Forno a microonde", 7, 0.8, 2));
    deviceManager.addDevice(std::make_shared<FCDevice>("Asciugatrice", 9, 1.8, 60));
    deviceManager.addDevice(std::make_shared<FCDevice>("Televisore", 8, 0.2, 60));

    CommandManager commandManager(deviceManager);

    // Apri il file di log
    std::ofstream logFile("log.txt", std::ios::app);
    if (!logFile) {
        std::cerr << "[Error] Impossibile aprire il file di log.\n";
        return 1;
    }

    std::string command;

    // Loop principale per gestire i comandi
    while (true) {
        // Mostra il prompt sul terminale
        std::cout << ">> ";
        std::getline(std::cin, command);

        // Scrivi l'input dell'utente nel file di log
        logFile << "Input: " << command << std::endl;

        // Esegui il comando
        if (!commandManager.executeCommand(command)) {
            logFile << "Programma terminato." << std::endl;
            break;
        }

        // Assicura che il log sia scritto immediatamente
        logFile.flush();
    }

    // Chiudi il file di log
    logFile.close();

    return 0;
}
