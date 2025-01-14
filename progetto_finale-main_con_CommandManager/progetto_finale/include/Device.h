#ifndef DEVICE_H
#define DEVICE_H

#include <string>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>

// Inline implementation of formatTime
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
    std::string name;
    int id;
    double powerConsumption;
    bool isOn;

public:
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

    bool getStatus() const { return isOn; }
    const std::string& getName() const { return name; }
    int getId() const { return id; }
    double getPowerConsumption() const { return isOn ? powerConsumption : 0.0; }

    friend class DeviceManager;
};

class ManualDevice : public Device {
private:
    bool hasTimerSet;
    int scheduledStartTime;
    int scheduledStopTime;

public:
    ManualDevice(std::string name, int id, double powerConsumption)
        : Device(std::move(name), id, powerConsumption),
          hasTimerSet(false),
          scheduledStartTime(-1),
          scheduledStopTime(-1) {}

    void toggle() override;
    double calculateConsumption(double hours) const override;
    void update(int currentTime) override;
    void printStatus() const override;
    void setTimer(int start, int stop = -1) override;
    void clearTimer() override;
    bool hasTimer() const override { return hasTimerSet; }
    int getScheduledStartTime() const override;
    int getScheduledStopTime() const override;
};

class FCDevice : public Device {
private:
    double cycleDuration;  // in minuti
    int startTime;
    bool hasTimerSet;
    int scheduledStartTime;

public:
    FCDevice(std::string name, int id, double powerConsumption, double cycleDuration)
        : Device(std::move(name), id, powerConsumption), 
          cycleDuration(cycleDuration),
          startTime(-1),
          hasTimerSet(false),
          scheduledStartTime(-1) {}

    void toggle() override;
    double calculateConsumption(double hours) const override;
    void update(int currentTime) override;
    void setStartTime(int time);
    void printStatus() const override;
    void setTimer(int start, int stop = -1) override;
    void clearTimer() override;
    bool hasTimer() const override { return hasTimerSet; }
    
    // Nuovi metodi
    double getCycleDuration() const { return cycleDuration; }
    void setCycleDuration(int minutes) { cycleDuration = minutes; }
    int getStartTime() const { return startTime; }
    
    int getScheduledStartTime() const override;
};

#endif