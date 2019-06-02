#include "msg_int.h"
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <stdio.h>

/**
 * Создать имя файла, с которым будет связана очередь сообщений
 * @param path путь к исполняему файлу
 * @return имя файла
 */
static char* _create_msg_file_name(const char *path);

/**
 * структура очереди целочисленных сообщений
 */
struct _msg_int{
    /**
     * id очереди сообщений
     */
    int msgid;
    /**
     * ключ, связанный с очередью сообщений
     */
    key_t key;
    /**
     * путь к файлу, связанного с очередью собщений
     */
    char* full_path;
};

struct msg_buffer {
    long msg_type;
    int data[1];
};

msg_int* create_msg_int_handler(const char* path){
    char* full_file_name = _create_msg_file_name(path);
    creat(full_file_name, 0666);
    msg_int* m = malloc(sizeof(msg_int));
    m->key = ftok(full_file_name, 0);
    m->msgid = msgget(m->key, 0666 | IPC_CREAT);
    m->full_path = full_file_name;
    return m;
}

void write_int(msg_int* m, long type, int number){
    struct msg_buffer msg = {type, {number}};
    msgsnd(m->msgid, &msg, sizeof(msg), 0);
}

int read_int(msg_int* m, long type){
    struct msg_buffer msg;
    msgrcv(m->msgid, &msg, sizeof(msg), type, 0);
    return msg.data[0];
}

void finalize_msg_int_handler(msg_int* m){
    msgctl(m->msgid, IPC_RMID, NULL);
    remove(m->full_path);
    free(m->full_path);
    free(m);
}

static char* _create_msg_file_name(const char *path){
    static const char* file_name = "msg_file_temp";
    static unsigned int counter = 0;
    size_t file_name_len = strlen(file_name);
    size_t path_len = strlen(path);
    while(path[path_len - 1] != '/') path_len--;
    char* full_file_name = malloc(sizeof(char) * (file_name_len + path_len + 11 + 1));
    strcpy(full_file_name, path);
    strcpy(full_file_name + path_len, file_name);
    memset(full_file_name + file_name_len + path_len, 0, sizeof(char) * (11 + 1));
    sprintf(full_file_name + file_name_len + path_len, "%u", counter++);
    return full_file_name;
}