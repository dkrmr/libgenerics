/*
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * For more information, please refer to <http://unlicense.org/>
 */
#include "trie.h"

/*
 * Auxiliar function;
 * find the node mapped by `string`, if necessary, allocates
 *
 * @return the node mapped with string and allocate
 */
tnode_t* trie_get_node_or_allocate (
		struct trie_t* t,
		void* string,
		size_t size)
{
	if(!t || !string) return NULL;
	struct tnode_t* node = &t->root;
	size_t i;
	char *ptr = string;

	for(i=0; i<size; i++){
		int byte = (int)ptr[i];
		if ( node->children[byte] == NULL ){
			node->children[byte] = (tnode_t*)malloc(sizeof(tnode_t));
			node->children[byte]->value = NULL;

			int j;
			for(j=0; j<NBYTE; j++)
				node->children[byte]->children[i] = NULL;

		}

		node = node->children[byte];
	}
	return node;
}

/*
 * Auxiliar function;
 * find the node mapped by `string`
 *
 * @return	the node mapped with `string` or null case the mapped
 * 		does not exists.
 */
tnode_t* node_at ( struct trie_t* t, void* string, size_t size)
{
	if(!t || !string) return NULL;
	struct tnode_t* node = &t->root;
	size_t i;
	char *ptr = string;

	for(i=0; i<size; i++){
		unsigned char byte = (unsigned char)ptr[i];

		if ( node->children[byte] == NULL )
			return NULL;

		node = node->children[byte];
	}
	return node;
}

/** Inicialize structure `t` with `member_size` size.
  * The t has to be allocated.
  *
  * @param t		pointer to the allocated struct trie_t;
  * @param member_size	size in bytes of the indexed elements
  * 			by the trie.
  */
gerror_t trie_create (struct trie_t* t, size_t member_size)
{
	if(!t) return GERROR_NULL_STRUCTURE;

	t->size = 0;
	t->member_size = member_size;
	t->root.value = NULL;
	
	int i;
	for(i=0; i<NBYTE; i++)
		t->root.children[i] = NULL;

	return GERROR_OK;
}

/*
 * auxiliar function.
 * destroy a node recursively.
 */
void trie_destroy_tnode (struct tnode_t* node)
{
	if( node ){
		int i;
		for(i=0; i<NBYTE; i++)
			trie_destroy_tnode(node->children[i]);

		if( node->value )
			free(node->value);

		free(node);
	}
}

/** Destroy the members pointed by `t`.
  * The structure is not freed.
  *
  * @return	GERROR_OK in case of success operation;
  * 		GERROR_NULL_STRUCURE in case `t` is a NULL
  */
gerror_t trie_destroy (struct trie_t* t)
{
	if(!t) return GERROR_NULL_STRUCTURE;

	int i;
	
	if(t->root.value)
		free(t->root.value);
	
	for(i=0; i<NBYTE; i++){
		trie_destroy_tnode(t->root.children[i]);
		t->root.children[i] = NULL;
	}
	t->size = 0;
	t->member_size = 0;

	return GERROR_OK;
}

/** Adds the `elem` and maps it with the `string` with size `size`.
  * This function overwrite any data left in the trie mapped with string.
  *
  * @param t		pointer to the trie structure;
  * @param string	pointer to the string of bytes to map elem;
  * @param size		size of the string of bytes
  * @param elem		pointer to the element to add
  */
gerror_t trie_add_element (struct trie_t* t, void* string, size_t size, void* elem)
{
	if(!t) return GERROR_NULL_STRUCTURE;
	struct tnode_t* node = trie_get_node_or_allocate(t, string, size);

	if(node->value == NULL)
		node->value = malloc(t->member_size);

	if(t->member_size && string)
		memcpy(node->value, elem, t->member_size);
	t->size++;

	return GERROR_OK;
}

/** Removes the element mapped by `string`.
  *
  * @param t		pointer to the structure trie_t;
  * @param string	pointer to the string of bytes to map elem;
  * @param size		size of the string of bytes.
  *
  * @return	GERROR_OK in case of success operation;
  * 		GERROR_NULL_STRUCURE in case `t` is a NULL
  * 		GERROR_OUT_OF_BOUND the `elem` does not exist
  * 		in `string map`
  */
gerror_t trie_remove_element (struct trie_t* t, void* string, size_t size)
{
	if(!t) return GERROR_NULL_STRUCTURE;

	struct tnode_t* node = node_at(t, string, size);
	if(!node) return GERROR_ACCESS_OUT_OF_BOUND;

	void* removed_value = node->value;

	node->value = NULL;

	t->size--;
	void* ptr = removed_value;
	if(ptr)
		free(ptr);
	return GERROR_OK;
}

/** Returns the element mapped by `string`. If the map does not 
  * exist, returns NULL.
  *
  * @param t		pointer to the structure;
  * @param string	pointer to the string of bytes to map elem;
  * @param size		size of the string of bytes.
  * @param elem		pointer to the memory allocated that
  * 			will be write with the elem mapped by `string`
  *
  * @return	GERROR_OK in case of success operation;
  * 		GERROR_NULL_STRUCURE in case `t` is a NULL
  */
gerror_t trie_get_element (struct trie_t* t, void* string, size_t size, void* elem)
{
	if(!t) return GERROR_NULL_STRUCTURE;

	struct tnode_t* node = node_at(t, string, size);

	if(node == NULL)
		return GERROR_ACCESS_OUT_OF_BOUND;

	if(t->member_size)
		memcpy(elem, node->value, t->member_size);

	return GERROR_OK;
}

/** Sets the value mapped by `string`.
  * Encapsulates the remove and add functions.
  *
  * @param t		pointer to the structure;
  * @param string	pointer to the string of bytes to map elem;
  * @param size		size of the string of bytes.
  * @param elem		pointer to the element to add
  */
gerror_t trie_set_element (struct trie_t* t, void* string, size_t size, void* elem)
{

	if(!t) return GERROR_NULL_STRUCTURE;
		
	struct tnode_t* node = node_at(t, string, size);
	if(node){
		if(t->member_size)
			memcpy(node->value, elem, t->member_size);

	}

	return GERROR_OK;
}
