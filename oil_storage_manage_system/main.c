#include <stdio.h>
#include "oil_storage.h"
#include "oil_storage_interface.h"


int main() {
    oil_storage* os = create_oil_storage(5,0,10000,3,10);
    for(int i = 0; i < 5; ++i){
        turn_on_download_pump(os, i);
    }
    start_oil_storage_interface(os);
    return 0;
}