#include "valve.hpp"

#define defaultTimeout 3000000

ValveController::ValveController(Adafruit_MCP23X08 * exp, int valveID):
    expander{exp},
    id{valveID},
    timeout{defaultTimeout}
{
    const esp_timer_create_args_t timer_args = {
        .callback = &onTimerEnd,
        .arg = (void *) this,
        .name = "valve-timer"
    };

    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &timer));
}
    
int ValveController::setTimeout(int time){
    timeout = time;

    return timeout;
}

bool ValveController::isActive(){
    return active;
}

bool ValveController::isAvailable(){
    return !esp_timer_is_active(timer);
}

bool ValveController::activateValve(){
    bool activated = false;
    if (isAvailable()) {
        ESP_ERROR_CHECK(esp_timer_start_once(timer, timeout));
        
        //uint8_t current = expander->readGPIO();
        //expander->writeGPIO(current || (1 << id));
        
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
    ValveController* controller = (ValveController *) arg;

    uint8_t current = controller->getExpander()->readGPIO();
    controller->getExpander()->writeGPIO(current || (0 << controller->getId()));
}