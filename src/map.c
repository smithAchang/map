/** 
 * Copyright (c) 2014 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include <stdlib.h>
#include <string.h>
#include "map.h"

struct map_node_t {
  unsigned hash;
  void *value;
  map_node_t *next;
  unsigned char key[];
  /* char value[]; *//*must skip the key field by ksize*/
};


static unsigned map_hash(const char *str) {
  unsigned hash = 5381;
  while (*str) {
    /*using the Time33 | djb2 hash alg; ^ operator will produce more collisions*/
    hash = ((hash << 5) + hash) + *str++;
  }
  return hash;
}


static map_node_t *map_newnode(const void *key, unsigned map_keysize, void *value, unsigned vsize) {
  map_node_t *node;
  unsigned ksize;
  
  if(map_keysize == 0) {
    ksize = strlen((const char*)key) + 1;
  }
  else {
    /*consider as primitive types, create box for them*/
    ksize = sizeof(void*);
    if(map_keysize > ksize) return NULL;
  }
  
  /*padding for machine byte alignment*/
  unsigned voffset = ksize + ((sizeof(void*) - ksize) % sizeof(void*));
  node = malloc(sizeof(*node) + voffset + vsize);
  if (!node) return NULL;
  
  if(map_keysize == 0) {
     memcpy(node->key, key, ksize);
     node->hash = map_hash(key);
  }
  else {
    /*consider as primitive types*/
    *(const void**)(node->key) = key;
    node->hash                 = (unsigned)key;
  }
  
  node->value = ((char*) (node->key)) + voffset;
  memcpy(node->value, value, vsize);
  return node;
}


static int map_bucketidx(map_base_t *m, unsigned hash) {
  /* If the implementation is changed to allow a non-power-of-2 bucket count,
   * the line below should be changed to use mod instead of AND */
  return hash & (m->nbuckets - 1);
}


static void map_addnode(map_base_t *m, map_node_t *node) {
  int n         = map_bucketidx(m, node->hash);
  node->next    = m->buckets[n];
  m->buckets[n] = node;
}


static void map_remove_node(map_base_t *m, map_node_t** pre_next) {
  map_node_t* node  = *pre_next;
  *pre_next         = node->next;
  free(node);
  m->nnodes--;  
}


int map_resize_(map_base_t *m, unsigned nbuckets) {
  map_node_t *nodes, *node, *next;
  map_node_t **buckets;
  int i; 
  /* MUST: Chain all nodes together before realloc*/
  nodes = NULL;
  i = m->nbuckets;
  while (i--) {
    node = (m->buckets)[i];
    while (node) {
      next = node->next;
      node->next = nodes;
      nodes = node;
      node = next;
    }
  }
  /* Reset buckets */
  buckets = realloc(m->buckets, sizeof(*m->buckets) * nbuckets);
  if (buckets != NULL) {
    m->buckets = buckets;
    m->nbuckets = nbuckets;
  }
  if (m->buckets) {
    memset(m->buckets, 0, sizeof(*m->buckets) * m->nbuckets);
    /* Re-add nodes to buckets */
    node = nodes;
    while (node) {
      next = node->next;
      map_addnode(m, node);
      node = next;
    }
  }
  /* Return error code if realloc() failed */
  return (buckets == NULL) ? -1 : 0;
}

/* return head's or next field's pointer of predecessor for modifying the field in future */
static map_node_t **map_getref(map_base_t *m, const void *key) {
  const unsigned ksize = m->nkeysize;
  const unsigned hash  = ksize == 0 ?  map_hash(key) : (unsigned)key;
  map_node_t **next;
  if (m->nbuckets > 0) {
    next = &m->heads[map_bucketidx(m, hash)];
    while (*next) {
      if ((*next)->hash == hash) {
        if(ksize == 0 && !strcmp((const char*) ((*next)->key), key)) {
           return next;
        }
        else if(key == *(const void**)((*next)->key)) {
           return next;
        } 
      }
      next = &(*next)->next;
    }
  }
  return NULL;
}


void map_deinit_(map_base_t *m) {
  map_node_t *next, *node;
  int i;
  i = m->nbuckets;
  while (i--) {
    node = m->buckets[i];
    while (node) {
      next = node->next;
      free(node);
      node = next;
    }
  }
  free(m->buckets);
}


void *map_get_(map_base_t *m, const void *key) {
  map_node_t **next = map_getref(m, key);
  return next ? (*next)->value : NULL;
}


int map_set_(map_base_t *m, const void *key, void *value, unsigned vsize) {
  int err;
  unsigned n;
  map_node_t **next, *node;
  /* Find & replace existing node */
  next = map_getref(m, key);
  if (next) {
    memcpy((*next)->value, value, vsize);
    return 0;
  }
  
  /* Add new node */
  node = map_newnode(key, m->nkeysize, value, vsize);
  if (node == NULL) goto fail;
  if (m->nnodes >= m->nbuckets) {
    if(m->nbuckets == 0) {
       /*when size is small enough, you can use vector. so the default value is 256*/
       n  = 256;
    }
    else {
      const unsigned tempCalc = m->nbuckets << 1;
      if(tempCalc == 0) {
        /*max boundary*/
        goto fail;
      }
      else {
        /*size must be power of 2*/
        n = tempCalc;
      }
    }
    
    err = map_resize_(m, n);
    if (err) goto fail;
  }
  map_addnode(m, node);
  m->nnodes++;
  return 0;
  fail:
  if (node) free(node);
  return -1;
}


void map_remove_(map_base_t *m, const void *key) {
  map_node_t **next = map_getref(m, key);
  if (next) {
    map_remove_node(m, next);
  }
}


map_iter_t map_iter_(void) {
  map_iter_t iter;
  iter.bucketidx = -1;
  iter.node      = NULL;
  iter.opaque    = NULL;
  return iter;
}


const void *map_next_(map_base_t *m, map_iter_t *iter) {
  if (iter->opaque) {
    iter->node   = (map_node_t*)iter->opaque;
    iter->opaque = NULL;
    if (iter->node == NULL) goto nextBucket;
  } 
  else if (iter->node) {
    iter->node = iter->node->next;
    if (iter->node == NULL) goto nextBucket;
  } else {
    nextBucket:
    do {
      if (++iter->bucketidx >= m->nbuckets) {
        return NULL;
      }
      iter->node = m->nexts[iter->bucketidx];
    } while (iter->node == NULL);
  }
  /*pointer to key field*/
  return (const void*) (iter->node->key);
}


int map_iter_acceptor_(map_iter_t *iter, Iter_Visitor_Func cb) {
    return cb(iter->node->key, iter->node->value);
}


map_iter_t* map_iter_remove_(map_base_t *m, map_iter_t *iter) {
  if(iter->opaque) return NULL;

  if (iter->node) {
     const void*  key = iter->node->key;
     if(m->nkeysize != 0) {
        key = *(const void**)iter->node->key;
     }

     map_node_t **predecessor_next_ptr = map_getref(m, key);
     if (predecessor_next_ptr) {
       map_remove_node(m, predecessor_next_ptr);
       /*store successor node for next operation after removal*/
       iter->opaque      = *predecessor_next_ptr;
       iter->node        = NULL;
     } 
  }
  return iter;
}
