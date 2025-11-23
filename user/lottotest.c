#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/pstat.h"

#define NUM_CHILDREN 3
#define TICKET_RATIO_1 30
#define TICKET_RATIO_2 30
#define TICKET_RATIO_3 30
#define LOOP_COUNT 10
#define PRINT_INTERVAL 1

int main(int argc, char *argv[])
{
    int pids[NUM_CHILDREN];
    int tickets[NUM_CHILDREN] = {TICKET_RATIO_1, TICKET_RATIO_2, TICKET_RATIO_3};

    // Create child processes
    for (int i = 0; i < NUM_CHILDREN; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            if (settickets(tickets[i]) < 0) {
                printf("settickets failed\n");
                exit(1);
            }
            // Infinite loop to consume CPU
            volatile int x = 0;
            while (1) {
                x++;
            }
        }
    }

    // Parent process
    printf("ticks_p1,ticks_p2,ticks_p3\n");

    for (int i = 0; i < LOOP_COUNT; i++) {
        if (i % PRINT_INTERVAL == 0) {
            struct pstat ps;
            if (getpinfo(&ps) < 0) {
                printf("getpinfo failed\n");
                break;
            }

            int child_ticks[NUM_CHILDREN] = {-1, -1, -1};
            for (int j = 0; j < NPROC; j++) {
                for (int k = 0; k < NUM_CHILDREN; k++) {
                    if (ps.pid[j] == pids[k]) {
                        child_ticks[k] = ps.ticks[j];
                    }
                }
            }

            if (child_ticks[0] != -1 && child_ticks[1] != -1 && child_ticks[2] != -1) {
                printf("[%d] %d,%d,%d\n", i, child_ticks[0], child_ticks[1], child_ticks[2]);
            }
        }
        pause(1);
    }

    for (int i = 0; i < NUM_CHILDREN; i++) {
        kill(pids[i]);
    }

    // Wait for child processes to exit
    for (int i = 0; i < NUM_CHILDREN; i++) {
        wait(0);
    }

    exit(0);
}
