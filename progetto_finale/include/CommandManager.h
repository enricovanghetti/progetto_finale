#ifndef COMMAND_MANAGER_H
#define COMMAND_MANAGER_H

#include "DeviceManager.h"
#include <string>
#include <sstream>
#include <functional>
#include <map>
#include <memory>
#include <algorithm>

//classe CommandManager per gestire i comandi utente per il sistema domotico casalingo
class CommandManager {
private:
    DeviceManager& deviceManager;
    std::map<std::string, std::function<void(std::istringstream&)>> commands; //mappa di comandi e relative funzioni

    //funzione per analizzare un orario in formato HH:MM
    static bool parseTime(const std::string& timeStr, int& hours, int& minutes) {
        std::istringstream iss(timeStr);
        char sep;
        return (iss >> hours >> sep >> minutes) && sep == ':' && 
               hours >= 0 && hours <= 23 && minutes >= 0 && minutes <= 59;
    }

    //funzione per verificare se una stringa è in formato HH:MM
    static bool isTimeFormat(const std::string& str) {
        int h, m;
        return parseTime(str, h, m);
    }

    //funzione per convertire una stringa in minuscolo (rende i device case-insensitive)
    static std::string toLower(const std::string& str) {
        std::string lower = str;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        return lower;
    }

    //gestisce il comadìndo 'set'
    void handleSet(std::istringstream& iss) {
        std::string secondWord;
        iss >> secondWord;
        // se non si scrive niente dopo 'set' da errore
        if (secondWord.empty()) {
            std::cout << "[Error] Comando 'set' richiede un dispositivo o 'time'. Dispositivi disponibili:\n";
            for (const auto& device : deviceManager.getDevices()) {
                std::cout << "- " << device->getName() << "\n";
            }
            return;
        }
        //entra nel cso in cui si tratti di un 'set time'
        if (secondWord == "time") {
            std::string timeStr;
            iss >> timeStr;
            if (!timeStr.empty()) {
                int hours, minutes;
                if (parseTime(timeStr, hours, minutes)) {
                    deviceManager.setTime(timeStr);
                } else {
                    std::cout << "[Error] Formato tempo non valido. Usa HH:MM\n";
                }
            }
            return;
        }
        //salva in stringa il resto
        std::string restOfLine;
        std::getline(iss, restOfLine);
        std::string fullDeviceName = secondWord;
        std::istringstream restStream(restOfLine);
        std::string word;
        std::vector<std::string> timeValues;
        bool isTimerCommand = false;
        bool hasValidCommand = false;

        //entra nel caso in cui si tratti di 'set <deviceName> on/off'
        while (restStream >> word) {
            if (word == "on" || word == "off") {
                hasValidCommand = true;
                
                std::string startTime, stopTime;
                restStream >> startTime;
                if (!startTime.empty()) {
                    restStream >> stopTime;
                    if (!stopTime.empty()) {
                        int startHours, startMinutes, stopHours, stopMinutes;
                        if (parseTime(startTime, startHours, startMinutes) && 
                            parseTime(stopTime, stopHours, stopMinutes)) {
                            auto device = deviceManager.findDevice(fullDeviceName);
                            if (device) {
                                int startTimeMinutes = startHours * 60 + startMinutes;
                                int stopTimeMinutes = stopHours * 60 + stopMinutes;
                                device->setTimer(startTimeMinutes, stopTimeMinutes);
                                std::cout << "[" << deviceManager.formatTime() << "] Impostato un timer per il dispositivo '"
                                        << fullDeviceName << "' dalle " << startTime << " alle " << stopTime << "\n";
                                isTimerCommand = true;
                            }
                        }
                    }
                }
                //non è un orario l'ultima parte della stringa --> accendi/spegni
                if (!isTimerCommand) {
                    deviceManager.toggleDevice(fullDeviceName);
                }
                break;
            }
            //l'ultima parte della stringa è un orario --> verrà gestita come <orario accnesione> <orario spegnimento> 
            else if (isTimeFormat(word)) {
                timeValues.push_back(word);
            } else {
                if (!fullDeviceName.empty()) fullDeviceName += " ";
                fullDeviceName += word;
            }
        }
        if (!hasValidCommand) {
            handleDeviceTimer(fullDeviceName, timeValues);
        }
    }

    //gestisce il timer per un dispositivo specificato
    void handleDeviceTimer(const std::string& deviceName, const std::vector<std::string>& timeValues) {
        auto device = deviceManager.findDevice(deviceName);
        //genera un errore quando il nome del dispositivo viene scritto male (non corrisponde ad alcun dispositivo presente)
        if (!device) {
            std::cout << "[Error] Dispositivo non trovato: " << deviceName << "\n";
            std::cout << "Dispositivi disponibili:\n";
            for (const auto& d : deviceManager.getDevices()) {
                std::cout << "- " << d->getName() << "\n";
            }
            return;
        }
        //non è presente un orario? allora accende/spegne quel dispositivo all'ora attuale
        if (!timeValues.empty()) {
            int startHours, startMinutes;
            parseTime(timeValues[0], startHours, startMinutes);
            int startTime = startHours * 60 + startMinutes;
            //ci sono 2 orari? allora il primo è l'orario di accensione del dispositvo e il secondo di spegnimento del dispositivo
            if (timeValues.size() > 1) {
                int stopHours, stopMinutes;
                parseTime(timeValues[1], stopHours, stopMinutes);
                int stopTime = stopHours * 60 + stopMinutes;
                device->setTimer(startTime, stopTime);
                std::cout << "[" << deviceManager.formatTime() << "] Impostato un timer per il dispositivo '"
                        << deviceName << "' dalle " << timeValues[0] << " alle " << timeValues[1] << "\n";
            } 
            //un solo orario? allora sarà lora a cui il dispostivo dovrà essere acceso
            else {
                device->setTimer(startTime);
                std::cout << "[" << deviceManager.formatTime() << "] Impostato un timer per il dispositivo '"
                        << deviceName << "' alle " << timeValues[0] << "\n";
            }
        } 
        //messagio di erorre
        else {
            std::cout << "[Error] Comando non valido per il dispositivo: " << deviceName << "\n";
            std::cout << "Usa 'on', 'off' o specifica un timer (HH:MM [HH:MM])\n";
        }
    }

    //gestisce il comando 'show' per mostrare i consumi
    void handleShow(std::istringstream& iss) {
        std::string deviceName;
        std::getline(iss, deviceName);
        //tutti i dispositivi
        if (deviceName.empty()) {
            deviceManager.showConsumption();
        } 
        //solo il dispositivo specificato
        else {
            deviceName = deviceName.substr(deviceName.find_first_not_of(" \t"));
            if (!deviceName.empty()) {
                deviceManager.showConsumption(deviceName);
            }
        }
    }

    //gestisce il comando 'reset' per ripristinare il timer, orario o lo stato del sistema
    void handleReset(std::istringstream& iss) {
        std::string resetType;
        iss >> resetType;
        if (resetType == "time") {
            deviceManager.resetTime();
        } else if (resetType == "timers") {
            deviceManager.resetTimers();
        } else if (resetType == "all") {
            deviceManager.resetAll();
        } else {
            std::cout << "[Error] Comando reset non valido. Usa 'time', 'timers' o 'all'\n";
        }
    }

    //gestisce il comando 'rm' per rimuovere un il timer da un dispositivo
    void handleRemove(std::istringstream& iss) {
        std::string deviceName;
        std::getline(iss, deviceName);
        if (!deviceName.empty()) {
            deviceName = deviceName.substr(deviceName.find_first_not_of(" \t"));
            deviceManager.removeDeviceTimer(deviceName);
        } else {
            std::cout << "[Error] Specificare il nome del dispositivo\n";
            std::cout << "Uso: rm <nome_dispositivo>\n";
        }
    }

public:
    //costruttore per inizializzare CommandManager con il DeviceManager associato
    explicit CommandManager(DeviceManager& manager) : deviceManager(manager) {
        commands["set"] = [this](std::istringstream& iss) { handleSet(iss); };
        commands["show"] = [this](std::istringstream& iss) { handleShow(iss); };
        commands["reset"] = [this](std::istringstream& iss) { handleReset(iss); };
        commands["rm"] = [this](std::istringstream& iss) { handleRemove(iss); };
    }

    //esegue un comando scritto dall'utente
    bool executeCommand(const std::string& commandLine) {
        if (commandLine == "exit") return false;

        std::istringstream iss(commandLine);
        std::string action;
        iss >> action;

        auto it = commands.find(action);
        if (it != commands.end()) {
            it->second(iss);
        } else {
            std::cout << "[Error] Comando non riconosciuto: " << action << "\n";
        }

        return true;
    }
};

#endif