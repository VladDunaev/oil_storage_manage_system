#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "oil_storage.h"
#include "oil_storage_interface.h"

#define MIN_LEVEL_STORAGE_DEFAULT 1000
#define MAX_LEVEL_STORAGE_DEFAULT 25000
#define MAX_SPEED 10

int main(int argc, char* argv[]) {
    srand((unsigned int)time(0));
    size_t cnt_tanks = 5;
    if (argc > 1){
        cnt_tanks = (size_t)strtol(argv[1], NULL, 10);
    }
    oil_storage* os = create_oil_storage(cnt_tanks, MIN_LEVEL_STORAGE_DEFAULT, MAX_LEVEL_STORAGE_DEFAULT, 0, 0);
    for(unsigned int i = 0; i < cnt_tanks; ++i){
        turn_on_download_pump(os, i);
        set_speed_download_pump(os, i, (unsigned int)(rand()%MAX_SPEED + 1));
        set_speed_upload_pump(os, i, (unsigned int)(rand()%MAX_SPEED + 1));
    }
    start_oil_storage_interface(os);
    finalize_oil_storage(os);
    return 0;
}