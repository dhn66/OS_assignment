#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

// Constant values definitions
#define N_COOK         2 	// Number of cooks   in the pizzeria
#define N_OVEN        15    // Number of ovens   in the pizzeria
#define N_PACKER       2	// Number of packers in the pizzeria
#define N_DELIVERER   10	// Number of deliverers in the pizzeria
						
#define T_ORDERLOW     1	// Min time for order
#define T_ORDERHIGH    3    // Max time for order

#define N_ORDERLOW     1	// Min number of pizzas ordered
#define N_ORDERHIGH    5	// Max number of pizzas	ordered

#define P_PLAIN		  60    // 60% probability for plain pizza

#define T_PAYMENTLOW   1 	// Min time for payment
#define T_PAYMENTHIGH  3	// Max time for payment

#define P_FAIL		  10    // 10% probability for failed order

#define C_PLAIN		  10	// Cost of plain   pizza
#define C_SPECIAL     12    // Cost of special pizza

#define T_PREP 		   1    // Time needed for pizza preparation
#define T_BAKE 		  10	// Time needed for pizza baking
#define T_PACK         1    // Time needed for pizza packing

#define T_DELLOW 	   5    // Min time for delivery
#define T_DELHIGH     15    // Max time for delivery


// Mutexes definitions 
pthread_mutex_t cookMutex;		
pthread_mutex_t ovenMutex;		 
pthread_mutex_t packerMutex;	
pthread_mutex_t deliveryMutex;	
pthread_mutex_t printMutex;	
pthread_mutex_t statsMutex;	

// Condition variables definitions
pthread_cond_t  cookCondVar;
pthread_cond_t  ovenCondVar;
pthread_cond_t  packerCondVar;
pthread_cond_t  deliveryCondVar;

// Function definitions
void init_mutexes_conditions();
void *pizzeria (void *args);
void destroy_mutexes_conditions();
void print_stats();

// Global variables 
int seed;
int n_cook      = N_COOK;
int n_oven      = N_OVEN;
int n_packer    = N_PACKER;
int n_deliverer = N_DELIVERER;

// Statistics initialized to 0
int totalIncome      = 0;  // Total Income of the store
int plainPizzas      = 0;  // Number of plain   Pizzas
int specialPizzas    = 0;  // Number of special Pizzas
int failedOrders     = 0;  // Number of failed orders
int successOrders    = 0;  // Number of successfull orders
int avgTimeServ	     = 0;  // Average Time for costumer service
int maxTimeServ	     = 0;  // Max Time for costumer service
int avgTimeCool	     = 0;  // Average Time for cool
int maxTimeCool	     = 0;  // Max Time for cool

int totalServingTime = 0;  // Sum of all serving times (Needed for avg calculation)
int totalCoolTime    = 0;  // Sum of all cool times (Needed for avg calculation)
int totalTime        = 0;  // packingTime + deliveryFinalTime