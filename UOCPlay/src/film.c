#include "film.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

// Parse input from CSVEntry
void film_parse(tFilm* data, tCSVEntry entry) {
	// Check input data
	assert(data != NULL);
	assert(csv_numFields(entry) == NUM_FIELDS_FILM);

	int pos = 0;

	// Name
	const char* name = entry.fields[pos++];
	assert(name != NULL);

	// Duration
	assert(strlen(entry.fields[pos]) == TIME_LENGTH);
	tTime duration;
	int itemsRead = sscanf(entry.fields[pos++], "%d:%d", &duration.hour, &duration.minutes);
	assert(itemsRead == 2);

	// Genre
	int genreValue = csv_getAsInteger(entry, pos++);
	assert(genreValue >= GENRE_FIRST && genreValue < GENRE_END);
	tFilmGenre genre = (tFilmGenre)genreValue;

	// Release date
	assert(strlen(entry.fields[pos]) == DATE_LENGTH);
	tDate release;
	itemsRead = sscanf(entry.fields[pos++], "%d/%d/%d", &release.day, &release.month, &release.year);
	assert(itemsRead == 3);

	// Rating
	float rating = csv_getAsReal(entry, pos++);
	assert(rating >= RATING_MIN && rating <= RATING_MAX);

	// isFree
	int isFree = csv_getAsInteger(entry, pos++);
	assert(isFree == 0 || isFree == 1);

	// Call film_init with the parsed data
	film_init(data, name, duration, genre, release, rating, (bool)isFree);
}

// Initialize a film
void film_init(tFilm* data, const char* name, tTime duration, tFilmGenre genre, tDate release, float rating, bool isFree) {
	// Check preconditions
	assert(data != NULL);
	assert(name != NULL);
	
	// Name
	data->name = (char*) malloc((strlen(name) + 1) * sizeof(char));
	assert(data->name != NULL);
	strcpy(data->name, name);
	
	// Duration
	time_cpy(&data->duration, duration);
	
	// Genre
	data->genre = genre;
	
	// Release
	date_cpy(&data->release, release);
	
	// Rating
	data->rating = rating;
	
	// isFree
	data->isFree = isFree;
}

// Copy a film from src to dst
void film_cpy(tFilm* dst, tFilm src) {
	// Check preconditions
	assert(dst != NULL);
	
	film_init(dst, src.name, src.duration, src.genre, src.release, src.rating, src.isFree);
}

// Get film data using a string
void film_get(tFilm data, char* buffer) {
	// Print all data at same time
    sprintf(buffer,"%s;%02d:%02d;%d;%02d/%02d/%04d;%.1f;%d",
        data.name,
		data.duration.hour, data.duration.minutes,
		data.genre,
		data.release.day, data.release.month, data.release.year,
		data.rating,
		data.isFree);
}

// Remove the data from a film
void film_free(tFilm* data) {
	// Check preconditions
	assert(data != NULL);
	
	if (data->name != NULL)
	{
		free(data->name);
		data->name = NULL;
	}
}

// Initialize the films list
tApiError filmList_init(tFilmList* list) {
	// Check preconditions
	assert(list != NULL);
	
	list->first = NULL;
	list->last = NULL;
	list->count = 0;
	
	return E_SUCCESS;
}

// Add a new film to the list
tApiError filmList_add(tFilmList* list, tFilm film) {
	// Check preconditions
	assert(list != NULL);
	
	tFilmListNode *node;
	
	// Check if the film is already in the list
	if (filmList_find(*list, film.name) != NULL)
		return E_FILM_DUPLICATED;
	
	// Create the node
	node = (tFilmListNode*)malloc(sizeof(tFilmListNode));
	assert(node != NULL);
	
	// Assign the properties of the nodes
	film_cpy(&node->elem, film);
	node->next = NULL;
	
	// Link the new node to the end of the list
	if (list->first == NULL)
		list->first = node;
	else
		list->last->next = node;
	
	list->last = node;
	list->count++;
	
	return E_SUCCESS;
}

// Remove a film from the list
tApiError filmList_del(tFilmList* list, const char* name) {
	// Check preconditions
	assert(list != NULL);
	assert(name != NULL);
	
	tFilmListNode *node, *prev;
	
	// Iterate until the node and remove it
	node = list->first;
	prev = NULL;
	
	while (node != NULL)
	{
		if (strcmp(node->elem.name, name) == 0)
			break;
		
		prev = node;
		node = node->next;
	}
	
	// If node does not exist, return an error
	if (node == NULL)
		return E_FILM_NOT_FOUND;
	
	// Link the list without the node to remove
	if (prev == NULL)
		list->first = node->next;
	else
		prev->next = node->next;
	
	if (list->last == node)
		list->last = prev;
	
	list->count--;
	
	film_free(&(node->elem));
	free(node);
	
	return E_SUCCESS;
}

// Return a pointer to the film
tFilm* filmList_find(tFilmList list, const char* name) {
	// Check preconditions
	assert(name != NULL);
	
	tFilmListNode *node;
	node = list.first;
	
	while (node != NULL)
	{
		if (strcmp(node->elem.name, name) == 0)
			return &(node->elem);
		
		node = node->next;
	}
	
	return NULL;
}

// Remove the films from the list
tApiError filmList_free(tFilmList* list) {
	// Check preconditions
	assert(list != NULL);
	
	tFilmListNode *node, *auxNode;
	
	node = list->first;
	auxNode = NULL;
	
	while (node != NULL)
	{
		auxNode = node->next;
		
		film_free(&(node->elem));
		free(node);
		
		node = auxNode;
	}
	
	filmList_init(list);
	
	return E_SUCCESS;
}

// Initialize the free films list
tApiError freeFilmList_init(tFreeFilmList* list) {
	// Check preconditions
	assert(list != NULL);
	
	list->first = NULL;
	list->last = NULL;
	list->count = 0;
	
	return E_SUCCESS;
}

// Add a new free film to the list
tApiError freeFilmList_add(tFreeFilmList* list, tFilm* film) {
	// Check preconditions
	assert(list != NULL);
	assert(film != NULL);

	if (freeFilmList_find(*list, film->name) != NULL)
		return E_FILM_DUPLICATED;

	tFreeFilmListNode* node = (tFreeFilmListNode*)malloc(sizeof(tFreeFilmListNode));
	assert(node != NULL);

	node->elem = film; // Store the reference
	node->next = NULL;

	if (list->first == NULL)
		list->first = node;
	else
		list->last->next = node;

	list->last = node;
	list->count++;

	return E_SUCCESS;
}

// Remove a free film from the list
tApiError freeFilmList_del(tFreeFilmList* list, const char* name) {
	// Check preconditions
	assert(list != NULL);
	assert(name != NULL);

	tFreeFilmListNode *node = list->first, *prev = NULL;

	while (node != NULL)
	{
		if (strcmp(node->elem->name, name) == 0)
			break;
		prev = node;
		node = node->next;
	}

	if (node == NULL)
		return E_FILM_NOT_FOUND;

	if (prev == NULL)
		list->first = node->next;
	else
		prev->next = node->next;

	if (list->last == node)
		list->last = prev;

	free(node);
	list->count--;

	return E_SUCCESS;
}

// Return a pointer to the free film
tFilm* freeFilmList_find(tFreeFilmList list, const char* name) {
	// Check preconditions
	assert(name != NULL);
	
	tFreeFilmListNode *node;
	node = list.first;
	
	while (node != NULL)
	{
		if (strcmp(node->elem->name, name) == 0)
			return node->elem;
			
		node = node->next;
	}
	
	return NULL;
}

// Remove the free films from the list
tApiError freeFilmsList_free(tFreeFilmList* list) {
	// Check preconditions
	assert(list != NULL);
	
	tFreeFilmListNode *node, *auxNode;
	
	node = list->first;
	auxNode = NULL;
	
	while (node != NULL)
	{
		auxNode = node->next;
		free(node);
		node = auxNode;
	}
	
	freeFilmList_init(list);
	
	return E_SUCCESS;
}

// Initialize the films catalog
tApiError catalog_init(tCatalog* catalog) {
	/////////////////////////////////
	// PR1_2a
	/////////////////////////////////
	// Comprobar que el puntero no sea NULL
    assert(catalog != NULL);
    
    // Inicializar la lista de películas
    tApiError err = filmList_init(&catalog->filmList);
    if (err != E_SUCCESS) {
        return err;
    }
    
    // Inicializar la lista de películas gratuitas
    err = freeFilmList_init(&catalog->freeFilmList);
    if (err != E_SUCCESS) {
        return err;
    }
	/////////////////////////////////
    return E_SUCCESS;
}

// Add a new film to the catalog
tApiError catalog_add(tCatalog* catalog, tFilm film) {
	/////////////////////////////////
	// PR1_2b
	/////////////////////////////////
	assert(catalog != NULL);
    
    // Verificar que la película no exista ya en el listado principal
    if (filmList_find(catalog->filmList, film.name) != NULL)
        return E_FILM_DUPLICATED;
    
    // Agregar la película al listado principal
    tApiError err = filmList_add(&catalog->filmList, film);
    if (err != E_SUCCESS)
        return err;
    
    // Recuperar la referencia de la película recién agregada
    tFilm* addedFilm = filmList_find(catalog->filmList, film.name);
    if (addedFilm == NULL)
        return E_MEMORY_ERROR; // Caso inesperado

    // Si la película es gratuita, agregarla al listado de películas gratuitas
    if (addedFilm->isFree) {
        err = freeFilmList_add(&catalog->freeFilmList, addedFilm);
        if (err != E_SUCCESS)
            return err;
    }
	/////////////////////////////////
    return E_SUCCESS;
}

// Remove a film from the catalog
tApiError catalog_del(tCatalog* catalog, const char* name) {
	/////////////////////////////////
	// PR1_2c
	/////////////////////////////////
	// Paso 1: Comprobar que los punteros no sean nulos
    assert(catalog != NULL);
    assert(name != NULL);

    // Buscar la película en la lista principal
    tFilm* filmPtr = filmList_find(catalog->filmList, name);
    if (filmPtr == NULL) {
        return E_FILM_NOT_FOUND; // O el error definido correspondiente
    }

    // Si la película es gratuita, eliminarla de la lista de películas gratuitas
    if (filmPtr->isFree) {
		tApiError err = freeFilmList_del(&catalog->freeFilmList, filmPtr->name);
		// Se puede ignorar un error de "no encontrada" si ya no estaba
		if (err != E_SUCCESS && err != E_FILM_NOT_FOUND) {
			return err;
		}
	}

    // Ahora eliminarla de la lista principal, lo que liberará la memoria
    /////////////////////////////////
    return filmList_del(&catalog->filmList, name);
}

// Return the number of total films
int catalog_len(tCatalog catalog) {
	/////////////////////////////////
	// PR1_2d
	/////////////////////////////////
	
	/////////////////////////////////
    return catalog.filmList.count;
}

// Return the number of free films
int catalog_freeLen(tCatalog catalog) {
	/////////////////////////////////
	// PR1_2d
	/////////////////////////////////
	
	/////////////////////////////////
    return catalog.freeFilmList.count;
}

// Remove the films from the catalog
tApiError catalog_free(tCatalog* catalog) {
	/////////////////////////////////
	// PR1_2e
	/////////////////////////////////
	// Comprobar que el puntero no sea NULL
    assert(catalog != NULL);

    tApiError err;
    
    // Liberar la lista principal de películas
    err = filmList_free(&catalog->filmList);
    if(err != E_SUCCESS)
        return err;
    
    // Liberar la lista de películas gratuitas
    err = freeFilmsList_free(&catalog->freeFilmList);
    if(err != E_SUCCESS)
        return err;
	/////////////////////////////////
    return E_SUCCESS;
}
