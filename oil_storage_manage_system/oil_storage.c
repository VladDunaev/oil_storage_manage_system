#include "oil_storage.h"
#include "storage_tank.h"
#include "msg_int.h"
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

#ifndef __OPERATION_NUMBER
    #define __OPERATION_NUMBER
    #define CREATE_STORAGE_TANK     0
    #define TURN_ON_STORAGE_TANK    1
    #define TURN_OFF_STORAGE_TANK   2
    #define GET_STATE_TANK          3
    #define SET_MINIMUM_LEVEL_TANK  4
    #define GET_MINIMUM_LEVEL_TANK  5
    #define SET_MAXIMUM_LEVEL_TANK  6
    #define GET_MAXIMUM_LEVEL_TANK  7
    #define GET_CURRENT_LEVEL_TANK  8
    #define TURN_ON_DOWNLOAD_PUMP   9
    #define TURN_OFF_DOWNLOAD_PUMP  10
    #define GET_STATE_DOWNLOAD_PUMP 11
    #define SET_SPEED_DOWNLOAD_PUMP 12
    #define GET_SPEED_DOWNLOAD_PUMP 13
    #define TURN_ON_UPLOAD_PUMP     14
    #define TURN_OFF_UPLOAD_PUMP    15
    #define GET_STATE_UPLOAD_PUMP   16
    #define SET_SPEED_UPLOAD_PUMP   17
    #define GET_SPEED_UPLOAD_PUMP   18
    #define FINALIZE_STORAGE_TANK   -1
#endif

/**
 * создать процессы для каждого резевуара
 * @param os указатель на нефтрехранилище
 */
static void _create_process_for_tanks(oil_storage* os);

/**
 * функция управления резервуаром
 * @param m_in очередь сообщений для чтения команд
 * @param m_out чередь сообщений для ответа на команды
 * @param type тип сообщеий, который посылает резевуар
 */
static void _manage_storage_tank(msg_int* m_in, msg_int* m_out, long type);

/**
 * записывает номер команды в указанный файл
 * @param m_in очередь сообщений для записи
 * @param type тип сообщения
 * @param operation_number номер команды
 */
static void _send_operation_number(msg_int* m_out, long type, int operation_number);

/**
 * Хранилище нефти
 */
struct _oil_storage{
    /**
     * количество резевуаров
     */
    size_t tanks_count;
    /**
     * идентификаторы процессов, в которых осуществляется управление резервуаром
     */
    pid_t* pids;
    /**
     * очередь для передачи информации резевуарам
     */
    msg_int* m_out;
    /**
     * очередь для получения информации от резервуаров
     */
    msg_int* m_in;
};

oil_storage* create_oil_storage(size_t storage_tanks_count, unsigned int min_level, unsigned int max_level, unsigned int speed_download_pump, unsigned int speed_upload_pump, const char* path){
    oil_storage* os = malloc(sizeof(oil_storage));
    os->tanks_count = storage_tanks_count;
    os->pids = malloc(sizeof(pid_t)*os->tanks_count);
    os->m_in = create_msg_int_handler(path);
    os->m_out = create_msg_int_handler(path);
    _create_process_for_tanks(os);
    for(int i = 0; i < os->tanks_count; ++i){
        _send_operation_number(os->m_out, i + 1, CREATE_STORAGE_TANK);
        write_int(os->m_out, i + 1, min_level);
        write_int(os->m_out, i + 1, max_level);
        write_int(os->m_out, i + 1, speed_download_pump);
        write_int(os->m_out, i + 1, speed_upload_pump);
    }
    return os;
}

void turn_on_tank(oil_storage* os, unsigned int number){
    _send_operation_number(os->m_out, number + 1, TURN_ON_STORAGE_TANK);
}

void turn_off_tank(oil_storage* os, unsigned int number){
    _send_operation_number(os->m_out, number + 1, TURN_OFF_STORAGE_TANK);
}

int get_state_tank(const oil_storage* os, unsigned int number){
    _send_operation_number(os->m_out, number + 1,  GET_STATE_TANK);
    int state_st = read_int(os->m_in, number + 1);
    return state_st;
}

void set_minimum_level_tank(oil_storage* os, unsigned int number, unsigned int min_level){
    _send_operation_number(os->m_out, number + 1, SET_MINIMUM_LEVEL_TANK);
    write_int(os->m_out, number + 1, min_level);
}

unsigned int get_minimum_level_tank(const oil_storage* os, unsigned int number){
    _send_operation_number(os->m_out, number + 1,  GET_MINIMUM_LEVEL_TANK);
    unsigned int min_level = read_int(os->m_in, number + 1);
    return min_level;
}

void set_maximum_level_tank(oil_storage* os, unsigned int number, unsigned int max_level){
    _send_operation_number(os->m_out, number + 1, SET_MAXIMUM_LEVEL_TANK);
    write_int(os->m_out, number + 1, max_level);
}

unsigned int get_maximum_level_tank(const oil_storage* os, unsigned int number){
    _send_operation_number(os->m_out, number + 1, GET_MAXIMUM_LEVEL_TANK);
    unsigned int max_level = read_int(os->m_in, number + 1);
    return max_level;
}

unsigned int get_current_level_tank(const oil_storage* os, unsigned int number){
    _send_operation_number(os->m_out, number + 1, GET_CURRENT_LEVEL_TANK);
    unsigned int cur_level = read_int(os->m_in, number + 1);
    return cur_level;
}

void finalize_oil_storage(oil_storage* os){
    for(int i = 0; i < os->tanks_count; ++i){
        _send_operation_number(os->m_out, i + 1, FINALIZE_STORAGE_TANK);
    }
    for(int i = 0; i < os->tanks_count; ++i){
        waitpid(os->pids[i], NULL, 0);
    }
    finalize_msg_int_handler(os->m_in);
    finalize_msg_int_handler(os->m_out);
    free(os->pids);
    free(os);
}

void turn_on_download_pump(oil_storage* os, unsigned int number){
    _send_operation_number(os->m_out, number + 1, TURN_ON_DOWNLOAD_PUMP);
}

void turn_off_download_pump(oil_storage* os, unsigned int number){
    _send_operation_number(os->m_out, number + 1,TURN_OFF_DOWNLOAD_PUMP);
}

int get_state_download_pump(const oil_storage* os, unsigned int number){
    _send_operation_number(os->m_out, number + 1, GET_STATE_DOWNLOAD_PUMP);
    int state_dp = read_int(os->m_in, number + 1);
    return state_dp;
}

void set_speed_download_pump(oil_storage *os, unsigned int number, unsigned int download_speed) {
    _send_operation_number(os->m_out, number + 1, SET_SPEED_DOWNLOAD_PUMP);
    write_int(os->m_out, number + 1, download_speed);
}

unsigned int get_speed_download_pump(const oil_storage* os, unsigned int number){
    _send_operation_number(os->m_out, number + 1, GET_SPEED_DOWNLOAD_PUMP);
    unsigned int download_speed = read_int(os->m_in, number + 1);
    return download_speed;
}

void turn_on_upload_pump(oil_storage* os, unsigned int number){
    _send_operation_number(os->m_out, number + 1, TURN_ON_UPLOAD_PUMP);
}

void turn_off_upload_pump(oil_storage* os, unsigned int number){
    _send_operation_number(os->m_out, number + 1, TURN_OFF_UPLOAD_PUMP);
}

int get_state_upload_pump(const oil_storage* os, unsigned int number){
    _send_operation_number(os->m_out, number + 1, GET_STATE_UPLOAD_PUMP);
    int state_up = read_int(os->m_in, number + 1);
    return state_up;
}

void set_speed_upload_pump(oil_storage *os, unsigned int number, unsigned int upload_speed) {
    _send_operation_number(os->m_out, number + 1, SET_SPEED_UPLOAD_PUMP);
    write_int(os->m_out, number + 1, upload_speed);
}

unsigned int get_speed_upload_pump(const oil_storage* os, unsigned int number){
    _send_operation_number(os->m_out, number + 1, GET_SPEED_UPLOAD_PUMP);
    unsigned int upload_speed = read_int(os->m_in, number + 1);
    return upload_speed;
}

size_t get_count_tanks(const oil_storage *os){
    return os->tanks_count;
}

static void _create_process_for_tanks(oil_storage* os){
    for(int i = 0; i < os->tanks_count; ++i){
        os->pids[i] = fork();
        if (os->pids[i] == 0){
            _manage_storage_tank(os->m_out, os->m_in, (long)(i) + 1);
            _exit(0);
        }
    }
}

static void _manage_storage_tank(msg_int* m_in, msg_int* m_out, long type){
    storage_tank* st = NULL;
    for(;;){
        int operation_number = read_int(m_in, type);
        switch (operation_number){
            case CREATE_STORAGE_TANK:{
                unsigned int params[4];
                params[0] = (unsigned int)read_int(m_in, type);
                params[1] = (unsigned int)read_int(m_in, type);
                params[2] = (unsigned int)read_int(m_in, type);
                params[3] = (unsigned int)read_int(m_in, type);
                st = create_storage_tank(params[0], params[1], params[2], params[3]);
                break;
            }
            case TURN_ON_STORAGE_TANK:{
                turn_on_storage_tank(st);
                break;
            }
            case TURN_OFF_STORAGE_TANK:{
                turn_off_storage_tank(st);
                break;
            }
            case GET_STATE_TANK:{
                int state_st = get_state_storage_tank(st);
                write_int(m_out, type, state_st);
                break;
            }
            case SET_MINIMUM_LEVEL_TANK:{
                unsigned int min_level = (unsigned int)read_int(m_in, type);
                set_minimum_level_storage_tank(st, min_level);
                break;
            }
            case GET_MINIMUM_LEVEL_TANK:{
                unsigned int min_level = get_minimum_level_storage_tank(st);
                write_int(m_out, type, (int)min_level);
                break;
            }
            case SET_MAXIMUM_LEVEL_TANK:{
                unsigned int max_level = (unsigned int)read_int(m_in, type);
                set_maximum_level_storage_tank(st, max_level);
                break;
            }
            case GET_MAXIMUM_LEVEL_TANK:{
                unsigned int max_level = get_maximum_level_storage_tank(st);
                write_int(m_out, type, (int)max_level);
                break;
            }
            case GET_CURRENT_LEVEL_TANK:{
                unsigned int cur_level = get_current_level_storage_tank(st);
                write_int(m_out, type, (int)cur_level);
                break;
            }
            case TURN_ON_DOWNLOAD_PUMP:{
                turn_on_injection_pump(st);
                break;
            }
            case TURN_OFF_DOWNLOAD_PUMP:{
                turn_off_injection_pump(st);
                break;
            }
            case GET_STATE_DOWNLOAD_PUMP:{
                int state_dp = get_state_injection_pump(st);
                write_int(m_out, type, (int)state_dp);
                break;
            }
            case SET_SPEED_DOWNLOAD_PUMP:{
                unsigned int speed_dp = (unsigned int)read_int(m_in, type);
                set_speed_injection_pump(st, speed_dp);
                break;
            }
            case GET_SPEED_DOWNLOAD_PUMP:{
                unsigned int speed_dp = get_speed_injection_pump(st);
                write_int(m_out, type, (int)speed_dp);
                break;
            }
            case TURN_ON_UPLOAD_PUMP:{
                turn_on_pumping_pump(st);
                break;
            }
            case TURN_OFF_UPLOAD_PUMP:{
                turn_off_pumping_pump(st);
                break;
            }
            case GET_STATE_UPLOAD_PUMP:{
                int state_up = get_state_pumping_pump(st);
                write_int(m_out, type, (int)state_up);
                break;
            }
            case SET_SPEED_UPLOAD_PUMP:{
                unsigned int speed_pp = read_int(m_in, type);
                set_speed_pumping_pump(st, speed_pp);
                break;
            }
            case GET_SPEED_UPLOAD_PUMP:{
                unsigned int speed_pp = get_speed_pumping_pump(st);
                write_int(m_out, type, (int)speed_pp);
                break;
            }
            case FINALIZE_STORAGE_TANK:{
                finalize_storage_tank(st);
                return;
            }
            default:{
                continue;
            }
        }
    }
}

static void _send_operation_number(msg_int* m_out, long type, int operation_number){
    write_int(m_out, type, operation_number);
}