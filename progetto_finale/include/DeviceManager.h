#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <vector>
#include <map>
#include <memory>
#include <string>
#include <algorithm>
#include "Device.h"

//classe per la gestione dei dispositivi e del consumo energetico
class DeviceManager {
    std::vector<std::shared_ptr<Device>> devices; //elenco dispositivi che vengono gestiti
    std::map<int, std::shared_ptr<Device>> activeDevices;//mappa dei dispositivi attivi con il loro ID
    std::map<int, double> deviceEnergyConsumption; //consumo energetico di ogni dispositivo
    std::vector<std::shared_ptr<Device>> deviceActivationOrder; //ordine di attivazione dei dispositivi
    double maxPowerLimit; //limite massimo di potenza consentita
    int currentTime; //orario corrente simulato
    int lastUpdateTime; //ultimo orario aggiornato
    bool isCheckingPower; //check per prevenire controlli ricorsivi di potenza

public:
    //costruttore
    explicit DeviceManager(double maxPowerLimit);
    //aggiunge un nuovo dispositivo al sistema
    void addDevice(const std::shared_ptr<Device>& device);
    //alterna stato di un dispositivo (acceso/spento) e imposta un timer (opzionale)
    void toggleDevice(const std::string& deviceName, int startAt = -1);

    //controllo se il consumo di potenza totale supera il limite
    void checkPowerConsumption();

    //imposta l'orario attuale e aggiorna lo stato dei dispositivi
    void setTime(const std::string& time);

    //mostra il consumo energetico di un dispositivo specifico o di tutti
    void showConsumption(const std::string& deviceName = "") const;

    //reset orario corrente a 00:00
    void resetTime();

    //rimuove tutti i timer impostati sui dispositivi
    void resetTimers();

    //resetta completamente il sistema, spegnendo tutti i dispositivi e cancellando i timer, riporta anche l'ra a 00:00
    void resetAll();
//rimuove un timer da un dispositivo specifico
    void removeDeviceTimer(const std::string& deviceName);

    //restituisce l'orario attuale formattato in HH:MM
    std::string formatTime() const;

    //trova un dispositivo per nome(case-insensitive)
    std::shared_ptr<Device> findDevice(const std::string& deviceName) const;

    //restituisce l'elenco di tutti i dispositivi gestiti
    const std::vector<std::shared_ptr<Device>>& getDevices() const { return devices; }

private:
    //restituisce orario attuale formattato
    std::string getCurrentTimeStamp() const;

    //formatta un orario specificato in minuti in HH:MM
    std::string formatSpecificTime(int minutes) const;

    //aggiorna il consumo energetico dei dispositivi in base al nuovo orario
    void updateDeviceConsumption(int newTime);
    //inizializza i dati di consumo energetico per tutti i dispositivi
    void initializeDeviceConsumption();
    //stampa il consumo energetico di un dispositivo specifico
    void printDeviceConsumption(const std::string& deviceName) const;
    //verifica se l'aggiunta di un dispositivo supera il limite massimo di potenza
    bool checkPowerForDevice(const std::shared_ptr<Device>& deviceToAdd);
    
    //converte una stringa in minuscolo (case-insensitive)
    static std::string toLowercase(const std::string& str) {
        std::string lower = str;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        return lower;
    }
};

#endif

