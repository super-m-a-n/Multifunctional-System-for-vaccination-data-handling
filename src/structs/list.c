/*file : virus_list.c */
#include <stdlib.h>
#include <stdio.h>
#include "list.h"
#include <string.h>
#include "items.h"
#include <assert.h>

// data struct for list
struct list {
	int type;		 // 0 : list of citizens info nodes. 1: list of virus info nodes 2 : list of countries info nodes
	ListNode dummy;  // fake first node, not really used, just a trick that makes insertion function simpler
	ListNode last;   // pointer to last node.  Makes insertion at end O(1)
	int size;
};

// data struct for node of list
struct list_node {
	void * value;
	ListNode next;       // Pointer to next entry
};


List list_create(int type)
{
  	// malloc list struct
	List list = malloc(sizeof(*list));
	if (list == NULL)
		fprintf(stderr, "Error : list_create -> malloc\n");
	assert(list != NULL);

	list->size = 0;
  	// malloc first-fake node
	list->dummy = malloc(sizeof(*list->dummy));
	if (list->dummy == NULL)
		fprintf(stderr, "Error : list_create-> malloc\n");
	assert(list->dummy != NULL);

	list->dummy->next = NULL;
	list->dummy->value = NULL;
	list->last = list->dummy;
	list->type = type;

	return list;
}

void list_destroy(List list)
{
	if (list == NULL)
		fprintf(stderr, "Error : list_destroy -> list is NULL\n");
	assert(list != NULL);

	ListNode node = list->dummy; 		// starting from fake-dummy node
	while (node != NULL) {				// for every node of list
		ListNode next = node->next;		// save next node
		if (node != list->dummy)
		{
			// delete/free each of the node's components 
			switch (list->type)
			{
				case 0 : citizen_info_destroy((CitizenInfo) node->value); break;
				case 1 : virus_info_destroy((VirusInfo) node->value); break;
				case 2 : country_info_destroy((CountryInfo) node->value); break;
			}
		}
		
		free(node);   // free node of list
		node = next;  // continue iteration of list
	}
  	// at last free the struct of list
	free(list);
}

int list_size(List list) {
	if (list == NULL)
		fprintf(stderr, "Error : list_size -> list is NULL\n");
	assert(list != NULL);
	return list->size;
}

void list_insert_next(List list, ListNode node, void * value)
{
	if (list == NULL)
		fprintf(stderr, "Error : list_insert_next -> list is NULL\n");
	assert(list != NULL);

  	// if previous node is NULL, insertion at start is done
	if (node == NULL)
		node = list->dummy;
  	// malloc new node
	ListNode new_node = malloc(sizeof(*new_node));
	if (new_node == NULL)
		fprintf(stderr, "Error : list_insert_next -> malloc\n");
	assert(new_node != NULL);

  	// initialize new node's components
	switch (list->type)
	{
		case 0 : new_node->value = (CitizenInfo) value; break;
		case 1 : new_node->value = (VirusInfo) value; break;
		case 2 : new_node->value = (CountryInfo) value; break;
	}

	// update pointers
	new_node->next = node->next;
	node->next = new_node;

	list->size++;

  	// if newly allocated entry is last entry of list, update last pointer
	if (list->last == node)
		list->last = new_node;
}

void list_insert_end(List list, void * value)
{
	if (list == NULL)
		fprintf(stderr, "Error : list_insert_end -> list is NULL\n");
	assert(list != NULL);

  	// just call list_insert_next with node the currently last node of list
  	list_insert_next(list, list->last, value);
}

void * list_search(List list, void * key)
{
	if (list == NULL)
		fprintf(stderr, "Error : list_search -> list is NULL\n");
	assert(list != NULL);

	void * node_key = NULL;

	// searches list to find if a node with given key already exists
	for (ListNode node = list->dummy->next; node != NULL; node = node->next)
	{
		switch (list->type)
		{
			case 0 : node_key = get_citizen_id((CitizenInfo) node->value); break;
			case 1 : node_key = get_virus_name((VirusInfo) node->value); break;
			case 2 : node_key = get_country_name((CountryInfo) node->value); break;
		}

		if (!strcmp((char *) node_key, (char *) key))
			return node->value;		// node with given key exists, so return it
	}

	return NULL;	// given key does not exist, return NULL
}

void list_print(List list)
{
	if (list == NULL)
		fprintf(stderr, "Error : list_print -> list is NULL\n");
	assert(list != NULL);

	for (ListNode node = list->dummy->next; node != NULL; node = node->next)
	{
		switch (list->type)
		{
			case 0 : citizen_info_print((CitizenInfo) node->value); break;
			case 1 : virus_info_print((VirusInfo) node->value); break;
			case 2 : country_info_print((CountryInfo) node->value); break;
		}
	}			
}

ListNode list_first(List list) 
{
	assert(list != NULL);
	// first node is next of dummy node
	return list->dummy->next;
}

ListNode list_dummy(List list)
{
	assert(list != NULL);
	return list->dummy;
}

ListNode list_next(List list, ListNode node) 
{
	assert(list != NULL);
	assert(node != NULL);
	return node->next;
}

void * list_value(List list, ListNode node)
{
	assert(list != NULL);
	assert(node != NULL);
	return node->value;
}

