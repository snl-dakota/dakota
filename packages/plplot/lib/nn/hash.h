/******************************************************************************
 *
 * File:           hash.h
 *
 * Purpose:        Hash table header
 *
 * Author:         Jerry Coffin
 *
 * Description:    Public domain code by Jerry Coffin, with improvements by
 *                 HenkJan Wolthuis.
 *                 Date last modified: 05-Jul-1997
 *
 * Revisions:      18-09-2002 -- modified by Pavel Sakov
 *
 *****************************************************************************/

#ifndef _HASH_H
#define _HASH_H

struct hashtable;
typedef struct hashtable hashtable;

/** Copies a key. The key must be able to be deallocated by free().
 */
typedef void* (*ht_keycp) (void*);

/** Returns 1 if two keys are equal, 0 otherwise.
 */
typedef int (*ht_keyeq) (void*, void*);

/** Converts key to an unsigned integer (not necessarily unique).
 */
typedef unsigned int (*ht_key2hash) (void*);

/** Creates a hash table of specified size.
 *
 * @param size Size of hash table for output points
 * @param cp Key copy function
 * @param eq Key equality check function
 * @param hash Hash value calculation function
 */
hashtable* ht_create(int size, ht_keycp cp, ht_keyeq eq, ht_key2hash hash);

/** Create a hash table of specified size and key type.
 */
hashtable* ht_create_d1(int size);      /* double[1] */
hashtable* ht_create_d2(int size);      /* double[2] */
hashtable* ht_create_str(int size);     /* char* */

/** Destroys a hash table.
 * (Take care of deallocating data by ht_process() prior to destroying the
 * table if necessary.)
 *
 * @param table Hash table to be destroyed
 */
void ht_destroy(hashtable* table);

/** Inserts a new entry into the hash table.
 *
 * @param table The hash table
 * @param key Ponter to entry's key
 * @param data Pointer to associated data
 * @return Pointer to the old data associated with the key, NULL if the key
 *         wasn't in the table previously
 */
void* ht_insert(hashtable* table, void* key, void* data);

/** Returns a pointer to the data associated with a key.  If the key has
 * not been inserted in the table, returns NULL.
 *
 * @param table The hash table
 * @param key The key
 * @return The associated data or NULL
 */
void* ht_find(hashtable* table, void* key);

/** Deletes an entry from the table.  Returns a pointer to the data that
 * was associated with the key so that the calling code can dispose it
 * properly.
 *
 * @param table The hash table
 * @param key The key
 * @return The associated data or NULL
 */
void* ht_delete(hashtable* table, void* key);

/** For each entry, calls a specified function with corresponding data as a
 * parameter.
 *
 * @param table The hash table
 * @param func The action function
 */
void ht_process(hashtable* table, void (*func) (void*));

#endif                          /* _HASH_H */
