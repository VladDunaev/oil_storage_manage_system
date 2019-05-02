#include "oil_storage.h"
#include "storage_tank.h"
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

/**
 * создать процессы для каждого резевуара
 * @param os указатель на нефтрехранилище
 */
static void _create_process_for_tanks(oil_storage* os);

/**
 * функция управления резервуаром
 * @param st резервуар
 * @param fd_in файловый дескриптор канала для чтения команд
 * @param fd_out файловый дескриптор канала для ответа на команды
 */
static void _manage_storage_tank(storage_tank* st, int fd_in, int fd_out);

/**
 * Хранилище нефти
 */
struct _oil_storage{
    /**
     * массив указателей на резервуары
     */
    storage_tank** tanks;
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
    os->tanks = malloc(sizeof(oil_storage*)*os->tanks_count);
    os->pids = malloc(sizeof(pid_t)*os->tanks_count);
    os->pipe_fds_in = malloc(sizeof(int*)*os->tanks_count);
    os->pipe_fds_out = malloc(sizeof(int*)*os->tanks_count);
    for(int i = 0; i < os->tanks_count; ++i){
        os->tanks[i] = create_storage_tank(min_level, max_level, speed_download_pump, speed_upload_pump);
        os->pipe_fds_in[i] = malloc(sizeof(int)*2);
        os->pipe_fds_out[i] = malloc(sizeof(int)*2);
        pipe(os->pipe_fds_in[i]);
        pipe(os->pipe_fds_out[i]);
    }
    _create_process_for_tanks(os);
    return os;
}


static void _create_process_for_tanks(oil_storage* os){
    for(int i = 0; i < os->tanks_count; ++i){
        os->pids[i] = fork();
        if (os->pids[i] == 0){
            close(os->pipe_fds_in[i][0]);
            close(os->pipe_fds_out[i][1]);
            _manage_storage_tank(os->tanks[i], os->pipe_fds_in[i][0], os->pipe_fds_out[i][1]);
            //finalize????
            return;
        }
        close(os->pipe_fds_in[i][1]);
        close(os->pipe_fds_out[i][0]);
    }
}