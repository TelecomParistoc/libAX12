#include <walkingdriver/driver.h>
#include <stdio.h>
#include <stdlib.h>

/* Test timing functions */

float cnt = 0;
static void f() {
    cnt += 0.5;
    printf("%f sec\n", cnt);
}
static void g() {
    printf("I should never be called\n");
}

int main() {
    int uid = scheduleIn(500, g);
    // check cancelScheduled works
    cancelScheduled(uid);
    // check it cannot be cancelled twice
    cancelScheduled(uid);
    // should print an ERROR then the time since start each 500ms up to 20sec
    for (int i = 1; i <= 41; i++) {
        scheduleIn(i*500, f);
    }
	while(1) {
        waitFor(500);
        printf("----------\n");
    }
	return 0;
}
