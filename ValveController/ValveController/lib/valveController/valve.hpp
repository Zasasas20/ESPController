#ifndef VALVE_CONTROLLER_H
#define VALVE_CONTROLLER_H

#include <Adafruit_MCP23X08.h> // Use Adafruit library for the I/O expander
#include "esp_timer.h" // Used for timeouts

class ValveController{
public:
    ValveController(Adafruit_MCP23X08 * exp, int valveID);

    int setTimeout(int time);
    bool isAvailable();
    bool activateValve();
    bool toggleValve(bool enable);
    bool isActive();
    int getId();
    Adafruit_MCP23X08* getExpander();

private:

    static void onTimerEnd(void* arg);
    Adafruit_MCP23X08* expander {nullptr};
    esp_timer_handle_t timer;
    int timeout;
    int id;
    bool active {true};
};

#endif //VALVE_CONTROLLER_H
