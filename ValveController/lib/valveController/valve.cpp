#include "valve.hpp"

#define defaultTimeout 3000000

ValveController::ValveController(Adafruit_MCP23X08 * exp, int valveID):
    expander{exp},
    id{valveID},
    timeout{defaultTimeout}
{

    args = new V_Args;
    args->exp = exp;
    args->id = valveID;
    const esp_timer_create_args_t timer_args = {
        .callback = &onTimerEnd,
        .arg = (void *) args,
        .name = "valve-timer"
    };

    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &timer));
}
    
void ValveController::setTimeout(uint64_t time){
    timeout = time;
}

bool ValveController::isActive(){
    return active;
}

bool ValveController::isAvailable(){
    return !esp_timer_is_active(timer);
}

bool ValveController::activateValve(){
    bool activated = false;
    if (!isActive()) return true;
    if (isAvailable()) {
        ESP_ERROR_CHECK(esp_timer_start_once(timer, timeout));
        
        uint8_t current = expander->readGPIO();
        expander->writeGPIO(current | (1 << id - 1));
        
        activated = true;
    }
    return activated;
}

bool ValveController::toggleValve(bool enable){
    active = enable;
    return active;
}

Adafruit_MCP23X08* ValveController::getExpander() {
    return expander;
}

int ValveController::getId() {
    return id;
}
    
void ValveController::onTimerEnd(void* arg){
    V_Args* args = (V_Args *) arg;

    uint8_t current = args->exp->readGPIO();
    args->exp->writeGPIO(current & ~(1 << args->id - 1));
}