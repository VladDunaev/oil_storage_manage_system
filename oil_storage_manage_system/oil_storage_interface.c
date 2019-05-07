#include "oil_storage_interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <termios.h>

static size_t width_tank                = 7;
static size_t height_tank               = 11;
static size_t distance_between_tanks    = 12;
static char* lower_border_empty         = NULL;
static char* lower_border_full          = NULL;
static char* mid_empty                  = NULL;
static char* mid_half                   = NULL;
static char* mid_full                   = NULL;
static char* upper_border_empty         = NULL;
static char* upper_border_full          = NULL;
static char* between_tanks              = NULL;

static struct termios stored_settings;

static void  _set_keypress_mode();
static void  _reset_keypress_mode();

static void _generate_pseudo_graphics_string();

static void _output_tanks_labels(const oil_storage *os);

static void _output_tanks_state(const oil_storage *os);

static void _output_format_tanks_labels(const oil_storage *os, char* name, size_t real_size_name, char** labels);

static void _output_characteristics_tanks(const oil_storage *os);

pthread_t _read_chars_thread;
int continue_read_char = 1;
int chars_buffer[10000];
int last_write_char = 0;
static void _output_console(oil_storage *os);
static void* _read_chars(void* params);

static char* _implement_command(oil_storage *os, char *command_line);

void start_oil_storage_interface(oil_storage *os){
    _set_keypress_mode();
    _generate_pseudo_graphics_string();
    printf("\033[2J");
    while(continue_read_char){
        printf("\033[0;0H");
        _output_tanks_labels(os);
        _output_tanks_state(os);
        _output_characteristics_tanks(os);
        printf("\033[K\n");
        _output_console(os);
        usleep(40*1000);
    }
    pthread_join(_read_chars_thread, NULL);
    _reset_keypress_mode();
    printf("\033[2J\033[0;0H");
    fflush(stdin);
}

static void  _set_keypress_mode(){
    struct termios new_settings;
    tcgetattr(0,&stored_settings);
    new_settings = stored_settings;
    new_settings.c_lflag &= (~ICANON);
    new_settings.c_cc[VTIME] = 0;
    new_settings.c_cc[VMIN] = 1;
    tcsetattr(0,TCSANOW,&new_settings);
}

static void  _reset_keypress_mode(){
    tcsetattr(0,TCSANOW,&stored_settings);
}

static void _generate_pseudo_graphics_string(){
    static char* left_lower_corner      = "└";
    static char* right_lower_corner     = "┘";
    static char* lower_empty            = "─";
    static char* lower_full             = "▀";
    static char* left_border            = "│";
    static char* right_border           = "│";
    static char* empty                  = " ";
    static char* half                   = "▄";
    static char* full                   = "█";
    static char* left_upper_corner      = "┌";
    static char* right_upper_corner     = "┐";
    static char* upper_empty            = "─";
    static char* upper_full             = "▄";
    static char* between_tanks_char    = " ";

    if (lower_border_empty         != NULL) free(lower_border_empty);
    if (lower_border_full          != NULL) free(lower_border_full);
    if (mid_empty                  != NULL) free(mid_empty);
    if (mid_half                   != NULL) free(mid_half);
    if (mid_full                   != NULL) free(mid_full);
    if (upper_border_empty         != NULL) free(upper_border_empty);
    if (upper_border_full          != NULL) free(upper_border_full);
    if (between_tanks              != NULL) free(between_tanks);

    lower_border_empty      =       malloc(width_tank * 4);
    lower_border_full       =       malloc(width_tank * 4);
    mid_empty               =       malloc(width_tank * 4);
    mid_half                =       malloc(width_tank * 4);
    mid_full                =       malloc(width_tank * 4);
    upper_border_empty      =       malloc(width_tank * 4);
    upper_border_full       =       malloc(width_tank * 4);
    between_tanks           =       malloc(width_tank * 4);

    sprintf(lower_border_empty,    "%s",     left_lower_corner);
    sprintf(lower_border_full,     "%s",     left_lower_corner);
    sprintf(mid_empty,             "%s",     left_border);
    sprintf(mid_half,              "%s",     left_border);
    sprintf(mid_full,              "%s",     left_border);
    sprintf(upper_border_empty,    "%s",     left_upper_corner);
    sprintf(upper_border_full,     "%s",     left_upper_corner);

    for(int i = 1; i + 1 < width_tank; ++i){
        strcat(lower_border_empty,    lower_empty);
        strcat(lower_border_full,     lower_full);
        strcat(mid_empty,             empty);
        strcat(mid_half,              half);
        strcat(mid_full,              full);
        strcat(upper_border_empty,    upper_empty);
        strcat(upper_border_full,     upper_full);
    }

    strcat(lower_border_empty,    right_lower_corner);
    strcat(lower_border_full,     right_lower_corner);
    strcat(mid_empty,             right_border);
    strcat(mid_half,              right_border);
    strcat(mid_full,              right_border);
    strcat(upper_border_empty,    right_upper_corner);
    strcat(upper_border_full,     right_upper_corner);

    between_tanks[0] = '\0';
    for(int i = 0; i < distance_between_tanks; ++i){
        strcat(between_tanks, between_tanks_char);
    }
}

static void _output_tanks_labels(const oil_storage *os){
    static char* tanks_label = "резевуар №";
    static const size_t length_tanks_label = 10;

    size_t count_tanks = get_count_tanks(os);
    printf("%s%s", between_tanks, between_tanks);
    for(int i = 0; i < count_tanks; ++i){
        char* label_with_space = malloc(strlen(tanks_label) + 3 + distance_between_tanks + width_tank);
        int num = i + 1;
        sprintf(label_with_space, "%s%d", tanks_label, num);
        size_t cur_sz = length_tanks_label + 1;
        while(num /= 10) cur_sz++;
        size_t out_str_len = strlen(label_with_space);
        while(cur_sz < width_tank + distance_between_tanks){
            label_with_space[out_str_len++] = ' ';
            cur_sz++;
        }
        label_with_space[out_str_len] = '\0';
        printf("%s", label_with_space);
        free(label_with_space);
    }
    printf("\033[K\n");
}


static void _output_tanks_state(const oil_storage *os){
    size_t count_tanks = get_count_tanks(os);
    unsigned int* cur_levels = malloc(sizeof(unsigned int)*count_tanks);
    unsigned int* max_levels = malloc(sizeof(unsigned int)*count_tanks);
    for(int i = 0; i < count_tanks; ++i){
        cur_levels[i] = get_current_level_tank(os, i);
        max_levels[i] = get_maximum_level_tank(os, i);
    }
    size_t count_segments = height_tank*2 - 2;
    for(int i = 0; i < count_tanks; ++i){
        if (i == 0) printf("%s%s", between_tanks, between_tanks);
        else printf("%s", between_tanks);
        if (count_segments * cur_levels[i] >= (count_segments - 1) * max_levels[i]){
            printf("%s", upper_border_full);
        }else{
            printf("%s", upper_border_empty);
        }
    }
    printf("\033[K\n");
    for(size_t j = height_tank - 1; j > 1; --j){
        int segment_value = (int)(2*j - 2);
        for(int i = 0; i < count_tanks; ++i){
            if (i == 0) printf("%s%s", between_tanks, between_tanks);
            else printf("%s", between_tanks);
            if (count_segments * cur_levels[i] >= segment_value * max_levels[i]){
                printf("%s", mid_full);
            } else if (count_segments * cur_levels[i] >= (segment_value - 1) * max_levels[i]){
                printf("%s", mid_half);
            } else {
                printf("%s", mid_empty);
            }
        }
        printf("\033[K\n");
    }
    for(int i = 0; i < count_tanks; ++i){
        if (i == 0) printf("%s%s", between_tanks, between_tanks);
        else printf("%s", between_tanks);
        if (count_segments * cur_levels[i] >= max_levels[i]){
            printf("%s", lower_border_full);
        }else{
            printf("%s", lower_border_empty);
        }
    }
    printf("\033[K\n");
    free(cur_levels);
    free(max_levels);
}

static void _output_format_tanks_labels(const oil_storage *os, char* name, size_t real_size_name, char** labels){
    char* label_with_space = malloc(strlen(name) + distance_between_tanks*2);
    sprintf(label_with_space, "%s", name);
    size_t label_with_space_len = strlen(label_with_space);
    while(real_size_name++ < 2 * distance_between_tanks){
        label_with_space[label_with_space_len++] = ' ';
    }
    label_with_space[label_with_space_len] = '\0';
    printf("%s", label_with_space);
    free(label_with_space);
    size_t count_tanks = get_count_tanks(os);
    for(int i = 0; i < count_tanks; ++i){
        char* chr_label = malloc(strlen(labels[i]) + distance_between_tanks + width_tank);
        sprintf(chr_label, "%s", labels[i]);
        size_t chr_label_len = strlen(chr_label);
        while(chr_label_len < distance_between_tanks + width_tank){
            chr_label[chr_label_len++] = ' ';
        }
        chr_label[chr_label_len] = '\0';
        printf("%s", chr_label);
        free(chr_label);
    }
    printf("\033[K\n");
}

static void _output_characteristics_tanks(const oil_storage *os){
    size_t count_tanks = get_count_tanks(os);
    int* tanks_on = malloc(sizeof(int)*count_tanks);
    unsigned int* cur_levels = malloc(sizeof(unsigned int)*count_tanks);
    unsigned int* max_levels = malloc(sizeof(unsigned int)*count_tanks);
    unsigned int* min_levels = malloc(sizeof(unsigned int)*count_tanks);
    int* download_on = malloc(sizeof(int)*count_tanks);
    unsigned int* download_speed = malloc(sizeof(unsigned int)*count_tanks);
    int* upload_on = malloc(sizeof(int)*count_tanks);
    unsigned int* upload_speed = malloc(sizeof(unsigned int)*count_tanks);
    for(int i = 0; i < count_tanks; ++i){
        tanks_on[i]         = get_state_tank(os,i);
        cur_levels[i]       = get_current_level_tank(os, i);
        max_levels[i]       = get_maximum_level_tank(os, i);
        min_levels[i]       = get_minimum_level_tank(os, i);
        download_on[i]      = get_state_download_pump(os, i);
        download_speed[i]   = get_speed_download_pump(os, i);
        upload_on[i]        = get_state_upload_pump(os, i);
        upload_speed[i]     = get_speed_upload_pump(os, i);
    }
    char** labels = malloc(sizeof(char*) * count_tanks);
    for(int i = 0; i < count_tanks; ++i) labels[i] = malloc(sizeof(char) * 20);

    for(int i = 0; i < count_tanks; ++i) {
        if (tanks_on[i] == STORAGE_TANK_ON) sprintf(labels[i], "ON");
        if (tanks_on[i] == STORAGE_TANK_OFF) sprintf(labels[i], "OFF");
    }
    _output_format_tanks_labels(os, "Состояние резервуара:", 21, labels);


    for(int i = 0; i < count_tanks; ++i) sprintf(labels[i], "%u", cur_levels[i]);
    _output_format_tanks_labels(os, "Текущий уровень:", 16, labels);

    for(int i = 0; i < count_tanks; ++i) sprintf(labels[i], "%u", max_levels[i]);
    _output_format_tanks_labels(os, "Максимальный уровень:", 21, labels);

    for(int i = 0; i < count_tanks; ++i) sprintf(labels[i], "%u", min_levels[i]);
    _output_format_tanks_labels(os, "Минимальный уровень:", 20, labels);

    for(int i = 0; i < count_tanks; ++i) {
        if (download_on[i] == PUMP_ON) sprintf(labels[i], "ON");
        if (download_on[i] == PUMP_OFF) sprintf(labels[i], "OFF");
    }
    _output_format_tanks_labels(os, "Насос закачки:", 14, labels);

    for(int i = 0; i < count_tanks; ++i) sprintf(labels[i], "%u", download_speed[i]);
    _output_format_tanks_labels(os, "Скорость закачки:", 17, labels);

    for(int i = 0; i < count_tanks; ++i) {
        if (upload_on[i] == PUMP_ON) sprintf(labels[i], "ON");
        if (upload_on[i] == PUMP_OFF) sprintf(labels[i], "OFF");
    }
    _output_format_tanks_labels(os, "Насос откачки:", 14, labels);

    for(int i = 0; i < count_tanks; ++i) sprintf(labels[i], "%u", upload_speed[i]);
    _output_format_tanks_labels(os, "Скорость откачки:", 17, labels);

    for(int i = 0; i < count_tanks; ++i) free(labels[i]);
    free(labels);
    free(upload_speed);
    free(upload_on);
    free(download_speed);
    free(download_on);
    free(min_levels);
    free(max_levels);
    free(cur_levels);
    free(tanks_on);
}

static void _output_console(oil_storage *os){
    const size_t console_log_max_size = 200;
    static const size_t console_string_max_len = 500;
    static char* console_log[200];
    static size_t console_log_size = 0;
    static int is_initial = 0;
    static int current_ptr = 0;
    if (!is_initial){
        for(int i = 0; i < console_log_max_size; ++i){
            console_log[i] = malloc(sizeof(char)*console_string_max_len);
            memset(console_log[i], 0, sizeof(char)*console_string_max_len);
        }
        console_log[0][0] = '>';
        is_initial = 1;
        pthread_create(&_read_chars_thread, NULL, _read_chars, NULL);
    }
    int start_i = 0;
    if (console_log_size > 20) start_i = console_log_size - 20;
    for(; start_i < console_log_size; ++start_i)  printf("%s\033[K\n", console_log[start_i]);
    printf("\033[s\n\033[K\n\033[K\n\033[K\033[u");
    while(current_ptr < last_write_char) {
        char c = chars_buffer[current_ptr++];
        if (c == '\n') {
            char* ans = _implement_command(os, console_log[console_log_size] + 1);
            printf("%s\033[K\n", console_log[console_log_size]);
            console_log_size++;
            strcpy(console_log[console_log_size], ans);
            printf("%s\033[K\n", console_log[console_log_size]);
            console_log_size++;
            console_log[console_log_size][0] = '>';
        }else{
            size_t cl_len = strlen(console_log[console_log_size]);
            if (c == 127){
                if (cl_len > 1) console_log[console_log_size][cl_len - 1] = '\0';
            }else{
                console_log[console_log_size][cl_len] = c;
            }
        }
    }
    printf("%s\033[K", console_log[console_log_size]);
    fflush(stdout);
}

static void* _read_chars(void* params){
    while(continue_read_char){
        chars_buffer[last_write_char] = getchar();
        last_write_char++;
    }
    return NULL;
}


static char* _implement_command(oil_storage *os, char *command_line){
    char command[100];
    sscanf(command_line, "%s", command);
    if (strcmp(command, "exit") == 0){
        continue_read_char = 0;
        return "ok (press any key)";
    }
    unsigned int number = strtol(command_line + strlen(command) + 1, &command_line, 10) - 1;
    if (strcmp(command, "turn_on_tank") == 0){
        turn_on_tank(os, number);
        return "ok";
    }
    if (strcmp(command, "turn_off_tank") == 0){
        turn_off_tank(os, number);
        return "ok";
    }
    if (strcmp(command, "set_minimum_level_tank") == 0){
        unsigned int min_level = strtol(command_line + 1, NULL, 10);
        set_minimum_level_tank(os, number, min_level);
        return "ok";
    }
    if (strcmp(command, "set_maximum_level_tank") == 0){
        unsigned int max_level = strtol(command_line + 1, NULL, 10);
        set_maximum_level_tank(os, number, max_level);
        return "ok";
    }
    if (strcmp(command, "turn_on_download_pump") == 0){
        turn_on_download_pump(os, number);
        return "ok";
    }
    if (strcmp(command, "turn_off_download_pump") == 0){
        turn_off_download_pump(os, number);
        return "ok";
    }
    if (strcmp(command, "set_speed_download_pump") == 0){
        unsigned int speed = strtol(command_line + 1, NULL, 10);
        set_speed_download_pump(os, number, speed);
        return "ok";
    }
    if (strcmp(command, "turn_on_upload_pump") == 0){
        turn_on_upload_pump(os, number);
        return "ok";
    }
    if (strcmp(command, "turn_off_upload_pump") == 0){
        turn_off_upload_pump(os, number);
        return "ok";
    }
    if (strcmp(command, "set_speed_upload_pump") == 0){
        unsigned int speed = strtol(command_line + 1, NULL, 10);
        set_speed_upload_pump(os, number, speed);
        return "ok";
    }
    if (strcmp(command, "get_state_tank") == 0){
        int state = get_state_tank(os, number);
        if (state == STORAGE_TANK_ON) return "ON";
        return "OFF";
    }
    if (strcmp(command, "get_minimum_level_tank") == 0){
        unsigned int min_level = get_minimum_level_tank(os, number);
        char* min_level_str = malloc(sizeof(char) * 9);
        sprintf(min_level_str, "%u", min_level);
        return min_level_str;
    }
    if (strcmp(command, "get_maximum_level_tank") == 0){
        unsigned int max_level = get_maximum_level_tank(os, number);
        char* max_level_str = malloc(sizeof(char) * 9);
        sprintf(max_level_str, "%u", max_level);
        return max_level_str;
    }
    if (strcmp(command, "get_current_level_tank") == 0){
        unsigned int cur_level = get_current_level_tank(os, number);
        char* cur_level_str = malloc(sizeof(char) * 9);
        sprintf(cur_level_str, "%u", cur_level);
        return cur_level_str;
    }
    if (strcmp(command, "get_state_download_pump") == 0){
        int state = get_state_download_pump(os, number);
        if (state == PUMP_ON) return "ON";
        return "OFF";
    }
    if (strcmp(command, "get_speed_download_pump") == 0){
        unsigned int speed = get_speed_download_pump(os, number);
        char* speed_str = malloc(sizeof(char) * 9);
        sprintf(speed_str, "%u", speed);
        return speed_str;
    }
    if (strcmp(command, "get_state_upload_pump") == 0){
        int state = get_state_upload_pump(os, number);
        if (state == PUMP_ON) return "ON";
        return "OFF";
    }
    if (strcmp(command, "get_speed_upload_pump") == 0){
        unsigned int speed = get_speed_upload_pump(os, number);
        char* speed_str = malloc(sizeof(char) * 9);
        sprintf(speed_str, "%u", speed);
        return speed_str;
    }
    return "Unknown command";
}