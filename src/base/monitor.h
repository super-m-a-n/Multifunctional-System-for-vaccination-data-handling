/* file: monitor.h */
#pragma once

typedef struct monitor * Monitor;

/* creates a monitor object */
Monitor monitor_create(unsigned int bloom_size, int max_level, float p);
/* destroys a monitor object and all of its components */
void monitor_destroy(Monitor monitor);
/* inserts given entry/line from file into all the necessary data structures of the monitor */
void monitor_insert(Monitor monitor, char * citizenID , char * firstName, char * lastName, char * country, unsigned int age, char * virusName, char * vacc, char * date);
/*prints all the data structures components of the monitor  (mainly for debugging) */ 
void monitor_print(Monitor monitor);

/*_____________________________________________________________*/

/* main utility functions of project*/

void vaccineStatusBloom(Monitor monitor, char * citizenID, char * virusName);
void vaccineStatus(Monitor monitor, char * citizenID, char * virusName);
void populationStatus(Monitor monitor, char * country, char * virusName, char * date1, char * date2);
void popStatusByAge(Monitor monitor, char * country, char * virusName, char * date1, char * date2);
void insertCitizenRecord(Monitor monitor, char * citizenID, char * firstName, char * lastName, char * country, int age, char * virusName, char * vacc, char * date);
void vaccinateNow(Monitor monitor, char * citizenID, char * firstName, char * lastName, char * country, int age, char * virusName);
void list_nonVaccinated_Persons(Monitor monitor, char * virusName);
void exit_monitor(Monitor monitor);

