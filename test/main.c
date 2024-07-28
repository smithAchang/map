/** 
 * Copyright (c) 2023 smithAchang
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <Unity/unity.h>

#include "map.h"

typedef struct {
   union {
     unsigned char uc;
     unsigned short us;
     unsigned int ui;
     const void* pv;
   };
} Basic_Assumption_t;

static clock_t s_beginTM, s_endTM;

void setUp(void)
{
  printf("\n%s ...\n", __func__);
  s_beginTM = clock();
}

void tearDown(void)
{
  s_endTM = clock();
  printf("%s eclipsed seconds = %.3lf (s)\n", __func__, (double)(s_endTM - s_beginTM)/CLOCKS_PER_SEC);
}

void testBasicAssumption(void)
{
  
  Basic_Assumption_t  a, b;
 

  for(unsigned char i=0; i<UCHAR_MAX; ++i)
  {
       a.uc = i;
       b.pv = (const void*)i;    
       TEST_ASSERT_EQUAL_INT8_MESSAGE(a.uc, b.uc, "when assigning the box field,  the fields of the char type of the  union must be equal");
       TEST_ASSERT_EQUAL_PTR_MESSAGE(a.uc, b.pv, "the box field must be equal to  the field of the char type");
  }
  
  for(unsigned short i=0; i<USHRT_MAX; ++i)
  {
       a.us = i;
       b.pv = (const void*)i;    
       TEST_ASSERT_EQUAL_INT16_MESSAGE(a.us, b.us, "when assigning the box field,  the fields of the short type of the  union must be equal");
       TEST_ASSERT_EQUAL_PTR_MESSAGE(a.us, b.pv, "the box field must be equal to  the field of the short type");
  }


  for(unsigned int i=0; i<UINT_MAX;++i)
  {
       a.ui = i;
       b.pv = (const void*)i;    
       TEST_ASSERT_EQUAL_INT_MESSAGE(a.ui, b.ui, "when assigning the box field,  the fields of the int type of the  union must be equal");
       TEST_ASSERT_EQUAL_PTR_MESSAGE(a.ui, b.pv, "the box field must be equal to  the field of the int type");
  }

}



int strkey_intvalue_compare(const void* key, const void* value)
{

   char szPrefix[] = "testkey";
   unsigned int prefixLen = sizeof(szPrefix) - 1;
   unsigned compare = atoi(key + prefixLen);

   return  compare == *(unsigned*)value;
}


void testintmap(void)
{
  map_int_t m;
  map_init(&m);
  const unsigned int max_keys = 1 << 24;
 
  printf("%s resize map before test ...\n", __func__); 
  map_resize(&m, max_keys);
 
  char szPrefix[] = "testkey";
  unsigned int prefixLen = strlen(szPrefix);
  char szKey[32];
  
  for(unsigned int i=0 ;i < max_keys; ++i) 
  {
    snprintf(szKey, sizeof(szKey), "%s%u", szPrefix, i);
    map_set(&m, szKey, i);
  }

 
  for(unsigned int i=0 ;i < max_keys; ++i)
  {
    snprintf(szKey, sizeof(szKey), "%s%u", szPrefix, i);
    int* pv = map_get(&m, szKey);
    TEST_ASSERT_EQUAL_INT64_MESSAGE(i, *pv, "must equal when get int from map");
  }

  map_iter_t iter = map_iter(&m);

  for(unsigned int i=0 ;(map_next(&m, &iter)); ++i) {
    
    TEST_ASSERT_TRUE_MESSAGE(map_iter_acceptor_(&iter, strkey_intvalue_compare), "must equal when get value from iter loop");
    map_iter_remove(&m, &iter);
  }

  TEST_ASSERT_EQUAL_INT_MESSAGE(0, m.base.nnodes, "must be zero after removing when iterating the map");
  
  map_deinit(&m);
}

void testvoidpointermap(void)
{
   map_void_t m;
   map_init(&m);
   const unsigned int max_keys = 1 << 24;
   
   char szPrefix[] = "testkey";
   char szKey[32];
  
   for(unsigned int i=0 ;i < max_keys; ++i)
   {
     snprintf(szKey, sizeof(szKey), "%s%u", szPrefix, i);
     map_set(&m, szKey, (void*)i);
   }

 
  for(unsigned int i=0 ;i < max_keys; ++i)
  {
    snprintf(szKey, sizeof(szKey), "%s%u", szPrefix, i);
    TEST_ASSERT_EQUAL_PTR_MESSAGE((void*)i, *map_get(&m, szKey), "must equal when get ptr from map");
  }
   

   map_deinit(&m);
}



int intkey_intvalue_compare(const void* key, const void* value)
{
   return  *(unsigned*)key == *(unsigned*)value;
}

void testint2intmap(void)
{
  map_int2int_t m;
  map_init_ex(&m);
  
  const unsigned int max_keys = 1 << 24;
   
  
  for(unsigned int i=0 ;i < max_keys; ++i)
  {
    map_set_ex(&m, i, i);
  }
 
  
  for(unsigned int i=0 ;i < max_keys; ++i)
  {
    TEST_ASSERT_EQUAL_INT_MESSAGE(i, *map_get_ex(&m, i), "must equal when get int2int value from map");
  }

  map_iter_t iter = map_iter_ex(&m);

  while (map_next_ex(&m, &iter)) {
    TEST_ASSERT_TRUE_MESSAGE(map_iter_acceptor_(&iter, intkey_intvalue_compare), "must equal when get value from iter loop");
    map_iter_remove_ex(&m, &iter);
  }

  TEST_ASSERT_EQUAL_INT_MESSAGE(0, m.base.nnodes, "must be zero after removing when iterating the map");

  map_deinit_ex(&m);
}

int intcompar(const void* a, const void* b)
{
	return memcmp(a, b, sizeof(int));
}

void testint2intmapCompareWithBsearch(void)
{ 
  char szMessage[256];
  clock_t beginTM, endTM;
  map_int2int_t m;
  map_init_ex(&m);
  
  const unsigned int max_keys = 1 << 24;
  
  int* keysArray  = malloc(max_keys * sizeof(int));
  int* orderedArray  = malloc(max_keys * sizeof(int));
  
  if(!keysArray || !orderedArray)
  {
	  perror("malloc in failure!");
	  return ;
  }
  
  srand(time(NULL));
  for(unsigned int i=0 ;i < max_keys; ++i)
  {
    map_set_ex(&m, i, i);
	keysArray[i] = (int)rand();
  }
    
  printf("use knuth shuffle alg to mixture the array\n");

  for(unsigned int i= max_keys - 1; i>0; --i)
  {
	  unsigned int j = rand() %(i + 1);
	  if(j != i)
	  {
		  int temp  = keysArray[i];
		  keysArray[i] = keysArray[j];
		  keysArray[j] = temp;
	  }
  }
  memcpy(orderedArray, keysArray, max_keys * sizeof(int));
    
  
  beginTM = clock();
  qsort(orderedArray, max_keys, sizeof(int), intcompar);
  endTM = clock();
  printf("%s eclipsed seconds = %.5lf (s) for sorting the key array\n", __func__, (double)(endTM - beginTM)/CLOCKS_PER_SEC);
  
  void* found = bsearch(&keysArray[0], orderedArray, max_keys, sizeof(int), intcompar);
  TEST_ASSERT_NOT_NULL_MESSAGE(found, "must find first original key in orderedArray");
  
  found = bsearch(&keysArray[max_keys-1], orderedArray, max_keys, sizeof(int), intcompar);
  TEST_ASSERT_NOT_NULL_MESSAGE(found, "must find tail original key in orderedArray");
  
  beginTM = clock();
  for(unsigned int i=0 ;i < max_keys; ++i)
  {
    TEST_ASSERT_NOT_NULL_MESSAGE(bsearch(&keysArray[0], orderedArray, max_keys, sizeof(int), intcompar),"must equal when get int2int value from ordered array for querying all key");
  }
  endTM = clock();
  printf("%s eclipsed seconds = %.5lf (s) for query all key from ordered array\n", __func__, (double)(endTM - beginTM)/CLOCKS_PER_SEC);
  
 
  beginTM = clock();
  for(unsigned int i=0 ;i < max_keys; ++i)
  {
    map_set_ex(&m, keysArray[i], i);
  }
  endTM = clock();
  printf("%s eclipsed seconds = %.5lf (s) for constructing map\n", __func__, (double)(endTM - beginTM)/CLOCKS_PER_SEC);
  
  beginTM = clock();
  for(unsigned int i=0 ;i < max_keys; ++i)
  {
    TEST_ASSERT_NOT_NULL_MESSAGE(map_get_ex(&m, keysArray[i]), "must equal when get int2int value from map for querying all key");
  }
  endTM = clock();
  printf("%s eclipsed seconds = %.5lf (s) for query all key from map\n", __func__, (double)(endTM - beginTM)/CLOCKS_PER_SEC);

  map_deinit_ex(&m);
}

void testint2intmap_iter(void)
{

  
  map_int2int_t m;
  map_init_ex(&m);
  
  int key  = 123456;

  map_set_ex(&m, key, key);

  const int *pkey;
  map_iter_t iter = map_iter_ex(&m);

  while ((pkey = map_next_ex(&m, &iter))) {
    TEST_ASSERT_EQUAL_INT_MESSAGE(key, *map_get_ex(&m, *pkey), "value must equal when get int2int using iter");
  }

  map_deinit_ex(&m);
}

void testint2pvoidmap(void)
{
  map_int2pvoid_t m;
  map_init_ex(&m);
  
  const unsigned int max_keys = 1 << 24;
   
  
  for(unsigned int i=0 ;i < max_keys; ++i)
  {
    map_set_ex(&m, i, (void*)i);
  }
 
  
  for(unsigned int i=0 ;i < max_keys; ++i)
  {
    TEST_ASSERT_EQUAL_PTR_MESSAGE(i, *map_get_ex(&m, i), "value must equal when get int2int value from map");
  }

  map_deinit_ex(&m);
}


void testint2pvoidmap_iter(void)
{
  map_int2pvoid_t m;
  map_init_ex(&m);
 
  int key      = 123456; 
  char szKey[] = "123456";
   
  
  map_set_ex(&m, key, (void*)szKey);
 
  const void * const*pkey;
  map_iter_t iter = map_iter(&m);

  while ((pkey = map_next(&m, &iter))) {
    TEST_ASSERT_EQUAL_PTR_MESSAGE(szKey, *map_get_ex(&m, *pkey), "value must equal when get int2pvoid value from map using iter");
  }

  map_deinit_ex(&m);
}


int main(void)
{
    printf("hello world for testing!\n");
   
    UNITY_BEGIN();
    
    //RUN_TEST(testBasicAssumption);

    RUN_TEST(testintmap);
    RUN_TEST(testvoidpointermap);
    RUN_TEST(testint2intmap);
    RUN_TEST(testint2intmap_iter);
    RUN_TEST(testint2pvoidmap);
    RUN_TEST(testint2pvoidmap_iter);
	RUN_TEST(testint2intmapCompareWithBsearch);
	
    
    UNITY_END();

   return 0;
}
