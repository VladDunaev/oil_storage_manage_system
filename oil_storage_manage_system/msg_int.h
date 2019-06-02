#ifndef OIL_STORAGE_MANAGE_SYSTEM_OIL__MSG_INT_H
#define OIL_STORAGE_MANAGE_SYSTEM_OIL__MSG_INT_H

/**
 * структура очереди целочисленных сообщений
 */
struct _msg_int;
typedef struct _msg_int msg_int;

/**
 * создать очередь целочисленных сообщений
 * @param path путь к исполняемому файлу
 * @return созданную очередь целочисленных сообщений
 */
msg_int* create_msg_int_handler(const char* path);

/**
 * Записать целое число
 * @param m указатель на очередь целочисленных сообщений
 * @param type тип сообщения
 * @param number записываемое число
 */
void write_int(msg_int* m, long type, int number);

/**
 * считать число из очереди целочисленных сообщений
 * @param m указатель на очередь целочисленных сообщений
 * @param type тип сообщения
 * @return считанное число
 */
int read_int(msg_int* m, long type);

/**
 * удалить очередь сообщений
 * @param m указатель на очередь целочисленных сообщений
 */
void finalize_msg_int_handler(msg_int* m);

#endif //OIL_STORAGE_MANAGE_SYSTEM_OIL__MSG_INT_H
