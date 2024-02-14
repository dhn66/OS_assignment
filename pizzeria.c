#include "p3160230-p3160265-pizzeria.h"

// Mutex and Condition variable initialization
void init_mutexes_conditions()
{
	// Cook mutex and condition variable
	pthread_mutex_init(&cookMutex  ,NULL);
	pthread_cond_init (&cookCondVar,NULL);
	
	// Oven mutex and condition variable
	pthread_mutex_init(&ovenMutex  ,NULL);
	pthread_cond_init (&ovenCondVar,NULL);
	
	// Packer mutex and condition variable
	pthread_mutex_init(&packerMutex  ,NULL);
	pthread_cond_init (&packerCondVar,NULL);
	
	// Delivery mutex and condition variable
	pthread_mutex_init(&deliveryMutex  ,NULL);
	pthread_cond_init (&deliveryCondVar,NULL);
	
	// Print and stats mutexes
	pthread_mutex_init(&printMutex,NULL);
	pthread_mutex_init(&statsMutex,NULL);
}

// Mutex and condition variable destruction
void destroy_mutexes_conditions()
{
	// Destroy mutexes 
	pthread_mutex_destroy(&cookMutex);
	pthread_mutex_destroy(&ovenMutex);
	pthread_mutex_destroy(&packerMutex);
	pthread_mutex_destroy(&deliveryMutex);
	pthread_mutex_destroy(&printMutex);
	pthread_mutex_destroy(&statsMutex);

	// Destroy condition variables
	pthread_cond_destroy(&cookCondVar);
	pthread_cond_destroy(&ovenCondVar);
	pthread_cond_destroy(&packerCondVar);
	pthread_cond_destroy(&deliveryCondVar);
}

// print stats 
void print_stats()
{
	printf("\n\t\t\t -----Pizzeria stats----- \n\n");

	printf("\tTotal income from sales: %d\n",totalIncome);
	printf("\tNumber of pizzas sold: %d | plain: %d | special: %d\n"
		,plainPizzas+specialPizzas,plainPizzas,specialPizzas);
	printf("\tSuccessful orders: %d | Failed orders: %d\n",successOrders,failedOrders);

	avgTimeServ = totalServingTime / successOrders;
	printf("\t\t-----Customer service times----- \n");
	printf("\tAverage time: %d minutes | Max time: %d minutes\n", avgTimeServ,maxTimeServ);
	
	avgTimeCool = totalCoolTime / successOrders;
	printf("\t\t-----Pizza cooling times----- \n");
	printf("\tAverage time: %d minutes | Max time: %d minutes\n",avgTimeCool,maxTimeCool);
}


// Function that will be executed by every thread
void *pizzeria (void *args)
{
	int threadId = *(int *)args;   	// threadId function argument 
	int rc;
	int packingTime 		= 0;
	int deliveryFinalTime   = 0;
	int coolTime 			= 0;
	
	// Timespec struct definitions
	struct timespec orderTime;
	struct timespec startTime;
	struct timespec endTime;

	struct timespec coolStartTime;
	struct timespec coolEndTime;

	// Customer connection to the online ordering system.
	clock_gettime(CLOCK_REALTIME, &startTime);
	
	if (threadId == 1){ // First costumer start at present time
		clock_gettime(CLOCK_REALTIME, &orderTime);
	} 
	// Other costumers wait for their order(threadId != 1)
	else {
		sleep(rand_r(&seed) % T_ORDERHIGH + T_ORDERLOW);	
		clock_gettime(CLOCK_REALTIME, &orderTime);
	}

	// Add order time to packing time
	packingTime += (orderTime.tv_sec - startTime.tv_sec) ;

	// Random number of pizzas ordered by customer
	int numberOfPizzas = rand_r(&seed) % N_ORDERHIGH + N_ORDERLOW;

	// Payment time 
	rc = pthread_mutex_lock(&statsMutex);
		clock_gettime(CLOCK_REALTIME, &startTime);

		sleep(rand_r(&seed) % T_PAYMENTHIGH + T_PAYMENTLOW);

		clock_gettime(CLOCK_REALTIME, &endTime);

		packingTime += (endTime.tv_sec - startTime.tv_sec);
	rc = pthread_mutex_unlock(&statsMutex);

	int randNum = rand_r(&seed) % 100 + 1; // random number between 1 and 100

	// Successful order (90% of the times)
	if (randNum > P_FAIL) 
	{
		rc = pthread_mutex_lock(&printMutex);
			printf("Order with id %d is successful\n",threadId);
		rc = pthread_mutex_unlock(&printMutex);
	
		if (randNum > P_PLAIN) // Special pizza
		{
			rc = pthread_mutex_lock(&statsMutex);
				specialPizzas += numberOfPizzas;
				totalIncome   += C_SPECIAL * numberOfPizzas;
				successOrders += 1;
			rc = pthread_mutex_unlock(&statsMutex);
		}
		else // Plain pizza
		{
			rc = pthread_mutex_lock(&statsMutex);
				plainPizzas   += numberOfPizzas;
				totalIncome   += C_PLAIN * numberOfPizzas;
				successOrders += 1;
			rc = pthread_mutex_unlock(&statsMutex);
		}
	}

	else	// Failed order(10% of the times)
	{ 	
		rc = pthread_mutex_lock(&printMutex);
			printf("Order with id %d failed\n",threadId);
			failedOrders += 1;
		rc = pthread_mutex_unlock(&printMutex);
		pthread_exit(NULL); // self destruct the thread
	}

	// Cook logic
	rc = pthread_mutex_lock(&cookMutex);
		clock_gettime(CLOCK_REALTIME, &startTime);

		while (n_cook == 0) {
			printf("Cook is not available\n");
			rc = pthread_cond_wait(&cookCondVar, &cookMutex);
		}

		n_cook -= 1; 		// Cooks available
	rc = pthread_mutex_unlock(&cookMutex);

	// Time spend for the pizzas preparation
	rc = pthread_mutex_lock(&statsMutex);
		sleep(T_PREP);
		clock_gettime(CLOCK_REALTIME, &endTime);
		
		// Add cooking time to packing time
		packingTime += (endTime.tv_sec - startTime.tv_sec);
	rc = pthread_mutex_unlock(&statsMutex);

	// Oven logic
	rc = pthread_mutex_lock(&ovenMutex);

		clock_gettime(CLOCK_REALTIME, &startTime);

		while(n_oven == 0){
			printf("Oven is not available\n");
			rc = pthread_cond_wait(&ovenCondVar,&ovenMutex);
		}

		n_oven -= 1; // 1 oven is no more available
		n_cook += 1; // 1 cook is now available

		/// Start of pizza cool time
		clock_gettime(CLOCK_REALTIME, &coolStartTime);

		// Signal threads that cooks are now available
		rc = pthread_cond_signal(&cookCondVar);
	
	rc = pthread_mutex_unlock(&ovenMutex);	

	// Time spend for the pizzas baking
	rc = pthread_mutex_lock(&statsMutex);
		sleep(T_BAKE);
		clock_gettime(CLOCK_REALTIME, &endTime);

		// Add baking time to packingTime
		packingTime += (endTime.tv_sec - startTime.tv_sec);

	rc = pthread_mutex_unlock(&statsMutex);

	// Packer logic
	rc = pthread_mutex_lock(&packerMutex);
		
		clock_gettime(CLOCK_REALTIME, &startTime);

		while(n_packer == 0){
			printf("Packer is not available\n");
			rc = pthread_cond_wait(&packerCondVar,&packerMutex);
		}

		n_packer -= 1; // 1 packer is no more available
		n_oven 	 += 1; // 1 oven is available

		// Signal threads that ovens are now available
		rc = pthread_cond_signal(&ovenCondVar);

	rc = pthread_mutex_unlock(&packerMutex);

	// Time spend for the pizzas packing
	rc = pthread_mutex_lock(&statsMutex);
		sleep(T_PACK); // T_PACK time for every pizza ordered
		n_packer 	+= 1;  			     // A packer is now available

		clock_gettime(CLOCK_REALTIME, &endTime);

		packingTime += (endTime.tv_sec - startTime.tv_sec);

		printf("Order with id %d was prepared in %d minutes \n",threadId,packingTime );

		// Signal threads that packers are now available
		rc = pthread_cond_signal(&packerCondVar);
	rc = pthread_mutex_unlock(&statsMutex);

	// Delivery logic
	rc = pthread_mutex_lock(&deliveryMutex);

		clock_gettime(CLOCK_REALTIME, &startTime);

		while(n_deliverer == 0){
			printf("Delivery is not available\n");
			rc = pthread_cond_wait(&deliveryCondVar,&deliveryMutex);
			
		}

		n_deliverer -= 1; // 1 delivery is no more available

		// Random time needed for the delivery
		int deliveryTime = rand_r(&seed) % T_DELHIGH + T_DELLOW;
		sleep(deliveryTime);

		clock_gettime(CLOCK_REALTIME, &endTime);

		deliveryFinalTime += (endTime.tv_sec - startTime.tv_sec);

		/// End of pizza cool time
		clock_gettime(CLOCK_REALTIME, &coolEndTime);
		coolTime = (coolEndTime.tv_sec - coolStartTime.tv_sec);


			rc = pthread_mutex_lock(&printMutex);
				printf("Order with id %d was deliverd in %d minutes\n",threadId,deliveryFinalTime);
			rc = pthread_mutex_unlock(&printMutex);
	rc = pthread_mutex_unlock(&deliveryMutex);


	// Find maxTimeServ and maxTimeCool
	rc = pthread_mutex_lock(&statsMutex);
		
		// Maximum time for serving a costumer
		totalTime = packingTime + deliveryFinalTime;
		totalServingTime += totalTime; // ALl times for all costumers

		if( totalTime > maxTimeServ)
		{
			maxTimeServ = totalTime;
		}

		// Maximum time for pizza to cool
		totalCoolTime += coolTime;
		if (coolTime > maxTimeCool)
		{
			maxTimeCool = coolTime;
		}

	rc = pthread_mutex_unlock(&statsMutex);

	rc = pthread_mutex_lock(&deliveryMutex);
		n_deliverer += 1;
		// Signal the threads that deliverys are available
		rc = pthread_cond_signal(&deliveryCondVar);
	rc = pthread_mutex_unlock(&deliveryMutex);

	// Self destruct the thread
	pthread_exit(NULL);
}

int main(int argc, char const *argv[])
{
	if (argc != 3) {
		printf("Please provide only two args: [Customer Number] [seed]\n");
		exit(-1);
	}
	
	// Convert argv[1] from string to integer
	int N_cust = atoi(argv[1]); // Number of Customers

	if (N_cust <= 0) {
		printf("Please provide a positive int in argv[1] \n");
		exit(-1);
	}

    // Convert argv[2] from string to integer
	seed = atoi(argv[2]); 	// Seed for random number generations 

	int rc;	
	pthread_t threads[N_cust]; // Threads array
	int threadsId[N_cust];

	// Init mutexes and condition variables
	init_mutexes_conditions();

	for (int i = 0; i < N_cust; ++i)
	{
		threadsId[i] = i + 1; // ThreadsId passed as an argument
		
		// Threads creation function
		rc = pthread_create(&threads[i], NULL, pizzeria, &threadsId[i]);
		
		// If rc is different then 0 , an error has occured
    	if (rc != 0) {
    		printf("ERROR ! ,in pthread_create()\n");
       		exit(-1);
       	}
	}

	for (int i = 0; i < N_cust; ++i) 
	{
		// Threads join
		rc = pthread_join(threads[i], NULL);
		
		// If rc is different then 0 , an error has occured
		if (rc != 0) {
			printf("ERROR ! ,in pthread_join()\n");
			exit(-1);		
		}
	}

	// Print program statistics
	print_stats();

	// Destroy all mutexes and condition variables
	destroy_mutexes_conditions();

	return 0;
}
