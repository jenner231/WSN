#include <stdint.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include "contiki.h"
#include "net/routing/routing.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "os/storage/cfs/cfs.h"

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

#define SEND_INTERVAL		  (10 * CLOCK_SECOND)
#define N 512 //size of signal
#define L 8 // length of DCT
#define M 4 //coefficients that determines amount of data transmitted


//Generating DCT matrix 
void generate_dct_matrix(double H[][L], int l) {
for(int i = 0; i < L; i++){
	for(int j = 0; j < L; j++){
		if(i == 0){
			H[i][j] = sqrt(1. / l); //delta = 1
			}
		else{
			H[i][j] = sqrt(2. / l) * cos((M_PI / l) * (j + 1. / 2.) * i); //delta = 0
			}
		}
	}
}
//generate the inverse dct matrix
void generate_idct_matrix(double H_inv[][L], int l) {
    // First, generate the regular DCT matrix
    double H[L][L];
    generate_dct_matrix(H, l);

    // Then, transpose it to get the inverse DCT matrix
    for (int i = 0; i < L; i++) {
        for (int j = 0; j < L; j++) {
            H_inv[i][j] = H[j][i]; // Transpose operation
        }
    }
}
//Dct transformation of the signal
void dct_compress(double x[N], double y_transmit[M][N / L]) {
    double H[L][L]; // DCT matrix for compressed signal
    double y[L][N / L]; // output
    
    // Generate DCT matrix
    generate_dct_matrix(H, L);
    
    // Apply DCT transformation block-wise
    for(int block = 0; block < N / L; block++){
        for(int j = 0; j < L; j++){
            y[j][block] = 0; // Initialize to zero before summation
            for(int k = 0; k < L; k++){
                y[j][block] += H[j][k] * x[block * L + k];
            }
        }
    }
    
    // Copy first M coefficients for each block
    for(int block = 0; block < N / L; block++){
        for(int j = 0; j < M; j++){
            y_transmit[j][block] = y[j][block];
        }
    }
}
//Decompression of the signal
void dct_decompress(double x[N], double y_received[M][N / L]){
double H_inv[L][L];
generate_idct_matrix(H_inv, L);
//reconstruction
for(int k = 0; k < N; k++){
	for(int i = 0; i < L; i++){
		x[k * L + i] = 0;
		for(int j = 0; j < M; j++){
			x[k * L + i] += H_inv[i][j] * y_received[j][k];
			}
		}
	}
}


//Sine wave for a signal, can change to whatever type of signal we want to produce
void generate_signal(double x[N]) {
for (int i = 0; i < N; i++){
	x[i] = sin(2 * M_PI * i / N);
	}
}

/*---------------------------------------------*/
PROCESS(decompress_process, "Decompress process");
AUTOSTART_PROCESSES(&decompress_process);
/*---------------------------------------------*/
//define global variables
//Some of the naming makes more sense when the code was split on different motes (y_received, y_transmit, x_rec...)
static struct etimer periodic_timer;
static double x[N]; // Signal
static double y_transmit[M][N / L]; //transmitted signal
static double x_rec[N]; //Reconstructed signal
PROCESS_THREAD(decompress_process, ev, data)
{
	PROCESS_BEGIN();
		//Initiate timer to trigger event		
		etimer_set(&periodic_timer, SEND_INTERVAL);
		generate_signal(x);
  		dct_compress(x, y_transmit);
		printf("Signal has now been compressed\n");
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
		printf("\nStarting decompression\n");
		dct_decompress(x_rec, y_transmit);		
		//writing decompressed signal to file, so we can model the decompressed signal
		int file = cfs_open("logs.txt", CFS_WRITE);
		if (file != -1) {
			char buffer[50];
			int length = sizeof(x_rec) / sizeof(x_rec[0]);
    			for (size_t i = 0; i < length; i++) {
        			// Convert each float to a string
        			snprintf(buffer, sizeof(buffer), "%f\n", x_rec[i]);
        			// Write the string to file
        			cfs_write(file, buffer, strlen(buffer));
    				}
			cfs_close(file);
		}
		printf("Decompression finished\n");
	PROCESS_END();
}