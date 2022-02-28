# producer-consumer
Synchronization and mutual exclusion in a multiple producer-consumer problem.
This solution eliminates any deadlocks between the producer and consumer threads of the master process and also prevents starvation of consumer threads. 

Modify the config file "config.h" to change the number of producers, consumers and total messages shared.

To compile (link pthread library):
> gcc -lpthread main.c
