#include <stdio.h>
#include <unistd.h>

int main() {
    while(1) {
        printf("This is the phrase.\n");
        usleep(500000);
    }
    return 0;
}