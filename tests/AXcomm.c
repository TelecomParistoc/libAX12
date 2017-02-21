#include <walkingdriver/driver.h>
#include <walkingdriver/ax-comm.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Test communication with AX12s */

int main() {
    uint8_t position;
    uint8_t error;
    int code;

    printf("init : %d\n", initAXcomm(115200));

    code = axRead8(146, 0x05, &position, &error);
    printf("read8 : code = %d, error = %x, result = %d\n", code, error, position);
    while(1) {
        axWrite8(0xFE, 0x19, 1, &error);
        waitFor(500);
        axWrite8(0xFE, 0x19, 0, &error);
        waitFor(500);
    }
    return 0;
}
