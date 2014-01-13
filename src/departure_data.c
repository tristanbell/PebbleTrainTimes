#include <stdlib.h>
#include <pebble.h>
#include "departure_data.h"

DepartureData* departure_data_create(const int size)
{
        DepartureData *data = (DepartureData*) malloc(sizeof(DepartureData) * size);
        return data;
}

void departure_data_destroy(DepartureData* dep)
{
    free(dep);
}

/*void grow_ar_to(DepartureData* ar, size_t new_bytes)
{
    DepartureData* tmp = realloc(ar, new_bytes);
    if(tmp != NULL)
    {
        ar = tmp;
    }
    else
    {
	
    }
}*/

void departure_list_add(DepartureList *list, DepartureData data)
{
        if (list->size == sizeof(list->departures) / sizeof(DepartureData)) {
               //  grow_ar_to(list.departures, list.size * 2);
                 return;
        }

        (*list).departures[list->size] = data;
        (*list).size++;
}

DepartureData departure_list_get(DepartureList list, int index)
{
        // do bounds checking here
        return list.departures[index];
}

DepartureList departure_list_create(size_t size)
{
         DepartureList list = (DepartureList) {
                .departures = departure_data_create(size),
                .size = 0
         };
           return list;
}

void departure_list_destroy(DepartureList list)
{
	departure_data_destroy(list.departures);
	list.size = 0;
}

DepartureDataList* departure_data_list_create(DepartureData data)
{
	DepartureDataList *list = malloc(sizeof(DepartureDataList));
	list->data = data;
	list->next = NULL;
	return list;
}

int departure_data_list_get_size(DepartureDataList* list)
{
	if (!list) {
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "List empty");
		return 0;
	}
	
	return 1 + departure_data_list_get_size(list->next);
}

void departure_data_list_destroy(DepartureDataList* list)
{
	DepartureDataList *temp, *toDelete;
	toDelete = list;
	
	while(toDelete) {
		temp = toDelete->next;
		free(toDelete);
		toDelete = temp;
	}
}

void departure_data_list_add(DepartureDataList** list, DepartureData data)
{
	if (!(*list)) {
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "New list created.");
		*list = departure_data_list_create(data);
		return;
	}
	
	departure_data_list_add(&((*list)->next), data);
}

DepartureDataList* departure_data_list_add2(DepartureDataList* list, DepartureData data)
{
	if (!list) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "New list created.");
		list = departure_data_list_create(data);
		return list;
	}
	
	DepartureDataList* temp = list;
	
	while (temp->next) {
		temp = temp->next;
	}
	temp->next = departure_data_list_create(data);
	
	return list;
}

void dep_print(DepartureDataList* departures)
{
	if (!departures) return;
	
	while (departures) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Printing: %s, %s", departures->data.destination, departures->data.expected_departure);
		departures = departures->next;
	}
}

DepartureData departure_data_list_get(DepartureDataList* departures, int index)
{
	//dep_print(departures);
	
	if (!departures) {
		DepartureData empty;
		empty.destination[0] = '\0';
		empty.expected_departure[0] = '\0';
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Error: no departures to get.");
		return empty;
		// Throw exception instead
	}

    DepartureDataList* temp = departures;

    int count = 0;
    while (temp->next && count++ < index) {
		temp = temp->next;
    }
	
	return temp->data;
}








