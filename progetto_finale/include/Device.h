#ifndef DEVICE_H
#define DEVICE_H

#include <string>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>

std::string formatTime(int minutes);

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

    bool getStatus() const { return isOn; }
    const std::string& getName() const { return name; }
    int getId() const { return id; }
    double getPowerConsumption() const { return isOn ? powerConsumption : 0.0; }
};

class ManualDevice : public Device {
public:
    ManualDevice(std::string name, int id, double powerConsumption)
        : Device(std::move(name), id, powerConsumption) {}

    void toggle() override;
    double calculateConsumption(double hours) const override;
    void update(int currentTime) override;
    void printStatus() const override;
};

class FCDevice : public Device {
private:
    double cycleDuration;
    int startTime;
    bool hasTimerSet;
    int scheduledStartTime;
    int scheduledEndTime;

public:
    FCDevice(std::string name, int id, double powerConsumption, double cycleDuration)
        : Device(std::move(name), id, powerConsumption), 
          cycleDuration(cycleDuration * 60),
          startTime(-1),
          hasTimerSet(false),
          scheduledStartTime(-1),
          scheduledEndTime(-1) {}

    void toggle() override;
    double calculateConsumption(double hours) const override;
    void update(int currentTime) override;
    void setStartTime(int time);
    void printStatus() const override;
    
    bool hasTimer() const { return hasTimerSet; }
    void clearTimer();
    void setTimer(int start, int end);
    double getCycleDuration() const { return cycleDuration; }
    int getScheduledStartTime() const { return scheduledStartTime; }
    int getScheduledEndTime() const { return scheduledEndTime; }
};

#endif
