typedef struct {
	char destination[160];
	// string for calling at a favorite station?
	char expected_departure[160];
	int platform;
} DepartureData;

typedef struct {
         DepartureData *departures;
         size_t size;
} DepartureList;

typedef struct DepartureDataList {
    DepartureData data;
    struct DepartureDataList *next;
} DepartureDataList;

void departure_data_list_destroy(DepartureDataList* list);
void departure_data_list_add(DepartureDataList** list, DepartureData data);
DepartureDataList* departure_data_list_add2(DepartureDataList* list, DepartureData data);
DepartureData departure_data_list_get(DepartureDataList* departures, int index);
int departure_data_list_get_size(DepartureDataList* list);
void dep_print(DepartureDataList* departures);

DepartureList departure_list_create();
void departure_list_destroy(DepartureList list);
void departure_list_add(DepartureList* list, DepartureData data);
DepartureData departure_list_get(DepartureList list, int index);
DepartureData* departure_data_create(const int size);
void departure_data_destroy(DepartureData* dep);