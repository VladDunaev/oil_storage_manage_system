#include "storage_tank.h"
#include "oil_storage_def.h"
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

/**
 * Функция котроля уровня нефтпродуктов в резервуаре
 * @param st_ptr указатель на резервуар
 * @return NULL
 */
static void* _control_level(void* st_ptr);

/**
 * резервуар для хранения нефтепродуктов
 */
struct _storage_tank{
    /**
     * минимальный уровень нефтепродуктов в резервуаре
     */
    unsigned int minimum_level;
    /**
     * максимальный уровень нефтепродуктов в резервуаре
     */
    unsigned int maximum_level;
    /**
     * уровень нефтепродуктов в резервуаре
     */
    int current_level;
    /**
     * состояния работы резервуара (STORAGE_TANK_ON - влючен, STORAGE_TANK_OFF - выключен)
     */
    int state;
    /**
     * насос для закачивания нефтепродуктов
     */
    pump* injection_pump;
    /**
     * насос для откачивания нефтепродуктов
     */
    pump* pumping_pump;
    /**
     *  поток, для контроля уровня нефтепродутов в резервуаре
     */
    pthread_t control_thread;
};


storage_tank* create_storage_tank(unsigned int min_level, unsigned int max_level, unsigned int speed_injection_pump, unsigned int speed_pumping_pump){
    storage_tank* st = malloc(sizeof(storage_tank));
    st->minimum_level = min_level;
    st->maximum_level = max_level;
    st->current_level = min_level;
    st->state = STORAGE_TANK_OFF;
    st->injection_pump = create_pump(&st->current_level, speed_injection_pump);
    st->pumping_pump = create_pump(&st->current_level, -speed_pumping_pump);
    return st;
}

void turn_on_storage_tank(storage_tank *st){
    if (st->state == STORAGE_TANK_OFF){
        st->state = STORAGE_TANK_ON;
        pthread_create(&st->control_thread, NULL, _control_level, st);
    }
}

void turn_off_storage_tank(storage_tank *st){
    if (st->state == STORAGE_TANK_ON){
        st->state = STORAGE_TANK_OFF;
        turn_off_pump(st->injection_pump);
        turn_off_pump(st->pumping_pump);
        pthread_join(st->control_thread, NULL);
    }
}

int get_state_storage_tank(const storage_tank *st){
    return st->state;
}

void set_minimum_level_storage_tank(storage_tank* st, unsigned int min_level){
    st->minimum_level = min_level;
}

unsigned int get_minimum_level_storage_tank(const storage_tank* st){
    return st->minimum_level;
}

void set_maximum_level_storage_tank(storage_tank* st, unsigned int max_level){
    st->maximum_level = max_level;
}

unsigned int get_maximum_level_storage_tank(const storage_tank* st){
    return st->maximum_level;
}

unsigned int get_current_level_storage_tank(storage_tank *st){
    if (st->current_level < 0) {
        st->current_level = 0;
    }
    return st->current_level;
}

void finalize_storage_tank(storage_tank* st){
    turn_off_storage_tank(st);
    finalize_pump(st->injection_pump);
    finalize_pump(st->pumping_pump);
    free(st);
}

void turn_on_injection_pump(storage_tank* st){
    if (st->state == STORAGE_TANK_OFF){
        turn_on_storage_tank(st);
    }
    if (st->current_level < st->maximum_level){
        turn_on_pump(st->injection_pump);
    }
}

void turn_off_injection_pump(storage_tank* st){
    turn_off_pump(st->injection_pump);
}

int get_state_injection_pump(const storage_tank* st){
    return get_state_pump(st->injection_pump);
}

void set_speed_injection_pump(storage_tank* st, unsigned int speed){
    set_delta_pump(st->injection_pump, speed);
}

unsigned int get_speed_injection_pump(const storage_tank* st){
    get_delta_pump(st->injection_pump);
}

void turn_on_pumping_pump(storage_tank* st){
    if (st->state == STORAGE_TANK_OFF){
        turn_on_storage_tank(st);
    }
    if (st->current_level > st->minimum_level){
        turn_on_pump(st->pumping_pump);
    }
}

void turn_off_pumping_pump(storage_tank* st){
    turn_off_pump(st->pumping_pump);
}

int get_state_pumping_pump(const storage_tank* st){
    get_state_pump(st->pumping_pump);
}

void set_speed_pumping_pump(storage_tank* st, unsigned int speed){
    set_delta_pump(st->pumping_pump, -speed);
}

unsigned int get_speed_pumping_pump(const storage_tank* st){
    return -get_delta_pump(st->pumping_pump);
}

static void* _control_level(void* st_ptr){
    storage_tank* st = st_ptr;
    while(st->state == STORAGE_TANK_ON){
        if (st->current_level <= st->minimum_level){
            turn_off_pump(st->pumping_pump);
        }
        if (st->current_level < 0){
            st->current_level = 0;
        }
        if (st->current_level >= st->maximum_level){
            turn_off_pump(st->injection_pump);
        }
        usleep(TIME_UNIT);
    }
    return NULL;
}