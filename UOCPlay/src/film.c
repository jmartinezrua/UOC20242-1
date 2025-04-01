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
    assert(list != NULL);

    // Crear un nuevo nodo
    tFilmListNode* node = (tFilmListNode*)malloc(sizeof(tFilmListNode));
    if (node == NULL) {
        return E_MEMORY_ERROR; // Error de memoria
    }

    // Copiar los datos de la película al nodo
    film_cpy(&node->elem, film);
    node->next = NULL;

    // Añadir el nodo al final de la lista
    if (list->first == NULL) {
        list->first = node;
    } else {
        list->last->next = node;
    }
    list->last = node;
    list->count++;

    return E_SUCCESS;
}

// Remove a film from the list
tApiError filmList_del(tFilmList* list, const char* name) {
    assert(list != NULL);
    assert(name != NULL);

    tFilmListNode *node = list->first, *prev = NULL;

    // Buscar el nodo a eliminar
    while (node != NULL) {
        if (strcmp(node->elem.name, name) == 0) {
            break;
        }
        prev = node;
        node = node->next;
    }

    // Si no se encuentra el nodo, devolver error
    if (node == NULL) {
        return E_FILM_NOT_FOUND;
    }

    // Ajustar los punteros para eliminar el nodo
    if (prev == NULL) {
        list->first = node->next;
    } else {
        prev->next = node->next;
    }

    if (list->last == node) {
        list->last = prev;
    }

    list->count--;

    // Liberar la memoria del nodo
    film_free(&node->elem);
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
    assert(list != NULL);
    assert(film != NULL);

    // Verificar si la película ya existe en la lista de películas gratuitas
    if (freeFilmList_find(*list, film->name) != NULL) {
        return E_FILM_DUPLICATED; // La película ya existe
    }

    // Crear un nuevo nodo
    tFreeFilmListNode* node = (tFreeFilmListNode*)malloc(sizeof(tFreeFilmListNode));
    if (node == NULL) {
        return E_MEMORY_ERROR; // Error de memoria
    }

    // Almacenar la referencia a la película
    node->elem = film;
    node->next = NULL;

    // Añadir el nodo al final de la lista
    if (list->first == NULL) {
        list->first = node;
    } else {
        list->last->next = node;
    }
    list->last = node;
    list->count++;

    return E_SUCCESS;
}

// Remove a free film from the list
tApiError freeFilmList_del(tFreeFilmList* list, const char* name) {
    assert(list != NULL);
    assert(name != NULL);

    tFreeFilmListNode *node = list->first, *prev = NULL;

    // Buscar el nodo a eliminar
    while (node != NULL) {
        if (strcmp(node->elem->name, name) == 0) {
            break;
        }
        prev = node;
        node = node->next;
    }

    // Si no se encuentra el nodo, devolver error
    if (node == NULL) {
        return E_FILM_NOT_FOUND;
    }

    // Ajustar los punteros para eliminar el nodo
    if (prev == NULL) {
        list->first = node->next;
    } else {
        prev->next = node->next;
    }

    if (list->last == node) {
        list->last = prev;
    }

    list->count--;

    // Liberar la memoria del nodo
    free(node);

    return E_SUCCESS;
}

// Find a free film in the list by its name
tFilm* freeFilmList_find(tFreeFilmList list, const char* name) {
    // Check preconditions
    assert(name != NULL);

    tFreeFilmListNode* node = list.first;

    // Recorrer la lista de películas gratuitas
    while (node != NULL) {
        if (strcmp(node->elem->name, name) == 0) {
            return node->elem; // Retorna un puntero a la película si se encuentra
        }
        node = node->next;
    }

    return NULL; // Retorna NULL si no se encuentra
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
    assert(catalog != NULL);

    // Inicializar las listas de películas y películas gratuitas
    tApiError error = filmList_init(&catalog->filmList);
    if (error != E_SUCCESS) {
        return error;
    }

    error = freeFilmList_init(&catalog->freeFilmList);
    if (error != E_SUCCESS) {
        return error;
    }

    return E_SUCCESS;
}

// Add a new film to the catalog
tApiError catalog_add(tCatalog* catalog, tFilm film) {
    // Verificar precondiciones
    assert(catalog != NULL);

    // Verificar si la película ya existe en la lista de películas
    if (filmList_find(catalog->filmList, film.name) != NULL) {
        return E_FILM_DUPLICATED; // La película ya existe
    }

    // Intentar añadir la película a la lista de películas
    tApiError error = filmList_add(&catalog->filmList, film);
    if (error != E_SUCCESS) {
        return error; // Error al añadir la película
    }

    // Si la película es gratuita, añadirla a la lista de películas gratuitas
    if (film.isFree) {
        error = freeFilmList_add(&catalog->freeFilmList, &catalog->filmList.last->elem);
        if (error != E_SUCCESS) {
            // Si falla, eliminar la película de la lista de películas
            filmList_del(&catalog->filmList, film.name);
            return error;
        }
    }

    return E_SUCCESS; // Película añadida correctamente
}

// Remove a film from the catalog
tApiError catalog_del(tCatalog* catalog, const char* name) {
    assert(catalog != NULL);
    assert(name != NULL);

    // Buscar la película en la lista de películas
    tFilm* film = filmList_find(catalog->filmList, name);
    if (film == NULL) {
        return E_FILM_NOT_FOUND; // La película no existe
    }

    // Si la película es gratuita, eliminarla de la lista de películas gratuitas
    if (film->isFree) {
        tApiError error = freeFilmList_del(&catalog->freeFilmList, name);
        if (error != E_SUCCESS) {
            return error; // Error al eliminar de la lista de películas gratuitas
        }
    }

    // Eliminar la película de la lista de películas
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
    assert(catalog != NULL);

    // Liberar las listas de películas y películas gratuitas
    tApiError error = filmList_free(&catalog->filmList);
    if (error != E_SUCCESS) {
        return error;
    }

    error = freeFilmsList_free(&catalog->freeFilmList);
    if (error != E_SUCCESS) {
        return error;
    }

    return E_SUCCESS;
}
