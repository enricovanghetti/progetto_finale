#ifndef DEVICE_H
#define DEVICE_H

#include <string>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>

//formattara il tempo in formato HH:MM
inline std::string formatTime(int minutes) {
    int hours = minutes / 60;
    int mins = minutes % 60;
    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << hours << ":"
        << std::setw(2) << std::setfill('0') << mins;
    return oss.str();
}

class Device {
protected:
    std::string name;      //nome device
    int id;          //ID dispositivo
    double powerConsumption;    //consumo energetico del dispositivo
    bool isOn;                //stato dispositivo (acceso/spento)

public:
    //costruttore
    Device(std::string name, int id, double powerConsumption)
        : name(std::move(name)), id(id), powerConsumption(powerConsumption), isOn(false) {}

    virtual ~Device() {}
    virtual void toggle() = 0;
    virtual double calculateConsumption(double hours) const = 0;
    virtual void update(int currentTime) = 0;
    virtual void printStatus() const = 0;
    virtual void setTimer(int start, int stop = -1) = 0;
    virtual void clearTimer() = 0;
    virtual bool hasTimer() const = 0;
    virtual int getScheduledStartTime() const { return -1; }
    virtual int getScheduledStopTime() const { return -1; }
    //restituisce true se acceso
    bool getStatus() const { return isOn; }

    //restituisce il nome del dispositivo
    const std::string& getName() const { return name; }

    //restituisce l'ID del dispositivo
    int getId() const { return id; }

    //restituisce il consumo energetico del dispositivo se acceso, altrimenti 0.0
    double getPowerConsumption() const { return isOn ? powerConsumption : 0.0; }

    friend class DeviceManager;
};

class ManualDevice : public Device {
private:
    bool hasTimerSet;         //verifica se un timer è impostato
    int scheduledStartTime;    //orario di accensione pianificato
    int scheduledStopTime;    //orario di spegnimento pianificato

public:
    //costruttore device a spegnimento manuale
    ManualDevice(std::string name, int id, double powerConsumption)
        : Device(std::move(name), id, powerConsumption),
          hasTimerSet(false),
          scheduledStartTime(-1),
          scheduledStopTime(-1) {}

    //alternara lo stato del dispositivo
    void toggle() override;
    //metodo per calcolare il consumo energetico
    double calculateConsumption(double hours) const override;
    //aggiorna lo stato del dispositivo in base al timer
    void update(int currentTime) override;

    //stampa lo stato del dispositivo
    void printStatus() const override;

    //imposta un timer per il dispositivo
    void setTimer(int start, int stop = -1) override;

    //rimuove il timer impostato
    void clearTimer() override;

    //verifica se un timer è impostato
    bool hasTimer() const override { return hasTimerSet; }
    //restituisce l'orario di accensione pianificato
    int getScheduledStartTime() const override;

    //restituisce l'orario di spegnimento pianificato
    int getScheduledStopTime() const override;
};

//classe per dispositivi a ciclo spegnimento automatico (a ciclo prefissato)
class FCDevice : public Device {
private:
    double cycleDuration;       //durata del ciclo in minuti
    int startTime;           //orario di inizio del ciclo
    bool hasTimerSet;         // verifica se un timer è impostato
    int scheduledStartTime;    //orario di accensione pianificato

public:
    //costruttore device a spegnimento automatico
    FCDevice(std::string name, int id, double powerConsumption, double cycleDuration)
        : Device(std::move(name), id, powerConsumption), 
          cycleDuration(cycleDuration),
          startTime(-1),
          hasTimerSet(false),
          scheduledStartTime(-1) {}

    //alterna lo stato del dispositivo FC
    void toggle() override;
    //calcola il consumo energetico in base alla durata del ciclo
    double calculateConsumption(double hours) const override;

    //aggiorna lo stato del dispositivo in base al timer
    void update(int currentTime) override;

    //imposta l'orario di inizio del ciclo
    void setStartTime(int time);

    //stampa lo stato del dispositivo
    void printStatus() const override;

    //imposta un timer per il dispositivo
    void setTimer(int start, int stop = -1) override;

    //rimuove il timer impostato
    void clearTimer() override;
    //verifica se un timer è impostato
    bool hasTimer() const override { return hasTimerSet; }

    //restituisce la durata del ciclo
    double getCycleDuration() const { return cycleDuration; }
    //imposta la durata del ciclo
    void setCycleDuration(int minutes) { cycleDuration = minutes; }
    //restituisce l'orario di inizio del ciclo
    int getStartTime() const { return startTime; }
    //restituisce l'orario di accensione pianificato
    int getScheduledStartTime() const override;
};

#endif
