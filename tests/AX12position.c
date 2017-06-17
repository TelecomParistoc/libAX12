#include <walkingdriver/driver.h>
#include <stdio.h>
#include <stdlib.h>

/* a simple program to read an AX12 position */

int main() {
    int AX12_ID;
    initAX12(115200);

    printf("Enter AX-12 ID : ");
    scanf("%d", &AX12_ID);
    if(AX12_ID > 254 || AX12_ID < 0) {
        printf("ID must be between 0 and 254\n");
        return -1;
    }

    // no torque, no speed
    AX12setTorque(AX12_ID, 0);
    //AX12setSpeed(AX12_ID, 0);

    while(1) {
        waitFor(200);
        printf("AX-12 %d at position %f\n", AX12_ID, AX12getPosition(AX12_ID));
    }
    return 0;
}
