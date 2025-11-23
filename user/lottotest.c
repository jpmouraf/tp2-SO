#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/pstat.h"

#define MAX_CHILDREN 10 
#define LOOP_COUNT 1000
#define PRINT_INTERVAL 1

int main(int argc, char *argv[])
{
    int num_children;
    int tickets[MAX_CHILDREN];
    int pids[MAX_CHILDREN];
    int child_ticks[MAX_CHILDREN];

    if (argc < 2) {
        fprintf(2, "Usage: lottotest <ticket_ratio_1> [ticket_ratio_2 ...]\n");
        exit(1);
    }

    num_children = argc - 1;
    if (num_children > MAX_CHILDREN) {
        fprintf(2, "Error: Maximum %d children supported.\n", MAX_CHILDREN);
        exit(1);
    }

    for (int i = 0; i < num_children; i++) {
        tickets[i] = atoi(argv[i + 1]);
        if (tickets[i] <= 0) {
            fprintf(2, "Error: Ticket ratios must be positive integers.\n");
            exit(1);
        }
        child_ticks[i] = -1; // Initialize child_ticks here for dynamic use
    }

    // Create child processes
    for (int i = 0; i < num_children; i++) {
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
    printf("[time] ");
    for (int i = 0; i < num_children; i++) {
        printf("ticks_p%d%s", i + 1, (i == num_children - 1) ? "" : ",");
    }
    printf("\n");

    for (int i = 0; i < LOOP_COUNT; i++) {
        if (i % PRINT_INTERVAL == 0) {
            struct pstat ps;
            if (getpinfo(&ps) < 0) {
                printf("getpinfo failed\n");
                break;
            }

            // Reset child_ticks for each print interval
            for (int k = 0; k < num_children; k++) {
                child_ticks[k] = -1;
            }

            for (int j = 0; j < NPROC; j++) {
                for (int k = 0; k < num_children; k++) {
                    if (ps.pid[j] == pids[k]) {
                        child_ticks[k] = ps.ticks[j];
                    }
                }
            }
            
            int all_children_found = 1;
            for(int k=0; k < num_children; k++) {
                if(child_ticks[k] == -1) {
                    all_children_found = 0;
                    break;
                }
            }

            if (all_children_found) {
                printf("[%d] ", i);
                for (int k = 0; k < num_children; k++) {
                    printf("%d%s", child_ticks[k], (k == num_children - 1) ? "" : ",");
                }
                printf("\n");
            }
        }
        pause(1);
    }

    for (int i = 0; i < num_children; i++) {
        kill(pids[i]);
    }

    // Wait for child processes to exit
    for (int i = 0; i < num_children; i++) {
        wait(0);
    }

    exit(0);
}
