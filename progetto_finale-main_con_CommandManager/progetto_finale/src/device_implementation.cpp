#include "../include/Device.h"

//alterna lo stato di accensione del dispositivo manuale
void ManualDevice::toggle() {
    isOn = !isOn;
}

//calcola il consumo energetico del dispositivo in base alle ore in cui è stato acceso, restituisce 0.0 se il dispositivo è spento
double ManualDevice::calculateConsumption(double hours) const {
    return isOn ? (powerConsumption * hours) : 0.0;
}
//aggiorna lo stato del dispositivo manuale in base al timer impostato, accende o spegne il dispositivo automaticamente quando l'orario attuale corrisponde all'intervallo di tempo specificato nel timer
void ManualDevice::update(int currentTime) {
    if (hasTimerSet) {
        if (currentTime >= scheduledStartTime && (scheduledStopTime == -1 || currentTime < scheduledStopTime)) {
            if (!isOn) {
                isOn = true;
                std::cout << "[" << formatTime(currentTime) << "] Il dispositivo '"
                          << name << "' si \u00e8 acceso automaticamente\n";
            }
        } else {
            if (isOn) {
                isOn = false;
                std::cout << "[" << formatTime(currentTime) << "] Il dispositivo '"
                          << name << "' si \u00e8 spento automaticamente\n";
            }
        }
    }
}

//stampa lo stato attuale del dispositivo manuale e mostra eventuali timer impostati
void ManualDevice::printStatus() const {
    std::cout << "Device: " << name << " (Manual) - Status: "
              << (isOn ? "ON" : "OFF");
    if (hasTimerSet) {
        std::cout << " - Timer: " << formatTime(scheduledStartTime);
        if (scheduledStopTime != -1) {
            std::cout << " - " << formatTime(scheduledStopTime);
        }
    }
    std::cout << "\n";
}

//imposta un timer per accensione e spegnimento del dispositivo manuale
void ManualDevice::setTimer(int start, int stop) {
    hasTimerSet = true;
    scheduledStartTime = start;
    scheduledStopTime = stop;
}

//rimuove il timer impostato per il dispositivo manuale
void ManualDevice::clearTimer() {
    hasTimerSet = false;
    scheduledStartTime = -1;
    scheduledStopTime = -1;
}

//restituisce l'orario a cui si deve accendere il dispositivo manuale
int ManualDevice::getScheduledStartTime() const {
    return scheduledStartTime;
}

//restituisce l'orario a cui si deve spegnere il dispositivo manuale
int ManualDevice::getScheduledStopTime() const {
    return scheduledStopTime;
}

//alterna lo stato di accensione del dispositivo a spegnimento automatico(FC), se viene acceso senza timer resetta il tempo di avvio
void FCDevice::toggle() {
    isOn = !isOn;
    if (isOn && !hasTimerSet) {
        startTime = -1;
    }
}
//calcola il consumo energetico del dispositivo a spegnimento automatico in base alla durata del ciclo e al numero di ore, se spento restituisce 0.0
double FCDevice::calculateConsumption(double hours) const {
    return isOn ? (powerConsumption * std::min(hours, cycleDuration / 60.0)) : 0.0;
}

//accende automaticamente il dispositivo quando l'orario corrente corrisponde al timer e lo spegne automaticamente alla fine del suo ciclo
void FCDevice::update(int currentTime) {
    if (!isOn && hasTimerSet && currentTime >= scheduledStartTime) {
        isOn = true;
        startTime = currentTime;
        std::cout << "[" << formatTime(currentTime) << "] Il dispositivo '"
                  << name << "' si \u00e8 acceso automaticamente\n";
    }
    else if (isOn) {
        if (startTime >= 0 && currentTime - startTime >= cycleDuration) {
            isOn = false;
            std::cout << "[" << formatTime(currentTime) << "] Il dispositivo '"
                      << name << "' si \u00e8 spento dopo il ciclo prefissato\n";
            clearTimer();
        }
    }
}

//imposta l'orario di inizio del ciclo per il dispositivo FC
void FCDevice::setStartTime(int time) {
    startTime = time;
}
//mostra lo stato attuale del dispositivo FC, indicando se è acceso o spento, e mostra l'eventuale timer impostato
void FCDevice::printStatus() const {
    std::cout << "Device: " << name << " (Fixed Cycle) - Status: "
              << (isOn ? "ON" : "OFF");
    if (hasTimerSet) {
        std::cout << " - Timer: " << formatTime(scheduledStartTime);
    }
    std::cout << "\n";
}

//imposta timer accensione dispositivo FC, ignora il tempo di spegnimento perchè determinato dalla durata del suo ciclo
void FCDevice::setTimer(int start, int stop) {
    (void)stop; // Ignora il tempo di spegnimento
    hasTimerSet = true;
    scheduledStartTime = start;
    startTime = -1;
}

//rimuove il timer 
void FCDevice::clearTimer() {
    hasTimerSet = false;
    scheduledStartTime = -1;
    startTime = -1;
}

//restituisce l'orario di accensione pianificato per dispositivo FC
int FCDevice::getScheduledStartTime() const {
    return scheduledStartTime;
}

