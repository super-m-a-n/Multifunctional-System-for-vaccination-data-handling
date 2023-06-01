/* file : items.h */
#pragma once
#include "bloom.h"
#include "skip_list.h"

typedef struct citizen_info * CitizenInfo;
typedef struct virus_info * VirusInfo;
typedef struct country_info * CountryInfo;

CitizenInfo citizen_info_create(char * id, char * name, char * surname, int age, CountryInfo country);
void citizen_info_destroy(CitizenInfo info);
char * get_citizen_id(CitizenInfo info);
char * get_citizen_name(CitizenInfo info);
char * get_citizen_surname(CitizenInfo info);
char * get_citizen_country(CitizenInfo info);
int get_citizen_age(CitizenInfo info);
void citizen_info_print(CitizenInfo info);

/*____________________________________________________________________________________________________*/

VirusInfo virus_info_create(char * virus_name, unsigned int bloom_size, int max_level, float p);
void virus_info_destroy(VirusInfo info);
char * get_virus_name(VirusInfo info);
Bloom get_bloom_filter(VirusInfo info);
SkipList get_vacc_list(VirusInfo info);
SkipList get_non_vacc_list(VirusInfo info);
void virus_info_print(VirusInfo info);

/*_____________________________________________________________________________________________________*/

CountryInfo country_info_create(char * country_name);
void country_info_destroy(CountryInfo info);
char * get_country_name(CountryInfo info);
void country_population_inc(CountryInfo info);
unsigned long country_population(CountryInfo info);
void country_info_print(CountryInfo info);

/*_____________________________________________________________________________________________________*/

int date_check(char * date);
int date_cmp(char * date1, char * date2);