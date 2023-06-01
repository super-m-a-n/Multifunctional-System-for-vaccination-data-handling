/* file : vaccineMonitor.c*/
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "monitor.h"
#include <string.h>
#include <time.h>

int main(int argc, char const *argv[])
{
	/*check for correct arg input from terminal*/
	if (argc != 5)
	{
		fprintf(stderr, "Error: wrong number of args\nUse: ./vaccineMonitor -c citizenRecordsFile -b bloomSize\n");
		exit(EXIT_FAILURE);
	}

	if (strcmp(argv[1], "-c") != 0 || strcmp(argv[3], "-b") != 0)
	{
		fprintf(stderr, "Error: one or more wrong input parameters\n Use : -c -b\n");
		exit(EXIT_FAILURE);
	}

	unsigned int bloom_size = atoi(argv[4]);
	if (!bloom_size)
	{
		fprintf(stderr, "Error: invalid input parameter bloomSize\n Use : positive integer\n");
		exit(EXIT_FAILURE);
	}

	srand((unsigned int)time(NULL));

	char * line = NULL;
	char * citizenID , * firstName, * lastName, * country, * virusName, * vacc, * date;
	int age;
    size_t length = 0;

    Monitor vaccine_monitor = monitor_create(bloom_size, 8, 0.5);
    printf("\nInitializing monitor\n");
    printf("Inserting input file data into monitor\n\n");

    FILE *file_ptr;
	file_ptr = fopen(argv[2], "r");  /*open citizen records txt file , in read mode*/
	if (file_ptr == NULL)
	{
	    fprintf(stderr, "Error: main->fopen, could not open file\n");
	    exit(EXIT_FAILURE);
	}
	else    /*following block of code reads from the file and inserts the entries of file*/
	{
	  	while(getline(&line, &length, file_ptr) != -1)
	    {
	    	date = NULL;
	    	line[strlen(line)-1] = '\0';		// remove newline character from line read from file
	      	char *str = strtok(line, " ");
	      	int i = 1;
	      	while(str != NULL)
	      	{
	         	switch (i)
	         	{
	         		case 1: citizenID = str; break;
	         		case 2: firstName = str; break;
	         		case 3: lastName = str; break;
	         		case 4: country = str; break;
	         		case 5: age = atoi(str); break;
	         		case 6: virusName = str; break;
	         		case 7: vacc = str; break;
	         		case 8: date = str; break;
	         	}

	         	// check if citizenID is a duplicate (same citizenID and virusName)
	 			// if that's the case, discard the record
	         	
	         	//printf("%s ", str);
	         	i++;
	         	str = strtok(NULL, " ");
	      	}
	      	//printf("\n");
	      	//printf("Inserting Record into the monitor\n");
	      	monitor_insert(vaccine_monitor, citizenID, firstName, lastName, country, age, virusName, vacc, date);
	     
	     }

	    free(line);
	}
	
	//monitor_print(vaccine_monitor);

	bool exit = false;
	char input[100];
	while (exit == false)
	{
		printf("Waiting for command/task >>  ");
		fgets(input, 100, stdin);
		if (!strcmp(input, "\n"))
			continue;
		input[strlen(input)-1] = '\0';		// remove newline character from line read from command line

		if (!strcmp(input, "/exit"))
		{
			exit_monitor(vaccine_monitor);
			exit = true;
		}
		else
		{
			char *str = strtok(input, " ");
	      	if (!strcmp(str, "/vaccineStatusBloom"))
	      	{
	      		int i = 0;
	      		while(str != NULL)
	      		{
	         		switch (i)
	         		{
	         			case 1: citizenID = str; break;
	         			case 2: virusName = str; break;
	         		}

	         		i++;
	         		str = strtok(NULL, " ");
	      		}

	      		if (i != 3)
	      			printf("Error : unknown or invalid command\n\n");
	      		else
	      			vaccineStatusBloom(vaccine_monitor, citizenID, virusName);
	      	}

	      	else if (!strcmp(str, "/vaccineStatus"))
	      	{
	      		int i = 0;
	      		virusName = NULL;
	      		while(str != NULL)
	      		{
	         		switch (i)
	         		{
	         			case 1: citizenID = str; break;
	         			case 2: virusName = str; break;
	         		}

	         		i++;
	         		str = strtok(NULL, " ");
	      		}

	      		if (i != 3 && i != 2)
	      			printf("Error : unknown or invalid command\n\n");
	      		else	  
	      			vaccineStatus(vaccine_monitor, citizenID, virusName);
	      	}

	      	else if (!strcmp(str, "/populationStatus"))
	      	{
	      		int i = 0;
	      		char * arg1, * arg2, * arg3, * arg4;
	      		while(str != NULL)
	      		{
	         		switch (i)
	         		{
	         			case 1: arg1 = str; break;
	         			case 2: arg2 = str; break;
	         			case 3: arg3 = str; break;
	         			case 4: arg4 = str; break;
	         		}
	         
	         		i++;
	         		str = strtok(NULL, " ");
	      		}

	      		if (i != 2 && i != 3 && i != 4 && i != 5)
	      			printf("Error : unknown or invalid command\n\n");
	      		else if (i == 2)
	      			populationStatus(vaccine_monitor, NULL, arg1, NULL, NULL);
	      		else if (i == 3)
	      			populationStatus(vaccine_monitor, arg1, arg2, NULL, NULL);
	      		else if (i == 4)
	      			populationStatus(vaccine_monitor, NULL, arg1, arg2, arg3);
	      		else if (i == 5)
	      			populationStatus(vaccine_monitor, arg1, arg2, arg3, arg4);

	      	}

	      	else if (!strcmp(str, "/popStatusByAge"))
	      	{
	      		int i = 0;
	      		char * arg1, * arg2, * arg3, * arg4;
	      		while(str != NULL)
	      		{
	         		switch (i)
	         		{
	         			case 1: arg1 = str; break;
	         			case 2: arg2 = str; break;
	         			case 3: arg3 = str; break;
	         			case 4: arg4 = str; break;
	         		}
	         
	         		i++;
	         		str = strtok(NULL, " ");
	      		}

	      		if (i != 2 && i != 3 && i != 4 && i != 5)
	      			printf("Error : unknown or invalid command\n\n");
	      		else if (i == 2)
	      			popStatusByAge(vaccine_monitor, NULL, arg1, NULL, NULL);
	      		else if (i == 3)
	      			popStatusByAge(vaccine_monitor, arg1, arg2, NULL, NULL);
	      		else if (i == 4)
	      			popStatusByAge(vaccine_monitor, NULL, arg1, arg2, arg3);
	      		else if (i == 5)
	      			popStatusByAge(vaccine_monitor, arg1, arg2, arg3, arg4);
	      	}

	      	else if (!strcmp(str, "/insertCitizenRecord"))
	      	{
	      		int i = 0;
	      		date = NULL;
	      		while(str != NULL)
	      		{
	         		switch (i)
	         		{
	         			case 1: citizenID = str; break;
	         			case 2: firstName = str; break;
	         			case 3: lastName = str; break;
	         			case 4: country = str; break;
	         			case 5: age = atoi(str); break;
	         			case 6: virusName = str; break;
	         			case 7: vacc = str; break;
	         			case 8: date = str; break;
	         		}
	         
	         		i++;
	         		str = strtok(NULL, " ");
	      		}

	      		if (i != 8 && i != 9)
	      			printf("Error : unknown or invalid command\n\n");
	      		else
	      			insertCitizenRecord(vaccine_monitor, citizenID, firstName, lastName, country, age, virusName, vacc, date);
	      	}

	      	else if (!strcmp(str, "/vaccinateNow"))
	      	{
	      		int i = 0;
	      		while(str != NULL)
	      		{
	         		switch (i)
	         		{
	         			case 1: citizenID = str; break;
	         			case 2: firstName = str; break;
	         			case 3: lastName = str; break;
	         			case 4: country = str; break;
	         			case 5: age = atoi(str); break;
	         			case 6: virusName = str; break;	         	
	         		}
	         
	         		i++;
	         		str = strtok(NULL, " ");
	      		}

	      		if (i != 7)
	      			printf("Error : unknown or invalid command\n\n");
	      		else
	      			vaccinateNow(vaccine_monitor, citizenID, firstName, lastName, country, age, virusName);
	      	}

	      	else if (!strcmp(str, "/list-nonVaccinated-Persons"))
	      	{
	      		int i = 0;	      
	      		while(str != NULL)
	      		{
	         		switch (i)
	         		{
	         			case 1: virusName = str; break;
	         		}

	         		i++;
	         		str = strtok(NULL, " ");
	      		}

	      		if (i != 2)
	      			printf("Error : unknown or invalid command\n\n");
	      		else	  
	      			list_nonVaccinated_Persons(vaccine_monitor, virusName);
	      	}

	      	else
	      		printf("Error : unknown or invalid command\n\n");
		}

	}


	fclose(file_ptr);
	return 0;
}