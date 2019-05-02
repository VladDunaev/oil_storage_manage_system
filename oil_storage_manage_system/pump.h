#ifndef OIL_STORAGE_MANAGE_SYSTEM_PUMP_H
#define OIL_STORAGE_MANAGE_SYSTEM_PUMP_H

#define TIME_UNIT 10;               //время в мс, за которое происходит одно изменение
#define PUMP_ON 1;                  //насос включен
#define PUMP_OFF 0;                 //насос выключен

/**
 * насос для перекачки нефтпродуктов
 */
struct _pump;
typedef struct _pump pump;

/**
 * создать насос
 * @param value текущее значение нефти (величина, которая будет изменяться)
 * @param delta_per_unit_time скорость перекачки (величина, на которую будет изменять уровень нефти)
 * @return указатель на насос
 */
pump* create_pump(const int* value, int delta_per_unit_time);

/**
 * включить насос
 * @param p указатель на насос
 */
void turn_on_pump(pump* p);


/**
 * выключить насос
 * @param p указатель на насос
 */
void turn_off_pump(pump* p);

/**
 * получить состояние работы насоса
 * @param p указатель на насос
 * @return PUMP_OFF - насос выключен, PUMP_ON - насос включен
 */
int get_state_pump(const pump* p);

/**
 * Установить скорость перекачки нефти
 * @param p указатель на насос
 * @param deltainjection_pump_per_unit_time скорость перекачки нефти
 */
void set_delta_pump(pump* p, int delta_per_unit_time);

/**
 * Получить скорость перекакчки нефти
 * @param p указатель на насос
 * @return скорость перекачки нефти
 */
int get_delta_pump(const pump* p);

/**
 * Уничтожить насос
 * @param p указатель на насос
 */
void finalize_pump(pump* p);

#endif //OIL_STORAGE_MANAGE_SYSTEM_PUMP_H
