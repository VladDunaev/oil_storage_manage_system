#ifndef OIL_STORAGE_MANAGE_SYSTEM_STORAGE_TANK_H
#define OIL_STORAGE_MANAGE_SYSTEM_STORAGE_TANK_H

#include "pump.h"

/**
 * резервуар для хранения нефтепродуктов
 */
struct _storage_tank;
typedef struct _storage_tank storage_tank;

/**
 * Создать резервуар
 * @param min_level минимально допустиммый уровень нефти
 * @param max_level максимально допустимый уровень нефти
 * @param speed_injection_pump скорость закачки нефти
 * @param speed_pumping_pump скорость откачки нефти
 * @return указатель на созданный резервуар
 */
storage_tank* create_storage_tank(unsigned int min_level, unsigned int max_level, unsigned int speed_injection_pump, unsigned int speed_pumping_pump);

/**
 * переключить резервуар в рабочее состояние
 * @param st указатель на резервуар
 */
void turn_on_storage_tank(storage_tank *st);

/**
 * переключить резервуар в нерабочее состояние
 * @param st указатель на резервуар
 */
void turn_off_storage_tank(storage_tank *st);

/**
 * получить состояние работы резервуара
 * @param st указатель на резервуар
 * @return STORAGE_TANK_OFF - нерабочее состояние, STORAGE_TANK_ON - рабочее состояние
 */
int get_state_storage_tank(const storage_tank *st);

/**
 * установить мимнимальный уровень нефти
 * @param st указатель на резервуар
 * @param min_level минимальный уровень нефти
 */
void set_minimum_level_storage_tank(storage_tank* st, unsigned int min_level);

/**
 * получить минимальный уровень нефти
 * @param st указатель на резервуар
 * @return минимальный уровень нефти
 */
unsigned int get_minimum_level_storage_tank(const storage_tank* st);

/**
 * установить максимальный уровень нефти
 * @param st указатель на резервуар
 * @param min_level максимальный уровень нефти
 */
void set_maximum_level_storage_tank(storage_tank* st, unsigned int max_level);

/**
 * получить максимальный уровень нефти
 * @param st указатель на резервуар
 * @return максимальный уровень нефти
 */
unsigned int get_maximum_level_storage_tank(const storage_tank* st);

/**
 * получить уровень нефти
 * @param st указатель на резервуар
 * @return уровень нефти
 */
unsigned int get_current_level_storage_tank(storage_tank *st);

/**
 * уничтожить резевуар
 * @param st указатель на резервуар
 */
void finalize_storage_tank(storage_tank* st);

/**
 * включить насос закачки нефтепродуктов
 * @param st указатель на резервуар
 */
void turn_on_injection_pump(storage_tank* st);

/**
 * выключить насос закачки нефтепродуктов
 * @param st указатель на резервуар
 */
void turn_off_injection_pump(storage_tank* st);

/**
 * получить состояние работы насоса закачки
 * @param st указатель на резервуар
 * @return PUMP_OFF - нерабочее состояние, PUMP_ON - рабочее состояние
 */
int get_state_injection_pump(const storage_tank* st);

/**
 * установить скорость закачки нефтепродуктов
 * @param st указатель на резервуар
 * @param speed скорость закачки
 */
void set_speed_injection_pump(storage_tank* st, unsigned int speed);

/**
 * получить скорость закачки нефтепродуктов
 * @param st указатель на резервуар
 * @return скорость закачки нефтепродуктов
 */
unsigned int get_speed_injection_pump(const storage_tank* st);

/**
 * включить насос откачки нефтепродуктов
 * @param st указатель на резервуар
 */
void turn_on_pumping_pump(storage_tank* st);

/**
 * выключить насос откачки нефтепродуктов
 * @param st указатель на резервуар
 */
void turn_off_pumping_pump(storage_tank* st);

/**
 * получить состояние работы насоса откачки
 * @param st указатель на резервуар
 * @return PUMP_OFF - нерабочее состояние, PUMP_ON - рабочее состояние
 */
int get_state_pumping_pump(const storage_tank* st);

/**
 * установить скорость откачки нефтепродуктов
 * @param st указатель на резервуар
 * @param speed скорость откачки
 */
void set_speed_pumping_pump(storage_tank* st, unsigned int speed);

/**
 * получить скорость откачки нефтепродуктов
 * @param st указатель на резервуар
 * @return скорость откачки нефтепродуктов
 */
unsigned int get_speed_pumping_pump(const storage_tank* st);

#endif //OIL_STORAGE_MANAGE_SYSTEM_STORAGE_TANK_H