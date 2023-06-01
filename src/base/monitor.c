/* file : monitor.c */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "monitor.h"
#include "skip_list.h"
#include "bloom.h"
#include "hash.h"
#include "list.h"
#include "items.h"
#include "time.h"
#include <assert.h>

struct monitor {
	HT citizens_info;
	HT viruses_info;
	HT countries_info;
	unsigned int bloom_size;
	int max_level;
	float p;
};

Monitor monitor_create(unsigned int bloom_size, int max_level, float p)
{
	Monitor monitor = malloc(sizeof(struct monitor));
	if (monitor == NULL)
		fprintf(stderr, "Error : monitor_create -> malloc\n");
	assert(monitor != NULL);

	monitor->citizens_info = hash_create(100, 0);
	monitor->viruses_info = hash_create(10, 1);
	monitor->countries_info = hash_create(10, 2);

	monitor->bloom_size = bloom_size;
	monitor->max_level = max_level;
	monitor->p = p;

	return monitor;
}

void monitor_destroy(Monitor monitor)
{
	if (monitor == NULL)
		fprintf(stderr, "Error : monitor_destroy -> monitor is NULL\n");
	assert(monitor != NULL);

	hash_destroy(monitor->countries_info);
	hash_destroy(monitor->citizens_info);
	hash_destroy(monitor->viruses_info);

	free(monitor);
}

void monitor_insert(Monitor monitor, char * citizenID , char * firstName, char * lastName, char * country, unsigned int age, char * virusName, char * vacc, char * date)
{

	if (monitor == NULL)
		fprintf(stderr, "Error : monitor_insert -> monitor is NULL\n");
	assert(monitor != NULL);

	// search for an already existing citizen record with same ID
	CitizenInfo citizen_info = (CitizenInfo) hash_search(monitor->citizens_info, citizenID);
	// search for an already existing virus record with given name
	VirusInfo virus_info = (VirusInfo) hash_search(monitor->viruses_info, virusName);
	// search for an already existing country record with given name
	CountryInfo country_info = (CountryInfo) hash_search(monitor->countries_info, country);

	if (citizen_info != NULL)		// if a citizen record with same ID already exists
	{
		// check if new record is inconsistent
		if (strcmp(firstName, get_citizen_name(citizen_info)) != 0 || strcmp(lastName, get_citizen_surname(citizen_info)) != 0 
			|| strcmp(country, get_citizen_country(citizen_info)) != 0 || age != get_citizen_age(citizen_info))
		{
			printf("ERROR IN RECORD : %s %s %s %s %d %s %s ", citizenID, firstName, lastName, country, age, virusName, vacc); printf( (date == NULL) ? "\n" : "%s\n", date);
			printf("INCONSISTENT INPUT DATA\n\n");
			return;
		}

		if (virus_info != NULL)
		{
			char * temp_date;
			// check if new record is duplicate (same ID, but also same virus - that means, an entry with given ID already exists for given virus)
			// if it exists it is either on the vaccinated skip list or non vaccinated skip list for given virus
			if (skip_list_search(get_vacc_list(virus_info), citizenID, &temp_date) || skip_list_search(get_non_vacc_list(virus_info), citizenID, &temp_date))
			{
				printf("ERROR IN RECORD : %s %s %s %s %d %s %s ", citizenID, firstName, lastName, country, age, virusName, vacc); printf( (date == NULL) ? "\n" : "%s\n", date);
				printf("INPUT DATA DUPLICATION\n\n");
				return;
			}
		}
	}

	// at last, check for invalid data form, i.e. vaccinated == "YES" but no date is given or vaccinated = "NO" but a date is given
	if ( ( !strcmp(vacc, "YES") && date == NULL) || (!strcmp(vacc, "NO") && date != NULL) )
	{
		printf("ERROR IN RECORD : %s %s %s %s %d %s %s ", citizenID, firstName, lastName, country, age, virusName, vacc); printf( (date == NULL) ? "\n" : "%s\n", date);
		printf("INVALID INPUT DATA FORM\n\n");
		return;
	}

	if (country_info == NULL)
	{
		country_info = country_info_create(country);		// if given country name is new, create new country record
		hash_insert(monitor->countries_info, country_info);			// insert it into countries index for future reference
	}

	if (citizen_info == NULL)			// given record is a new citizen record (new ID)
	{
		citizen_info = citizen_info_create(citizenID, firstName, lastName, age, country_info);	// create new citizen record
		hash_insert(monitor->citizens_info, citizen_info);				// insert it into citizens index for future reference
	}

	if (virus_info == NULL)
	{
		virus_info = virus_info_create(virusName, monitor->bloom_size, monitor->max_level, monitor->p);
		hash_insert(monitor->viruses_info, virus_info);
	}

	// insert citizen into bloom filter, correct skip list, of given virus
	if (!strcmp(vacc, "YES"))
	{
		bloom_insert(get_bloom_filter(virus_info), (unsigned char*) citizenID);	// bloom filter of virus, keeps track of the vaccinated citizens
		skip_list_insert(get_vacc_list(virus_info), citizen_info, date);		// insert into vaccinated persons skip list if citizen was vaccinated
	}
	else
		skip_list_insert(get_non_vacc_list(virus_info), citizen_info, date);	// insert into not vaccinated skip list if citizen was not vaccinated
	
}

void monitor_print(Monitor monitor)
{
	if (monitor == NULL)
		fprintf(stderr, "Error : monitor_print -> monitor is NULL\n");
	assert(monitor != NULL);

	printf("Printing monitor components: \n\n");
	printf("Printing countries hash-index: \n\n");
	hash_print(monitor->countries_info);
	printf("\n\nPrinting citizens hash-index: \n\n");
	hash_print(monitor->citizens_info);
	printf("\n\nPrinting viruses hash-index: \n\n");
	hash_print(monitor->viruses_info);
}


/*_____________________________________________________________________________________________________________*/

/* main utility functions */

void vaccineStatusBloom(Monitor monitor, char * citizenID, char * virusName)
{
	if (monitor == NULL)
		fprintf(stderr, "Error : vaccineStatusBloom -> monitor is NULL\n");
	assert(monitor != NULL);

	// search for an existing virus record with given virus name
	VirusInfo virus_info = (VirusInfo) hash_search(monitor->viruses_info, virusName);

	if (virus_info == NULL)
	{
		fprintf(stderr, "Error : vaccineStatusBloom -> Given virus name does not exist in database\n\n");
		return;
	}

	// search for an existing cititzen record with given citizen ID
	CitizenInfo citizen_info = (CitizenInfo) hash_search(monitor->citizens_info, citizenID);

	if (citizen_info == NULL)
	{
		fprintf(stderr, "Error : vaccineStatusBloom -> Given citizen ID does not exist in database\n\n");
		return;
	}

	printf("\nChecking vaccine status of citizen with [ ID = %s ] for [ virus = %s ] \n", citizenID, virusName);

	if (bloom_check(get_bloom_filter(virus_info), (unsigned char *) citizenID))
		printf("MAYBE\n\n");			// bloom filter check returns true (maybe is in, maybe is not (false positive))
	else
		printf("NOT VACCINATED\n\n");	// bloom filter check returns false (definitely is not in)
}

void vaccineStatus(Monitor monitor, char * citizenID, char * virusName)
{
	if (monitor == NULL)
		fprintf(stderr, "Error : vaccineStatus -> monitor is NULL\n");
	assert(monitor != NULL);

	// search for an existing cititzen record with given citizen ID
	CitizenInfo citizen_info = (CitizenInfo) hash_search(monitor->citizens_info, citizenID);

	if (citizen_info == NULL)
	{
		fprintf(stderr, "Error : vaccineStatus -> Given citizen ID does not exist in database\n\n");
		return;
	}

	if (virusName != NULL)		// if a specific virus was given as argument
	{	// search for an existing virus record with given virus name
		VirusInfo virus_info = (VirusInfo) hash_search(monitor->viruses_info, virusName);

		if (virus_info == NULL)
		{
			fprintf(stderr, "Error : vaccineStatus -> Given virus name does not exist in database\n\n");
			return;
		}

		printf("\nChecking vaccine status of citizen with [ ID = %s ] for [ virus = %s ] \n", citizenID, virusName);

		char * date = NULL;

		if (!skip_list_search(get_vacc_list(virus_info), citizenID, &date))		// if citizen id was not found into vaccinated skip list for given virus
			printf("NOT VACCINATED\n\n");
		else
			printf("VACCINATED ON %s \n\n", date);
	}

	else
	{
		// no specific virus was given, so do the same search, but for every virus
		printf("\nChecking vaccine status of citizen with [ ID = %s ] for all associated viruses\n", citizenID);
		VirusInfo virus_info;
		// iterate upon the hash-table of viruses
		while ((virus_info = hash_iterate_next(monitor->viruses_info)) != NULL)
		{
			char * date = NULL;
			if (skip_list_search(get_vacc_list(virus_info), citizenID, &date))		// if citizen id was found into vaccinated skip list for given virus
				printf("%s YES %s\n", get_virus_name(virus_info), date);
			else if (skip_list_search(get_non_vacc_list(virus_info), citizenID, &date)) // if citizen id was found into not vaccinated list for given virus
				printf("%s NO\n", get_virus_name(virus_info));
			// if citizen is not associated with particular virus, then we dont print anything
		}
		printf("\n");
	}

}

void populationStatus(Monitor monitor, char * country, char * virusName, char * date1, char * date2)
{
	if (monitor == NULL)
		fprintf(stderr, "Error : populationStatus -> monitor is NULL\n");
	assert(monitor != NULL);

	// check for correct dates 
	if (date1 != NULL && date2 != NULL)
	{
		if (!date_check(date1) || !date_check(date2) || date_cmp(date1, date2) > 0 )
		{
			fprintf(stderr, "Error : populationStatus -> Invalid dates\n\n");
			return;
		}
	}

	if (date_check(virusName))
	{
		fprintf(stderr, "Error : populationStatus -> Invalid dates\n\n");
		return;
	}

	// search for an existing virus record with given virus name
	VirusInfo virus_info = (VirusInfo) hash_search(monitor->viruses_info, virusName);

	if (virus_info == NULL)
	{
		fprintf(stderr, "Error : populationStatus -> Given virus name does not exist in database\n\n");
		return;
	}

	if (country != NULL)
	{
		// search for an already existing country record with given name
		CountryInfo country_info = (CountryInfo) hash_search(monitor->countries_info, country);

		if (country_info == NULL)
		{	
			fprintf(stderr, "Error : populationStatus -> Given country name does not exist in database\n\n");
			return;
		}

		int num_of_vaccinated_in_range = skip_list_GroupByCountry(get_vacc_list(virus_info), country, date1, date2);	// get num of vaccinated people of country in given date range
		int num_of_vaccinated = skip_list_GroupByCountry(get_vacc_list(virus_info), country, NULL, NULL);				// get total num of vaccinated people of country
		int num_of_not_vaccinated = skip_list_GroupByCountry(get_non_vacc_list(virus_info), country, date1, date2);		// get total num of not vaccinated people of country
		if (num_of_vaccinated + num_of_not_vaccinated != 0)
		{	float percentage = 100 * (((float) num_of_vaccinated_in_range)/ (num_of_vaccinated + num_of_not_vaccinated));
			printf("\n%s %d %f%% \n\n", country, num_of_vaccinated_in_range, percentage);
		}
		else
			printf("\n%s %d 0%% \n\n", country, num_of_vaccinated_in_range);
	}	
	else
	{
		// no country argument was given, thus we will do the same but for all countries
		CountryInfo country_info;
		// iterate upon the hash-table of countries
		while ((country_info = (CountryInfo) hash_iterate_next(monitor->countries_info)) != NULL)
		{
			int num_of_vaccinated_in_range = skip_list_GroupByCountry(get_vacc_list(virus_info), get_country_name(country_info), date1, date2);			// get num of vaccinated people of country in given date range
			int num_of_vaccinated = skip_list_GroupByCountry(get_vacc_list(virus_info), get_country_name(country_info), NULL, NULL);					// get total num of vaccinated people of country
			int num_of_not_vaccinated = skip_list_GroupByCountry(get_non_vacc_list(virus_info), get_country_name(country_info), date1, date2);			// get total num of not vaccinated people of country
			if (num_of_vaccinated + num_of_not_vaccinated != 0)
			{	float percentage = 100 * (((float) num_of_vaccinated_in_range)/ (num_of_vaccinated + num_of_not_vaccinated));
				printf("\n%s %d %f%% \n", get_country_name(country_info), num_of_vaccinated_in_range, percentage);
			}
			else
				printf("\n%s %d 0%% \n", get_country_name(country_info), num_of_vaccinated_in_range);
		}
		printf("\n");
	}
}

void popStatusByAge(Monitor monitor, char * country, char * virusName, char * date1, char * date2)
{
	if (monitor == NULL)
		fprintf(stderr, "Error : popStatusByAge -> monitor is NULL\n");
	assert(monitor != NULL);

	// check for correct dates 
	if (date1 != NULL && date2 != NULL)
	{
		if (!date_check(date1) || !date_check(date2) || date_cmp(date1, date2) > 0 )
		{
			fprintf(stderr, "Error : popStatusByAge -> Invalid dates\n\n");
			return;
		}
	}

	if (date_check(virusName))
	{
		fprintf(stderr, "Error : popStatusByAge -> Invalid dates\n\n");
		return;
	}

	// search for an existing virus record with given virus name
	VirusInfo virus_info = (VirusInfo) hash_search(monitor->viruses_info, virusName);

	if (virus_info == NULL)
	{
		fprintf(stderr, "Error : popStatusByAge -> Given virus name does not exist in database\n\n");
		return;
	}

	if (country != NULL)
	{
		// search for an already existing country record with given name
		CountryInfo country_info = (CountryInfo) hash_search(monitor->countries_info, country);

		if (country_info == NULL)
		{	
			fprintf(stderr, "Error : popStatusByAge -> Given country name does not exist in database\n\n");
			return;
		}

		int vacc_20, non_vacc_20, vacc_40, non_vacc_40, vacc_60, non_vacc_60, vacc_older, non_vacc_older;		// total vaccinated/not vaccinated counters
		int vacc_20_in_range, vacc_40_in_range, vacc_60_in_range, vacc_older_in_range;							// counters refering to the vaccinated in given date range
		
		skip_list_GroupByAge(get_vacc_list(virus_info), country, date1, date2, &vacc_20_in_range, &vacc_40_in_range, &vacc_60_in_range, &vacc_older_in_range);
		skip_list_GroupByAge(get_vacc_list(virus_info), country, NULL, NULL, &vacc_20, &vacc_40, &vacc_60, &vacc_older);
		skip_list_GroupByAge(get_non_vacc_list(virus_info), country, date1, date2, &non_vacc_20, &non_vacc_40, &non_vacc_60, &non_vacc_older);
		
		printf("\n%s\n", country);
		if (vacc_20 + non_vacc_20 != 0)
		{	float percentage = 100 * (((float) vacc_20_in_range)/ (vacc_20 + non_vacc_20));
			printf("0-20 %d %f%% \n", vacc_20_in_range, percentage);
		}
		else
			printf("0-20 %d 0%% \n", vacc_20_in_range);
		
		if (vacc_40 + non_vacc_40 != 0)
		{	float percentage = 100 * (((float) vacc_40_in_range)/ (vacc_40 + non_vacc_40));
			printf("20-40 %d %f%% \n", vacc_40_in_range, percentage);
		}
		else
			printf("20-40 %d 0%% \n", vacc_40_in_range);
		
		if (vacc_60 + non_vacc_60 != 0)
		{	float percentage = 100 * (((float) vacc_60_in_range)/ (vacc_60 + non_vacc_60));
			printf("40-60 %d %f%% \n", vacc_60_in_range, percentage);
		}
		else
			printf("40-60 %d 0%% \n", vacc_60_in_range);
		
		if (vacc_older + non_vacc_older != 0)
		{	float percentage = 100 * (((float) vacc_older_in_range)/ (vacc_older + non_vacc_older));
			printf("60+ %d %f%% \n\n", vacc_older_in_range, percentage);
		}
		else
			printf("60+ %d 0%% \n\n", vacc_older_in_range);
	}	
	else
	{
		// no country argument was given, thus we will do the same but for all countries
		CountryInfo country_info;
		// iterate upon the hash-table of countries
		while ((country_info = (CountryInfo) hash_iterate_next(monitor->countries_info)) != NULL)
		{
			int vacc_20, non_vacc_20, vacc_40, non_vacc_40, vacc_60, non_vacc_60, vacc_older, non_vacc_older;		// total vaccinated/not vaccinated counters
			int vacc_20_in_range, vacc_40_in_range, vacc_60_in_range, vacc_older_in_range;							// counters refering to the vaccinated in given date range

			skip_list_GroupByAge(get_vacc_list(virus_info), get_country_name(country_info), date1, date2, &vacc_20_in_range, &vacc_40_in_range, &vacc_60_in_range, &vacc_older_in_range);
			skip_list_GroupByAge(get_vacc_list(virus_info), get_country_name(country_info), NULL, NULL, &vacc_20, &vacc_40, &vacc_60, &vacc_older);
			skip_list_GroupByAge(get_non_vacc_list(virus_info), get_country_name(country_info), date1, date2, &non_vacc_20, &non_vacc_40, &non_vacc_60, &non_vacc_older);
			
			printf("%s\n", get_country_name(country_info));
			if (vacc_20 + non_vacc_20 != 0)
			{	float percentage = 100 * (((float) vacc_20_in_range)/ (vacc_20 + non_vacc_20));
				printf("0-20 %d %f%% \n", vacc_20_in_range, percentage);
			}
			else
				printf("0-20 %d 0%% \n", vacc_20_in_range);
			
			if (vacc_40 + non_vacc_40 != 0)
			{	float percentage = 100 * (((float) vacc_40_in_range)/ (vacc_40 + non_vacc_40));
				printf("20-40 %d %f%% \n", vacc_40_in_range, percentage);
			}
			else
				printf("20-40 %d 0%% \n", vacc_40_in_range);
			
			if (vacc_60 + non_vacc_60 != 0)
			{	float percentage = 100 * (((float) vacc_60_in_range)/ (vacc_60 + non_vacc_60));
				printf("40-60 %d %f%% \n", vacc_60_in_range, percentage);
			}
			else
				printf("40-60 %d 0%% \n", vacc_60_in_range);
			
			if (vacc_older + non_vacc_older != 0)
			{	float percentage = 100 * (((float) vacc_older_in_range)/ (vacc_older + non_vacc_older));
				printf("60+ %d %f%% \n\n", vacc_older_in_range, percentage);
			}
			else
				printf("60+ %d 0%% \n\n", vacc_older_in_range);
		}
		printf("\n");
	}
}

void insertCitizenRecord(Monitor monitor, char * citizenID, char * firstName, char * lastName, char * country, int age, char * virusName, char * vacc, char * date)
{
	if (monitor == NULL)
		fprintf(stderr, "Error : insertCitizenRecord -> monitor is NULL\n");
	assert(monitor != NULL);

	// at first, check for invalid data form, i.e. vaccinated == "YES" but no date is given or vaccinated = "NO" but a date is given
	if ( ( !strcmp(vacc, "YES") && date == NULL) || (!strcmp(vacc, "NO") && date != NULL) )
	{
		printf("Error : insertCitizenRecord -> invalid input data form in given record : %s %s %s %s %d %s %s ", citizenID, firstName, lastName, country, age, virusName, vacc);
		printf( (date == NULL) ? "\n\n" : "%s\n\n", date);
		return;
	}

	// check for correct date
	if (date != NULL)
	{	if (!date_check(date))
		{
			fprintf(stderr, "Error : insertCitizenRecord -> Invalid date\n\n");
			return;
		}
	}

	// search for an already existing citizen record with same ID
	CitizenInfo citizen_info = (CitizenInfo) hash_search(monitor->citizens_info, citizenID);
	// search for an already existing virus record with given name
	VirusInfo virus_info = (VirusInfo) hash_search(monitor->viruses_info, virusName);
	// search for an already existing country record with given name
	CountryInfo country_info = (CountryInfo) hash_search(monitor->countries_info, country);

	if (citizen_info != NULL)		// if a citizen record with same ID already exists
	{
		// check if new record is inconsistent
		if (strcmp(firstName, get_citizen_name(citizen_info)) != 0 || strcmp(lastName, get_citizen_surname(citizen_info)) != 0 
			|| strcmp(country, get_citizen_country(citizen_info)) != 0 || age != get_citizen_age(citizen_info))
		{
			printf("Error : insertCitizenRecord -> inconsistent input data in given record : %s %s %s %s %d %s %s ", citizenID, firstName, lastName, country, age, virusName, vacc);
			printf( (date == NULL) ? "\n\n" : "%s\n\n", date);
			return;
		}

		if (virus_info != NULL)
		{
			char * temp_date;
			// check if new record is duplicate (same ID, but also same virus - that means, an entry with given ID already exists for given virus)
			// if it exists it is either on the vaccinated skip list or non vaccinated skip list for given virus
			if (skip_list_search(get_vacc_list(virus_info), citizenID, &temp_date))
			{
				printf("Error : insertCitizenRecord -> CITIZEN %s ALREADY VACCINATED ON %s\n\n", citizenID, temp_date);
				return;
			} 

			if (skip_list_search(get_non_vacc_list(virus_info), citizenID, &temp_date))
			{
				printf("Error : insertCitizenRecord -> CITIZEN %s ALREADY IN THE NOT-VACCINATED LIST\n", citizenID);
				printf("In case you want to vaccinate the citizen, use /vaccinateNow\n\n");
				return;
			}
		}
	}

	if (country_info == NULL)
	{
		country_info = country_info_create(country);		// if given country name is new, create new country record
		hash_insert(monitor->countries_info, country_info);			// insert it into countries index for future reference
	}

	// given record is a new citizen record (new ID)
	// first of all do a small check for valid citizen ID
	for (int i = 0; i < strlen(citizenID); ++i)
	{
		if (citizenID[i] < '0' || citizenID[i] > '9')
		{
			printf("Error : vaccinateNow -> given citizen ID is not a string of digits\n\n");
			return;
		}
	}

	citizen_info = citizen_info_create(citizenID, firstName, lastName, age, country_info);		// create new citizen record
	hash_insert(monitor->citizens_info, citizen_info);				// insert it into citizens index for future reference
	
	if (virus_info == NULL)
	{
		virus_info = virus_info_create(virusName, monitor->bloom_size, monitor->max_level, monitor->p);
		hash_insert(monitor->viruses_info, virus_info);
	}

	// insert citizen into bloom filter, correct skip list, of given virus
	if (!strcmp(vacc, "YES"))
	{
		bloom_insert(get_bloom_filter(virus_info), (unsigned char*) citizenID);	// bloom filter of virus, keeps track of the vaccinated citizens
		skip_list_insert(get_vacc_list(virus_info), citizen_info, date);		// insert into vaccinated persons skip list if citizen was vaccinated
	}
	else
		skip_list_insert(get_non_vacc_list(virus_info), citizen_info, date);	// insert into not vaccinated skip list if citizen was not vaccinated

	printf("Inserted record for citizen with [ ID = %s ] \n\n", citizenID);
}

void vaccinateNow(Monitor monitor, char * citizenID, char * firstName, char * lastName, char * country, int age, char * virusName)
{
	if (monitor == NULL)
		fprintf(stderr, "Error : vaccinateNow -> monitor is NULL\n");
	assert(monitor != NULL);

	// search for an already existing citizen record with same ID
	CitizenInfo citizen_info = (CitizenInfo) hash_search(monitor->citizens_info, citizenID);
	// search for an already existing virus record with given name
	VirusInfo virus_info = (VirusInfo) hash_search(monitor->viruses_info, virusName);
	// search for an already existing country record with given name
	CountryInfo country_info = (CountryInfo) hash_search(monitor->countries_info, country);

	// create todays date
	time_t t = time(NULL); 
	struct tm tm = *localtime(&t);
  	int year = tm.tm_year + 1900;	int month = tm.tm_mon + 1;	int day = tm.tm_mday;
  	char day_str[3]; char month_str[3]; char year_str[5];	char todays_date[12] = "";
  	sprintf(day_str, "%d", day);	sprintf(month_str, "%d", month);	sprintf(year_str, "%d", year);
  	strcat(todays_date, day_str);
  	strcat(todays_date, "-");
  	strcat(todays_date, month_str);
  	strcat(todays_date, "-");
  	strcat(todays_date, year_str);

	if (virus_info == NULL)
	{
		fprintf(stderr, "Error : vaccineStatus -> Given virus name does not exist in database\n\n");
		return;
	}

	if (citizen_info != NULL)		// if a citizen record with same ID already exists
	{
		// check if new record is inconsistent
		if (strcmp(firstName, get_citizen_name(citizen_info)) != 0 || strcmp(lastName, get_citizen_surname(citizen_info)) != 0 
			|| strcmp(country, get_citizen_country(citizen_info)) != 0 || age != get_citizen_age(citizen_info))
		{
			printf("Error : vaccinateNow -> inconsistent input data in given record : %s %s %s %s %d %s \n\n", citizenID, firstName, lastName, country, age, virusName);
			return;
		}

		char * date;
		if (skip_list_search(get_vacc_list(virus_info), citizenID, &date))		// citizen with given ID is already vaccinated for given virus
		{
			printf("Error : vaccinateNow -> CITIZEN %s ALREADY VACCINATED ON %s\n\n", citizenID, date);
			return;
		}

		if (skip_list_search(get_non_vacc_list(virus_info), citizenID, &date)) // citizen is not vaccinated for given virus, but is on not-vaccinated list
			skip_list_delete(get_non_vacc_list(virus_info), citizenID);    // remove citizen with given ID from not-vaccinated skip list for virus

		bloom_insert(get_bloom_filter(virus_info), (unsigned char*) citizenID);		// insert into bloom filter of virus
		skip_list_insert(get_vacc_list(virus_info), citizen_info, todays_date);		// insert into vaccinated persons skip list of virus, with today's date
		printf("\nVaccinated citizen with [ ID = %s ] for [ virus = %s ] \n\n", citizenID, virusName);
		return;
	}

	// given ID does not already exist, so create new citizen record
	// first of all do a small check for valid citizen ID
	for (int i = 0; i < strlen(citizenID); ++i)
	{
		if (citizenID[i] < '0' || citizenID[i] > '9')
		{
			printf("Error : vaccinateNow -> given citizen ID is not a string of digits\n\n");
			return;
		}
	}

	if (country_info == NULL)
	{
		country_info = country_info_create(country);		// if given country name is new, create new country record
		hash_insert(monitor->countries_info, country_info);			// insert it into countries index for future reference
	}

	citizen_info = citizen_info_create(citizenID, firstName, lastName, age, country_info);	// create new citizen record
	hash_insert(monitor->citizens_info, citizen_info);				// insert it into citizens index for future reference
	
	bloom_insert(get_bloom_filter(virus_info), (unsigned char*) citizenID);		// insert into bloom filter of virus
	skip_list_insert(get_vacc_list(virus_info), citizen_info, todays_date);		// insert into vaccinated persons skip list of virus, with today's date
	printf("\nVaccinated citizen with [ ID = %s ] for [ virus = %s ] \n\n", citizenID, virusName);
}

void list_nonVaccinated_Persons(Monitor monitor, char * virusName)
{
	if (monitor == NULL)
		fprintf(stderr, "Error : list_nonVaccinated_Persons -> monitor is NULL\n");
	assert(monitor != NULL);

	// search for an existing virus record with given virus name
	VirusInfo virus_info = (VirusInfo) hash_search(monitor->viruses_info, virusName);

	if (virus_info == NULL)
	{
		fprintf(stderr, "Error : list_nonVaccinated_Persons -> Given virus name does not exist in database\n\n");
		return;
	}

	printf("\nPrinting citizen records of not-vaccinated citizens for [ virus = %s ] \n", virusName);
	skip_list_print_data(get_non_vacc_list(virus_info));		// just print all citizen records found on not vaccinated skip list
}

void exit_monitor(Monitor monitor)
{
	if (monitor == NULL)
		fprintf(stderr, "Error : exit_monitor -> monitor is NULL\n");
	assert(monitor != NULL);
	monitor_destroy(monitor);	
}