#include <walkingdriver/driver.h>
#include <stdio.h>
#include <stdlib.h>

#define AX12_ID 163

/* Test AX12 move callback */
void onHigh();
void onLow() {
	printf("move finished\n");
	AX12move(AX12_ID, 20, onHigh);
}
void onHigh() {
	AX12move(AX12_ID, -20, onLow);
}

int main() {
	printf("init : %d\n", initAX12(115200));

	AX12setTorque(AX12_ID, 100);
	AX12setSpeed(AX12_ID, 10);
	AX12move(AX12_ID, 0, onLow);

	while(1)
		waitFor(100);
}
