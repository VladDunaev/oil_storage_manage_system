#include "pump.h"
#include "oil_storage_def.h"
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * Функция, в которой осуществляется работа насоса
 * @param p_ptr указатель на насос
 * @return NULL;
 */
static void* _pump_work(void *p_ptr);

/**
 * насос для перекачки нефтeпродуктов
 */
struct _pump{
    /**
     * текущее значение нефти (величина, которая будет изменяться)
     */
    int* value;
    /**
     * скорость перекачки (величина, на которую будет изменять уровень нефти)
     */
    int delta;
    /**
     * состояние работы насоса(PUMP_ON - включен, PUMP_OFF выключен)
     */
    int state;
    /**
     * поток, в котором осуществляется работа насоса
     */
    pthread_t work_thread;
};

pump* create_pump(int* value, int delta_per_unit_time){
    pump* p = malloc(sizeof(pump));
    if (p != NULL){
        p->value = value;
        p->delta = delta_per_unit_time;
        p->state = PUMP_OFF;
    }
    return p;
}

void turn_on_pump(pump* p){
    if (p->state == PUMP_OFF){
        p->state = PUMP_ON;
        pthread_create(&p->work_thread, NULL, _pump_work, p);
    }
}

void turn_off_pump(pump* p){
    if (p->state == PUMP_ON){
        p->state = PUMP_OFF;
        pthread_join(p->work_thread, NULL);
    }
}

int get_state_pump(const pump* p){
    return p->state;
}

void set_delta_pump(pump* p, int delta_per_unit_time){
    p->delta = delta_per_unit_time;
}

int get_delta_pump(const pump* p){
    return p->delta;
}

void finalize_pump(pump* p){
    turn_off_pump(p);
    free(p);
}

void *_pump_work(void *p_ptr) {
    pump* p = p_ptr;
    while(p->state == PUMP_ON){
        *p->value += p->delta;
        usleep(TIME_UNIT*1000);
    }
    return NULL;
}