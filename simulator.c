/* SUBMIT ONLY THIS FILE */
/* NAME: ....... */
/* UCI ID: .......*/

// only include standard libraries.
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "simulator.h" // implements

#define EPSILON 0.02
#define MAX_CYCLE 1000000
double PI_VAL = 3.14159265358979323846;

void simulate(double *avg_access_time,
              int avg_access_time_len,
              int procs,
              char dist){

    for (int m = 1; m <= avg_access_time_len; m++) {
        int *access_count = (int *)calloc(procs, sizeof(int));
        int *current_request = (int *)calloc(procs, sizeof(int));
        int *base_module = NULL;    // for normal distribution
        if (dist == 'n') base_module = (int *)calloc(procs, sizeof(int));

        for (int p = 0; p < procs; p++) {
            if (dist == 'n') {
                base_module[p] = rand_uniform(m);
                current_request[p] = base_module[p];
            } else {
                current_request[p] = rand_uniform(m);
            }
        }
        
        // proc_orderd[i] is the i-th processor that should be served next round
        int *proc_order = (int *)malloc(procs * sizeof(int));
        for (int i = 0; i < procs; i++) {
            proc_order[i] = i;
        }        

        int cycle = 0;
        double prev_sys_avg = 0.0, curr_sys_avg = 0.0;
        int all_defined = 0;

        /** 
        * 1. For each processor, if its requested memory module is free, mark it as paired,
        *    increment its access counter, and generate a new request.
        * 2. Rotate the processor order so that the first processor that did 
        *    not pair becomes the new first processor.
        * 3. Compute the cumulative average access time from the cycle count
        *    and access counts of served processors.
        * 4. If all processors have been served and the relative change in
        *    average access time is less than EPSILON, terminate the loop.
        */
        while (cycle < MAX_CYCLE) {
            cycle++;
            // mem_modules[i] = 0 if mem[i] free, otherwise = 1;
            // paired[i] = 0 if processor[i] is waiting, otherwise = 1;
            int *mem_modules = (int *)calloc(m, sizeof(int));
            int *paired = (int *)calloc(procs, sizeof(int));

            for (int i = 0; i < procs; i++) {
                int proc_idx = proc_order[i];
                int req = current_request[proc_idx];
                if (mem_modules[req] == 0) {
                    mem_modules[req] = 1;
                    paired[proc_idx] = 1;
                    access_count[proc_idx]++;

                    if (dist == 'n') {
                        current_request[proc_idx] = rand_normal_wrap(base_module[proc_idx], 5, m);
                    } else {
                        current_request[proc_idx] = rand_uniform(m);
                    }
                }                          
            }
            free(mem_modules);
            
            int first_waiting_proc = -1;
            for (int i = 0; i < procs; i++) {
                int proc_idx = proc_order[i];
                if (paired[proc_idx] == 0) {
                    first_waiting_proc = 1;
                    break;
                }
            }
            // the first waiting processor is re-labeled with index 0
            if (first_waiting_proc != -1) {
                int *new_order = (int *)malloc(procs * sizeof(int));
                for (int i = 0; i < procs; i++) {
                    new_order[i] = proc_order[(first_waiting_proc + i) % procs];
                }
                free(proc_order);  
                proc_order = new_order; 
            }        
            free(paired);
            
            all_defined = 1;
            double sum = 0.0;
            for (int i = 0; i < procs; i++) {
                if (access_count[i] == 0) {
                    all_defined = 0;
                    break;
                }
                sum += ((double)cycle / access_count[i]);
            }
            if (all_defined) {
                curr_sys_avg = sum / procs;
                if (cycle > 1) {
                    double diff = fabs(1.0 - (prev_sys_avg / curr_sys_avg));
                    if (diff < EPSILON)
                        break;
                }
                prev_sys_avg = curr_sys_avg;
            }            
        }

        avg_access_time[m - 1] = curr_sys_avg;
        free(proc_order);
        free(access_count);
        free(current_request);
        if (base_module != NULL) {
            free(base_module);
        }            
    }
}

int rand_uniform(int max){
    return rand() % max;
}

int rand_normal_wrap(int mean, int dev, int max){
    static double U, V;
    static int phase = 0;
    double Z;
    if(phase == 0){
        U = (rand() + 1.) / (RAND_MAX + 2.);
        V = rand() / (RAND_MAX + 1.);
        Z = sqrt(-2 *log(U)) * sin(2 * PI_VAL * V);
    }else{
        Z = sqrt(-2 * log(U)) * cos(2 * PI_VAL * V);
    }
    phase = 1 - phase;
    double res = dev*Z + mean;

    // round result up or down depending on whether
    // it is even or odd. This compensates some bias.
    int res_int;
    // if even, round up. If odd, round down.
    if ((int)res % 2 == 0)
        res_int = (int)(res+1);
    else
        res_int = (int)(res);

    // wrap result around max
    int res_wrapped = res_int % max;
    // deal with % of a negative number in C
    if(res_wrapped < 0)
        res_wrapped += max;
    return res_wrapped;
}


