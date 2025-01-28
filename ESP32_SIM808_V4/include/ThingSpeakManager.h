#ifndef THINGSPEAK_MANAGER_H
#define THINGSPEAK_MANAGER_H

class ThingSpeakManager {
public:
    static void sendDataToThingSpeak(int sensorValues[7]);
};

#endif // THINGSPEAK_MANAGER_H
