#include <stdio.h>
#include <stdlib.h> 
#include <assert.h>
#include "csv.h"
#include "api.h"
#include <string.h>

//////////////////////////////////////////////
static void subscription_free(tSubscription* data);
static void local_dateToString(tDate date, char* buffer);
//////////////////////////////////////////////




// Get the API version information
const char* api_version()
{
    return "UOC PP 20242";
}

// Load data from a CSV file. If reset is true, remove previous data
tApiError api_loadData(tApiData* data, const char* filename, bool reset)
{
    FILE *fin;
    char buffer[FILE_READ_BUFFER_SIZE];

    // Comprobar punteros de entrada
    if (data == NULL || filename == NULL)
        return E_MEMORY_ERROR;

    // Si se desea reiniciar, liberar datos previos y re-inicializarlos
    if (reset) {
        tApiError err = api_freeData(data);
        if (err != E_SUCCESS)
            return err;
        err = api_initData(data);
        if (err != E_SUCCESS)
            return err;
    }
    
    fin = fopen(filename, "r");
    if (fin == NULL)
        return E_FILE_NOT_FOUND;
    
    // Leer el archivo línea a línea
    while (fgets(buffer, FILE_READ_BUFFER_SIZE, fin) != NULL) {
        // Eliminar posibles saltos de línea y espacios en blanco al final
        size_t len = strlen(buffer);
        if (len == 0)
            continue;
        if (buffer[len - 1] == '\n')
            buffer[len - 1] = '\0';
        if (strlen(buffer) == 0)
            continue;
        
        // Inicializar y parsear la entrada CSV
        tCSVEntry entry;
        csv_initEntry(&entry);
        // Se asume que csv_parseEntry separa la línea en campos y, si type es NULL, lo determina a partir de la primera entrada.
        csv_parseEntry(&entry, buffer, NULL);
        
        // Añadir la entrada a la API; si ocurre un error, se libera la entrada y se retorna el error
        tApiError addErr = api_addDataEntry(data, entry);
        if (addErr != E_SUCCESS) {
            csv_freeEntry(&entry);
            fclose(fin);
            return addErr;
        }
        // Liberar recursos de la entrada temporal
        csv_freeEntry(&entry);
    }
    fclose(fin);
    
    return E_SUCCESS;
}

// Initialize the data structure
tApiError api_initData(tApiData* data) {
	/////////////////////////////////
	// PR1_3b
	/////////////////////////////////
	// Verificar que el puntero no sea NULL
    if (data == NULL)
        return E_MEMORY_ERROR;
    
    tApiError err = people_init(&data->people);
    if (err != E_SUCCESS)
        return err;
    
    err = subscriptions_init(&data->subscriptions);
    if (err != E_SUCCESS)
        return err;
    
    err = catalog_init(&data->catalog);
    if (err != E_SUCCESS)
        return err;
    /////////////////////////////////
    return E_SUCCESS;
}

// Add a person into the data if it does not exist
tApiError api_addPerson(tApiData* data, tCSVEntry entry) {
	/////////////////////////////////
	// PR1_3c
	/////////////////////////////////
	// Comprobar que el número de campos es el esperado
    // Verificar formato y tipo de la entrada
    // Verificar que el número de campos es el esperado para una persona
    if (csv_numFields(entry) != NUM_FIELDS_PERSON)
        return E_INVALID_ENTRY_FORMAT;
    
    // Validar que ningún campo esté vacío
    for (int i = 0; i < entry.numFields; i++) {
        if (entry.fields[i] == NULL || strlen(entry.fields[i]) == 0)
            return E_INVALID_ENTRY_FORMAT;
    }
    
    // Verificar que el tipo de entrada sea "PERSON"
    const char* type = csv_getType(&entry);
    if (strcmp(type, "PERSON") != 0)
        return E_INVALID_ENTRY_TYPE;
    
    // Parsear la entrada CSV en una estructura tPerson
    tPerson tempPerson;
    person_parse(&tempPerson, entry);
    
    // Intentar añadir la persona; si ya existe, liberar la memoria y devolver error
    tApiError err = people_add(&data->people, tempPerson);
    if (err != E_SUCCESS) {
        person_free(&tempPerson);
        return err;
    }
    
    // Liberar recursos temporales (si fuese necesario)
    person_free(&tempPerson);
	/////////////////////////////////
    return E_SUCCESS;
}

// Add a subscription if it does not exist
tApiError api_addSubscription(tApiData* data, tCSVEntry entry) {
	/////////////////////////////////
	// PR1_3d
	/////////////////////////////////
	// Validar número de campos para una suscripción
    if (csv_numFields(entry) != NUM_FIELDS_SUBSCRIPTION)
        return E_INVALID_ENTRY_FORMAT;
    
    // Validar que ningún campo esté vacío
    for (int i = 0; i < entry.numFields; i++) {
        if (entry.fields[i] == NULL || strlen(entry.fields[i]) == 0)
            return E_INVALID_ENTRY_FORMAT;
    }
    
    // Verificar que el tipo de entrada sea "SUBSCRIPTION"
    const char* type = csv_getType(&entry);
    if (strcmp(type, "SUBSCRIPTION") != 0)
        return E_INVALID_ENTRY_TYPE;
    
    // Parsear la entrada CSV en una estructura tSubscription
    tSubscription tempSub;
    subscription_parse(&tempSub, entry);
    
    // Comprobar si ya existe una suscripción con el mismo id
    if (subscriptions_find(data->subscriptions, tempSub.id) >= 0) {
        subscription_free(&tempSub);
        return E_SUBSCRIPTION_DUPLICATED;
    }
    
    // Verificar que la persona asociada exista
    if (people_find(data->people, tempSub.document) < 0) {
        subscription_free(&tempSub);
        return E_PERSON_NOT_FOUND;
    }
    
    // Intentar añadir la suscripción
    tApiError err = subscriptions_add(&data->subscriptions, data->people, tempSub);
    if (err != E_SUCCESS) {
        subscription_free(&tempSub);
        return err;
    }
	/////////////////////////////////
    return E_SUCCESS;
}

// Add a film if it does not exist
tApiError api_addFilm(tApiData* data, tCSVEntry entry) {
	/////////////////////////////////
	// PR1_3e
	/////////////////////////////////
	// Validar número de campos para una película
    if (csv_numFields(entry) != NUM_FIELDS_FILM)
        return E_INVALID_ENTRY_FORMAT;
    
    // Validar que ningún campo esté vacío
    for (int i = 0; i < entry.numFields; i++) {
        if (entry.fields[i] == NULL || strlen(entry.fields[i]) == 0)
            return E_INVALID_ENTRY_FORMAT;
    }
    
    // Verificar que el tipo de entrada sea "FILM"
    const char* type = csv_getType(&entry);
    if (strcmp(type, "FILM") != 0)
        return E_INVALID_ENTRY_TYPE;
    
    // Parsear la entrada CSV en una estructura tFilm
    tFilm film;
    film_parse(&film, entry);
    
    // Intentar añadir la película al catálogo
    tApiError err = catalog_add(&data->catalog, film);
    if (err != E_SUCCESS) {
        film_free(&film);
        return err;
    }
    
    // Liberar la memoria asignada en la estructura temporal
    film_free(&film);
	/////////////////////////////////
    return E_SUCCESS;
}

// Get the number of people registered on the application
int api_peopleCount(tApiData data) {
	/////////////////////////////////
	// PR1_3f
	/////////////////////////////////
	
	/////////////////////////////////
    return people_count(data.people);
}

// Get the number of subscriptions registered on the application
int api_subscriptionsCount(tApiData data) {
	/////////////////////////////////
	// PR1_3f
	/////////////////////////////////
	
	/////////////////////////////////
    return subscriptions_len(data.subscriptions);
}

// Get the number of films registered on the application
int api_filmsCount(tApiData data) {
	/////////////////////////////////
	// PR1_3f
	/////////////////////////////////
	
	/////////////////////////////////
    return catalog_len(data.catalog);
}

// Get the number of free films registered on the application
int api_freeFilmsCount(tApiData data) {
	/////////////////////////////////
	// PR1_3f
	/////////////////////////////////
	
	/////////////////////////////////
    return catalog_freeLen(data.catalog);
}

// Free all used memory
tApiError api_freeData(tApiData* data) {
	/////////////////////////////////
	// PR1_3g
	/////////////////////////////////
	if (data == NULL)
        return E_MEMORY_ERROR;
    
    tApiError err;
    
    err = people_free(&data->people);
    if (err != E_SUCCESS)
        return err;
    
    err = subscriptions_free(&data->subscriptions);
    if (err != E_SUCCESS)
        return err;
    
    err = catalog_free(&data->catalog);
    if (err != E_SUCCESS)
        return err;
    /////////////////////////////////
    return E_SUCCESS;
}

// Add a new entry
tApiError api_addDataEntry(tApiData* data, tCSVEntry entry) {
	/////////////////////////////////
	// PR1_3h
	/////////////////////////////////
	const char* type = csv_getType(&entry);
    if (strcmp(type, "PERSON") == 0)
        return api_addPerson(data, entry);
    else if (strcmp(type, "SUBSCRIPTION") == 0)
        return api_addSubscription(data, entry);
    else if (strcmp(type, "FILM") == 0)
        return api_addFilm(data, entry);
    /////////////////////////////////
    return E_INVALID_ENTRY_TYPE;
}

// Get subscription data
tApiError api_getSubscription(tApiData data, int id, tCSVEntry *entry) {
	/////////////////////////////////
	// PR1_4a
	/////////////////////////////////
	// Comprobar que el número de campos es el esperado para una suscripción
    // Comprobar que la entrada tiene el número de campos esperado para una suscripción
   // Comprobar que la entrada tiene el número de campos esperado para una suscripción
   if (entry == NULL) return E_MEMORY_ERROR;

   int idx = subscriptions_find(data.subscriptions, id);
   if (idx < 0) return E_SUBSCRIPTION_NOT_FOUND;

   tSubscription* sub = &data.subscriptions.elems[idx];
   char startDate[11], endDate[11];
   local_dateToString(sub->start_date, startDate);
   local_dateToString(sub->end_date, endDate);

    char buffer[FILE_READ_BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "%d;%s;%s;%s;%s;%.2f;%d",
             sub->id, sub->document, startDate, endDate,
             sub->plan, sub->price, sub->numDevices);

    csv_parseEntry(entry, buffer, "SUBSCRIPTION");
	/////////////////////////////////
    return E_SUCCESS;
}

// Get film data
tApiError api_getFilm(tApiData data, const char* name, tCSVEntry *entry) {
	/////////////////////////////////
	// PR1_4b
	/////////////////////////////////
	// Verificar punteros de entrada
    if (entry == NULL || name == NULL)
        return E_MEMORY_ERROR;
    
    // Buscar la película en el catálogo (usando filmList_find definido en film.h)
    tFilm* filmPtr = filmList_find(data.catalog.filmList, name);
    if (filmPtr == NULL)
        return E_FILM_NOT_FOUND;
    
    // Formatear los datos de la película
    char buffer[FILE_READ_BUFFER_SIZE];
    film_get(*filmPtr, buffer);
    
    // Rellenar el CSVEntry con la cadena formateada y asignar el tipo "FILM"
    csv_parseEntry(entry, buffer, "FILM");
	/////////////////////////////////
	return E_SUCCESS;
}

// Get free films data
tApiError api_getFreeFilms(tApiData data, tCSVData *freeFilms) {
	/////////////////////////////////
	// PR1_4c
	/////////////////////////////////
	if (freeFilms == NULL)
        return E_MEMORY_ERROR;
    
    // Inicializar la estructura csvData de free films.
    freeFilms->count = 0;
    freeFilms->isValid = true;
    freeFilms->entries = NULL;
    
    // Recorrer la lista enlazada de películas en data.catalog.filmList.
    tFilmListNode *node = data.catalog.filmList.first;
    while (node != NULL) {
        // Si la película es gratuita, agregarla a freeFilms.
        if (node->elem.isFree) {
            char buffer[FILE_READ_BUFFER_SIZE];
            film_get(node->elem, buffer);
            
            tCSVEntry newEntry;
            csv_initEntry(&newEntry);
            csv_parseEntry(&newEntry, buffer, "FILM");
            
            tCSVEntry *temp = realloc(freeFilms->entries, sizeof(tCSVEntry) * (freeFilms->count + 1));
            if (temp == NULL) {
                return E_MEMORY_ERROR;
            }
            freeFilms->entries = temp;
            freeFilms->entries[freeFilms->count] = newEntry;
            freeFilms->count++;
        }
        node = node->next;
    }
	/////////////////////////////////
    return E_SUCCESS;
}

// Get films data by genre
tApiError api_getFilmsByGenre(tApiData data, tCSVData *films, int genre) {
	/////////////////////////////////
	// PR1_4d
	/////////////////////////////////
	if (films == NULL)
        return E_MEMORY_ERROR;
    
    // Inicializar la estructura csvData de films.
    films->count = 0;
    films->isValid = true;
    films->entries = NULL;
    
    // Recorrer la lista enlazada de películas en el catálogo.
    tFilmListNode *node = data.catalog.filmList.first;
    while (node != NULL) {
        // Si la película tiene el género solicitado, agregarla a films.
        if (node->elem.genre == (tFilmGenre)genre) {
            char buffer[FILE_READ_BUFFER_SIZE];
            // film_get formatea la información de la película en el formato especificado.
            film_get(node->elem, buffer);
            
            tCSVEntry newEntry;
            csv_initEntry(&newEntry);
            // Llena newEntry con la cadena formateada y asigna el tipo "FILM".
            csv_parseEntry(&newEntry, buffer, "FILM");
            
            // Ampliar el array de entradas dinámicamente.
            tCSVEntry *temp = realloc(films->entries, sizeof(tCSVEntry) * (films->count + 1));
            if (temp == NULL)
                return E_MEMORY_ERROR;
            films->entries = temp;
            films->entries[films->count] = newEntry;
            films->count++;
        }
        node = node->next;
    }
	/////////////////////////////////
    return E_SUCCESS;
}

////////////////////////////////////////////

// Definición local de subscription_free para evitar modificar el módulo subscription
static void subscription_free(tSubscription* data) {
    (void)data; // No hay memoria dinámica que liberar
}

static void local_dateToString(tDate date, char* buffer) {
    sprintf(buffer, "%02d/%02d/%04d", date.day, date.month, date.year);
}
///////////////////////////////////////////
