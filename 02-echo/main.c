#include "pico/stdlib.h"
#include "stdio.h"
#include "stdlib.h"


main(){
    stdio_init_all();
    while(1)
    {
        char symbol = getchar();
        printf("received char: %c [ ASCII code: %d ]\n", symbol, symbol);
        
    } 
}