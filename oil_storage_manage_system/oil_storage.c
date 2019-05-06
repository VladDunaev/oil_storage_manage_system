#include "oil_storage.h"
#include "storage_tank.h"
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>

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
    #define GET_SPEED_DOWNLOAD_PUMP 12
    #define TURN_ON_UPLOAD_PUMP     13
    #define TURN_OFF_UPLOAD_PUMP    14
    #define GET_STATE_UPLOAD_PUMP   15
    #define SET_SPEED_UPLOAD_PUMP   16
    #define GET_SPEED_UPLOAD_PUMP   17
    #define FINALIZE_STORAGE_TANK   18
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
        int operation_number = CREATE_STORAGE_TANK;
        write(os->pipe_fds_in[i][1], &operation_number, sizeof(operation_number));
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


static void _create_process_for_tanks(oil_storage* os){
    for(int i = 0; i < os->tanks_count; ++i){
        os->pids[i] = fork();
        if (os->pids[i] == 0){
            close(os->pipe_fds_in[i][1]);
            close(os->pipe_fds_out[i][0]);
            _manage_storage_tank(os->pipe_fds_in[i][0], os->pipe_fds_out[i][1]);
            //finalize????
            return;
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
            default:{
                return;
            }
        }
    }
}