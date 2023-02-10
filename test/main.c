/** 
 * Copyright (c) 2023 smithAchang
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<time.h>
#include <Unity/unity.h>

#include "map.h"

clock_t beginTM, endTM;

void setUp(void)
{
  printf("%s ...\n", __func__);
  beginTM = clock();
}

void tearDown(void)
{
  printf("%s ...\n", __func__);
  endTM = clock();
  printf("%s eclipsed seconds = %ld (s), ms = %ld (ms)\n", __func__, (endTM - beginTM)/CLOCKS_PER_SEC, (endTM - beginTM));
}


void testintmap(void)
{
  map_int_t m;
  map_init(&m);
  const unsigned int max_keys = 1 << 24;
 
  printf("resverve map before test ...\n"); 
  map_resize(&m, max_keys);
 
  char szPrefix[] = "testkey";
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

int main(void)
{
   printf("hello world for testing!\n");
   
    UNITY_BEGIN();
    
    RUN_TEST(testintmap);
    RUN_TEST(testvoidpointermap);
    
    UNITY_END();

   return 0;
}
