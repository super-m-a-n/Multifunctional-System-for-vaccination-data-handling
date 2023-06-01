/* file : items.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bloom.h"
#include "skip_list.h"
#include "items.h"
#include <assert.h>

struct citizen_info {
	char * id;
	char * name;
	char * surname;
	int age;
	CountryInfo country;
};

struct virus_info {
	char * virus_name;						// name of the virus
	Bloom bloom_filter;						// bloom filter for virus
	SkipList vaccinated_persons;			// vaccinated persons skip list for virus
	SkipList not_vaccinated_persons;		// not vaccinated persons skip list for virus
};

struct country_info {
	char * country_name;
	unsigned long population;
};

CitizenInfo citizen_info_create(char * id, char * name, char * surname, int age, CountryInfo country)
{
	CitizenInfo info = malloc(sizeof(struct citizen_info));
	if (info == NULL)
		fprintf(stderr, "Error : citizen_info_create -> malloc\n");
	assert(info != NULL);

	info->id = malloc(strlen(id) + 1);
	strcpy(info->id, id);
	info->name = malloc(strlen(name) + 1);
	strcpy(info->name, name);
	info->surname = malloc(strlen(surname) + 1);
	strcpy(info->surname, surname);
	info->age = age;
	info->country = country;
	country_population_inc(country);		// new citizen from given country was recorded and inserted into database

	return info;
}

void citizen_info_destroy(CitizenInfo info)
{
	if (info == NULL)
		fprintf(stderr, "Error : citizen_info_delete -> malloc\n");
	assert(info != NULL);

	free(info->id);
	free(info->name);
	free(info->surname);
	free(info);
}

char * get_citizen_id(CitizenInfo info)
{
	return info->id;
}

char * get_citizen_name(CitizenInfo info)
{
	return info->name;
}

char * get_citizen_surname(CitizenInfo info)
{
	return info->surname;
}

char * get_citizen_country(CitizenInfo info)
{
	return get_country_name(info->country);
}

int get_citizen_age(CitizenInfo info)
{
	return info->age;
}

void citizen_info_print(CitizenInfo info)
{
	printf("%s %s %s %s %d\n", info->id, info->name, info->surname, get_country_name(info->country), info->age);
}

/*_______________________________________________________________________________________________________________*/


VirusInfo virus_info_create(char * virus_name, unsigned int bloom_size, int max_level, float p)
{
	VirusInfo info = malloc(sizeof(struct virus_info));
	if (info == NULL)
		fprintf(stderr, "Error : virus_info_create -> malloc\n");
	assert(info != NULL);

	info->virus_name = malloc(strlen(virus_name) + 1);
	strcpy(info->virus_name, virus_name);

	info->bloom_filter = bloom_create(bloom_size);
	info->vaccinated_persons = skip_list_create(max_level, p);
	info->not_vaccinated_persons = skip_list_create(max_level, p);

	return info;
}

void virus_info_destroy(VirusInfo info)
{
	if (info == NULL)
		fprintf(stderr, "Error : virus_info_delete -> malloc\n");
	assert(info != NULL);

	free(info->virus_name);
	bloom_destroy(info->bloom_filter);
	skip_list_destroy(info->vaccinated_persons);
	skip_list_destroy(info->not_vaccinated_persons);

	free(info);
}

char * get_virus_name(VirusInfo info)
{
	return info->virus_name;
}

Bloom get_bloom_filter(VirusInfo info)
{
	return info->bloom_filter;
}

SkipList get_vacc_list(VirusInfo info)
{
	return info->vaccinated_persons;
}

SkipList get_non_vacc_list(VirusInfo info)
{
	return info->not_vaccinated_persons;
}

void virus_info_print(VirusInfo info)
{
	printf("%s\n", info->virus_name);
	printf("Vaccinated People skip list:\n\n");
	skip_list_print(info->vaccinated_persons);
	printf("Not Vaccinated People skip list:\n\n");
	skip_list_print(info->not_vaccinated_persons);
}

/*_______________________________________________________________*/


CountryInfo country_info_create(char * country_name)
{
	CountryInfo info = malloc(sizeof(struct country_info));
	if (info == NULL)
		fprintf(stderr, "Error : country_info_create -> malloc\n");
	assert(info != NULL);

	info->country_name = malloc(strlen(country_name) + 1);
	strcpy(info->country_name, country_name);
	info->population = 0;

	return info;
}

void country_info_destroy(CountryInfo info)
{
	if (info == NULL)
		fprintf(stderr, "Error : country_info_delete -> malloc\n");
	assert(info != NULL);

	free(info->country_name);
	free(info);
}

char * get_country_name(CountryInfo info)
{
	return info->country_name;
}

void country_population_inc(CountryInfo info)
{
	info->population++;
}

unsigned long country_population(CountryInfo info)
{
	return info->population;
}

void country_info_print(CountryInfo info)
{
	printf("%s\n", info->country_name);
}

/*______________________________________________________________________*/
// utility functions for dates

// checking for validity of a date
int date_check(char * date)
{
	char * day, * month, * year;
	//char * temp_date = calloc(strlen(date) + 1, sizeof(char));		// create a temp date, since strtok modifies initial string, and we do not want that
	char temp_date[12];
	strcpy(temp_date, date);

	char *str = strtok(temp_date, "-");
	int i = 1;
	while(str != NULL)
	{
		switch (i)
	    {
	    	case 1: day = str; break;
	        case 2: month = str; break;
	        case 3: year = str; break;
	    }
	    i++;
	    str = strtok(NULL, "-");
	}

	if (i != 4)
		return 0;
	if (strlen(day) > 2 || strlen(month) > 2 || strlen(year) != 4)
		return 0;
	for (int i = 0; i < strlen(day); i++)
	{
		if (day[i] < '0' || day[i] > '9')
			return 0;
	}
	for (int i = 0; i < strlen(month); i++)
	{
		if (month[i] < '0' || month[i] > '9')
			return 0;
	}

	for (int i = 0; i < strlen(year); i++)
	{
		if (year[i] < '0' || year[i] > '9')
			return 0;
	}

	if (atoi(day) < 1 || atoi(day) > 30 || atoi(month) < 1 || atoi(month) > 12 )
		return 0;

	return 1;
}

// comparing two valid dates
int date_cmp(char * date1, char * date2)
{
	int day1, month1, year1;
	int day2, month2, year2;

	//char * temp_date1 = calloc(strlen(date1) + 1, sizeof(char));	// create temp dates, since strtok modifies initial strings, and we do not want that
	char temp_date1[12];
	strcpy(temp_date1, date1);
	//char * temp_date2 = calloc(strlen(date2) + 1, sizeof(char));
	char temp_date2[12];
	strcpy(temp_date2, date2);
	

	char *str1 = strtok(temp_date1, "-");
	int i = 1;
	while(str1 != NULL)
	{
		switch (i)
	    {
	    	case 1: day1 = atoi(str1); break;
	        case 2: month1 = atoi(str1); break;
	        case 3: year1 = atoi(str1); break;
	    }
	    i++;
	    str1 = strtok(NULL, "-");
	}

	char *str2 = strtok(temp_date2, "-");
	i = 1;
	while(str2 != NULL)
	{
		switch (i)
	    {
	    	case 1: day2 = atoi(str2); break;
	        case 2: month2 = atoi(str2); break;
	        case 3: year2 = atoi(str2); break;
	    }
	    i++;
	    str2 = strtok(NULL, "-");
	}

	if (year2 > year1) return -1;
	else if (year2 < year1) return 1;
	else if (month2 > month1) return -1;
	else if (month2 < month1) return 1;
	else if (day2 > day1) return -1;
	else if (day2 < day1) return 1;
	else return 0;
}