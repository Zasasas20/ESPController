#ifndef VALVE_CONTROLLER_H
#define VALVE_CONTROLLER_H

#include <Adafruit_MCP23X08.h> // Use Adafruit library for the I/O expander
#include "esp_timer.h" // Used for timeouts

struct V_Args {
    Adafruit_MCP23X08* exp;
    int id;
};

class ValveController{
public:
    ValveController(Adafruit_MCP23X08 * exp, int valveID);

    void setTimeout(uint64_t time);
    bool isAvailable();
    bool activateValve();
    bool toggleValve(bool enable);
    bool isActive();
    int getId();
    Adafruit_MCP23X08* getExpander();

private:

    static void onTimerEnd(void* arg);
    Adafruit_MCP23X08* expander { nullptr };
    V_Args* args { nullptr };
    esp_timer_handle_t timer;
    uint64_t timeout;
    int id;
    bool active {true};
};

#endif //VALVE_CONTROLLER_H
