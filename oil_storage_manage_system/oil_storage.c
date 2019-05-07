#include "oil_storage.h"
#include "storage_tank.h"
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
 * @param fd_in файловый дескриптор канала для чтения команд
 * @param fd_out файловый дескриптор канала для ответа на команды
 */
static void _manage_storage_tank(int fd_in, int fd_out);

/**
 * записывает номер команды в указанный файл
 * @param fd дескриптор файла для записи
 * @param operation_number номер команды
 */
static void _send_operation_number(int fd, int operation_number);

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
     * каналы для передачи информации резевуарам
     */
    int** pipe_fds_in;
    /**
     * каналы для получения информации от резервуаров
     */
    int** pipe_fds_out;
};

oil_storage* create_oil_storage(size_t storage_tanks_count, unsigned int min_level, unsigned int max_level, unsigned int speed_download_pump, unsigned int speed_upload_pump){
    oil_storage* os = malloc(sizeof(oil_storage));
    os->tanks_count = storage_tanks_count;
    os->pids = malloc(sizeof(pid_t)*os->tanks_count);
    os->pipe_fds_in = malloc(sizeof(int*)*os->tanks_count);
    os->pipe_fds_out = malloc(sizeof(int*)*os->tanks_count);
    for(int i = 0; i < os->tanks_count; ++i){
        os->pipe_fds_in[i] = malloc(sizeof(int)*2);
        os->pipe_fds_out[i] = malloc(sizeof(int)*2);
        pipe(os->pipe_fds_in[i]);
        pipe(os->pipe_fds_out[i]);
    }
    _create_process_for_tanks(os);
    for(int i = 0; i < os->tanks_count; ++i){
        _send_operation_number(os->pipe_fds_in[i][1], CREATE_STORAGE_TANK);
        unsigned int params[] = {
                min_level,
                max_level,
                speed_download_pump,
                speed_upload_pump,
        };
        write(os->pipe_fds_in[i][1], params, sizeof(params));
    }
    return os;
}

void turn_on_tank(oil_storage* os, unsigned int number){
    _send_operation_number(os->pipe_fds_in[number][1], TURN_ON_STORAGE_TANK);
}

void turn_off_tank(oil_storage* os, unsigned int number){
    _send_operation_number(os->pipe_fds_in[number][1], TURN_OFF_STORAGE_TANK);
}

int get_state_tank(const oil_storage* os, unsigned int number){
    _send_operation_number(os->pipe_fds_in[number][1], GET_STATE_TANK);
    int state_st;
    read(os->pipe_fds_out[number][0], &state_st, sizeof(state_st));
    return state_st;
}

void set_minimum_level_tank(oil_storage* os, unsigned int number, unsigned int min_level){
    _send_operation_number(os->pipe_fds_in[number][1], SET_MINIMUM_LEVEL_TANK);
    write(os->pipe_fds_in[number][1], &min_level, sizeof(min_level));
}

unsigned int get_minimum_level_tank(const oil_storage* os, unsigned int number){
    _send_operation_number(os->pipe_fds_in[number][1], GET_MINIMUM_LEVEL_TANK);
    unsigned int min_level;
    read(os->pipe_fds_out[number][0], &min_level, sizeof(min_level));
    return min_level;
}

void set_maximum_level_tank(oil_storage* os, unsigned int number, unsigned int max_level){
    _send_operation_number(os->pipe_fds_in[number][1], SET_MAXIMUM_LEVEL_TANK);
    write(os->pipe_fds_in[number][1], &max_level, sizeof(max_level));
}

unsigned int get_maximum_level_tank(const oil_storage* os, unsigned int number){
    _send_operation_number(os->pipe_fds_in[number][1], GET_MAXIMUM_LEVEL_TANK);
    unsigned int max_level;
    read(os->pipe_fds_out[number][0], &max_level, sizeof(max_level));
    return max_level;
}

unsigned int get_current_level_tank(const oil_storage* os, unsigned int number){
    _send_operation_number(os->pipe_fds_in[number][1], GET_CURRENT_LEVEL_TANK);
    unsigned int cur_level;
    read(os->pipe_fds_out[number][0], &cur_level, sizeof(cur_level));
    return cur_level;
}

void finalize_oil_storage(oil_storage* os){
    for(int i = 0; i < os->tanks_count; ++i){
        _send_operation_number(os->pipe_fds_in[i][1], FINALIZE_STORAGE_TANK);
    }
    for(int i = 0; i < os->tanks_count; ++i){
        waitpid(os->pids[i], NULL, 0);
        close(os->pipe_fds_in[i][1]);
        close(os->pipe_fds_out[i][0]);
        free(os->pipe_fds_out[i]);
        free(os->pipe_fds_in[i]);
    }
    free(os->pipe_fds_out);
    free(os->pipe_fds_in);
    free(os->pids);
    free(os);
}

void turn_on_download_pump(oil_storage* os, unsigned int number){
    _send_operation_number(os->pipe_fds_in[number][1], TURN_ON_DOWNLOAD_PUMP);
}

void turn_off_download_pump(oil_storage* os, unsigned int number){
    _send_operation_number(os->pipe_fds_in[number][1], TURN_OFF_DOWNLOAD_PUMP);
}

int get_state_download_pump(const oil_storage* os, unsigned int number){
    _send_operation_number(os->pipe_fds_in[number][1], GET_STATE_DOWNLOAD_PUMP);
    int state_dp;
    read(os->pipe_fds_out[number][0], &state_dp, sizeof(state_dp));
    return state_dp;
}

void set_speed_download_pump(oil_storage *os, unsigned int number, unsigned int download_speed) {
    _send_operation_number(os->pipe_fds_in[number][1], SET_SPEED_DOWNLOAD_PUMP);
    write(os->pipe_fds_in[number][1], &download_speed, sizeof(download_speed));
}

unsigned int get_speed_download_pump(const oil_storage* os, unsigned int number){
    _send_operation_number(os->pipe_fds_in[number][1], GET_SPEED_DOWNLOAD_PUMP);
    unsigned int download_speed;
    read(os->pipe_fds_out[number][0], &download_speed, sizeof(download_speed));
    return download_speed;
}

void turn_on_upload_pump(oil_storage* os, unsigned int number){
    _send_operation_number(os->pipe_fds_in[number][1], TURN_ON_UPLOAD_PUMP);
}

void turn_off_upload_pump(oil_storage* os, unsigned int number){
    _send_operation_number(os->pipe_fds_in[number][1], TURN_OFF_UPLOAD_PUMP);
}

int get_state_upload_pump(const oil_storage* os, unsigned int number){
    _send_operation_number(os->pipe_fds_in[number][1], GET_STATE_UPLOAD_PUMP);
    int state_up;
    read(os->pipe_fds_out[number][0], &state_up, sizeof(state_up));
    return state_up;
}

void set_speed_upload_pump(oil_storage *os, unsigned int number, unsigned int upload_speed) {
    _send_operation_number(os->pipe_fds_in[number][1], SET_SPEED_UPLOAD_PUMP);
    write(os->pipe_fds_in[number][1], &upload_speed, sizeof(upload_speed));
}

unsigned int get_speed_upload_pump(const oil_storage* os, unsigned int number){
    _send_operation_number(os->pipe_fds_in[number][1], GET_SPEED_UPLOAD_PUMP);
    unsigned int upload_speed;
    read(os->pipe_fds_out[number][0], &upload_speed, sizeof(upload_speed));
    return upload_speed;
}

size_t get_count_tanks(const oil_storage *os){
    return os->tanks_count;
}

static void _create_process_for_tanks(oil_storage* os){
    for(int i = 0; i < os->tanks_count; ++i){
        os->pids[i] = fork();
        if (os->pids[i] == 0){
            close(os->pipe_fds_in[i][1]);
            close(os->pipe_fds_out[i][0]);
            _manage_storage_tank(os->pipe_fds_in[i][0], os->pipe_fds_out[i][1]);
            close(os->pipe_fds_in[i][0]);
            close(os->pipe_fds_out[i][1]);
            _exit(0);
        }
        close(os->pipe_fds_in[i][0]);
        close(os->pipe_fds_out[i][1]);
    }
}

static void _manage_storage_tank(int fd_in, int fd_out){
    storage_tank* st = NULL;
    for(;;){
        int operation_number;
        read(fd_in, &operation_number, sizeof(operation_number));
        switch (operation_number){
            case CREATE_STORAGE_TANK:{
                unsigned int params[4];
                read(fd_in, params, sizeof(params));
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
                write(fd_out, &state_st, sizeof(state_st));
                break;
            }
            case SET_MINIMUM_LEVEL_TANK:{
                unsigned int min_level;
                read(fd_in, &min_level, sizeof(min_level));
                set_minimum_level_storage_tank(st, min_level);
                break;
            }
            case GET_MINIMUM_LEVEL_TANK:{
                unsigned int min_level = get_minimum_level_storage_tank(st);
                write(fd_out, &min_level, sizeof(min_level));
                break;
            }
            case SET_MAXIMUM_LEVEL_TANK:{
                unsigned int max_level;
                read(fd_in, &max_level, sizeof(max_level));
                set_maximum_level_storage_tank(st, max_level);
                break;
            }
            case GET_MAXIMUM_LEVEL_TANK:{
                unsigned int max_level = get_maximum_level_storage_tank(st);
                write(fd_out, &max_level, sizeof(max_level));
                break;
            }
            case GET_CURRENT_LEVEL_TANK:{
                unsigned int cur_level = get_current_level_storage_tank(st);
                write(fd_out, &cur_level, sizeof(cur_level));
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
                write(fd_out, &state_dp, sizeof(state_dp));
                break;
            }
            case SET_SPEED_DOWNLOAD_PUMP:{
                unsigned int speed_dp;
                read(fd_in, &speed_dp, sizeof(speed_dp));
                set_speed_injection_pump(st, speed_dp);
                break;
            }
            case GET_SPEED_DOWNLOAD_PUMP:{
                unsigned int speed_dp = get_speed_injection_pump(st);
                write(fd_out, &speed_dp, sizeof(speed_dp));
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
                write(fd_out, &state_up, sizeof(state_up));
                break;
            }
            case SET_SPEED_UPLOAD_PUMP:{
                unsigned int speed_pp;
                read(fd_in, &speed_pp, sizeof(speed_pp));
                set_speed_pumping_pump(st, speed_pp);
                break;
            }
            case GET_SPEED_UPLOAD_PUMP:{
                unsigned int speed_pp = get_speed_pumping_pump(st);
                write(fd_out, &speed_pp, sizeof(speed_pp));
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

static void _send_operation_number(int fd, int operation_number){
    write(fd, &operation_number, sizeof(operation_number));
}