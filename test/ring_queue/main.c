#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <sys/time.h>
#include <string.h>
#include <unity.h>

#include "ring_queue.h"


#define USEC_UNIT (1000000)
#define TEST_LOOP (10000000)

#ifndef likely
  #define likely(x) __buildin_expect(!!(x), 0)
#endif

#ifndef unlikely
  #define unlikely(x) __buildin_expect(!!(x), 0)
#endif

/*declare firstly .this can avoid the difference between declaration and implementation*/
double getDiff(const struct timeval *start,const struct timeval *end);

/*implemention*/
double getDiff(const struct timeval *start,const struct timeval *end)
 {

  unsigned long diff =
               (end->tv_sec - start->tv_sec) * USEC_UNIT
             + (end->tv_usec - start->tv_usec);

  return (double)diff/USEC_UNIT;
 }


void testMacroRet()
{

  ring_queue_int_t  ai;
  ring_queue_init(&ai, 3);

  int i = 0, j;
  ring_queue_en(&ai, i,);
  TEST_ASSERT_EQUAL_UINT_MESSAGE(1, ring_queue_len(&ai), "len must be one");
  ring_queue_en_no_return(&ai, 2);
  TEST_ASSERT_EQUAL_UINT_MESSAGE(2, ring_queue_len(&ai), "len must be two");
  ring_queue_en_no_return(&ai, 3);
  TEST_ASSERT_EQUAL_UINT_MESSAGE(3, ring_queue_len(&ai), "len must be three");

  TEST_ASSERT_TRUE_MESSAGE(ring_queue_full(&ai), "queue must be full");

  ring_queue_de_no_return(&ai, j);
  printf("%s dequeue: %d\n", __func__, j);
  ring_queue_de(&ai, j, );
  printf("%s dequeue: %d\n", __func__, j);

  ring_queue_reset(&ai);
  ring_queue_deinit(&ai);

}


int testMacroRet2()
{

  ring_queue_int_t  ai;
  ring_queue_init(&ai, 3);

  int i = 0,j;
  ring_queue_en(&ai, i, -1);

  ring_queue_en(&ai, 2, -1);
  ring_queue_en(&ai, 3, -1);

  ring_queue_de(&ai, j, -1);
  printf("%s dequeue: %d\n", __func__, j);

  ring_queue_reset(&ai);
  ring_queue_deinit(&ai);

  return 0;

}


void testQueueFull()
{

  ring_queue_int_t  ai;
  ring_queue_init(&ai, 3);

  int i = 0, j;
  ring_queue_en(&ai, i,);
  TEST_ASSERT_EQUAL_UINT_MESSAGE(1, ring_queue_len(&ai), "len must be one");
  ring_queue_en_no_return(&ai, 2);
  TEST_ASSERT_EQUAL_UINT_MESSAGE(2, ring_queue_len(&ai), "len must be two");
  ring_queue_en_no_return(&ai, 3);
  TEST_ASSERT_EQUAL_UINT_MESSAGE(3, ring_queue_len(&ai), "len must be three");

  TEST_ASSERT_TRUE_MESSAGE(ring_queue_full(&ai), "queue must be full");

  ring_queue_en_no_return(&ai, 4);
  TEST_ASSERT_FALSE_MESSAGE(1, "flow must not reach here");

  ring_queue_deinit(&ai);

}

void testQueueEmpty()
{

  ring_queue_int_t  ai;
  ring_queue_init(&ai, 3);

  TEST_ASSERT_TRUE_MESSAGE(ring_queue_empty(&ai), "queue must be empty");

  int i = 0, j;
  ring_queue_en(&ai, i,);
  TEST_ASSERT_EQUAL_UINT_MESSAGE(1, ring_queue_len(&ai), "len must be one");
  ring_queue_en_no_return(&ai, 2);
  TEST_ASSERT_EQUAL_UINT_MESSAGE(2, ring_queue_len(&ai), "len must be two");
  ring_queue_en_no_return(&ai, 3);
  TEST_ASSERT_EQUAL_UINT_MESSAGE(3, ring_queue_len(&ai), "len must be three");

  TEST_ASSERT_TRUE_MESSAGE(ring_queue_full(&ai), "queue must be full");

  ring_queue_de_no_return(&ai, j);
  printf("dequeue %d\n", j);
  
  ring_queue_de_no_return(&ai, j);
  printf("dequeue %d\n", j);

  ring_queue_de_no_return(&ai, j);
  printf("dequeue %d\n", j);
  TEST_ASSERT_TRUE_MESSAGE(ring_queue_empty(&ai), "queue must be empty");

  ring_queue_de_no_return(&ai, j);
  TEST_ASSERT_FALSE_MESSAGE(1, "flow must not reach here");

  ring_queue_deinit(&ai);

}

void setUp(void)
{
   printf("\n+ %-26s begin to run ^^^\n", Unity.CurrentTestName);
}

void tearDown(void)
{
  printf("\n- %-26s end to run   $$$\n", Unity.CurrentTestName);
}

struct ATest {
  ring_queue_int_t q_int;
  ring_queue_intptr_t q_intptr;
};


void testAsStructMbr()
{

  struct ATest a;

  ring_queue_init(&a.q_int, 123);
  ring_queue_init(&a.q_intptr, 127);

  TEST_ASSERT_EQUAL_UINT_MESSAGE(123, ring_queue_capacity(&a.q_int), "capacity must be 123");
  TEST_ASSERT_EQUAL_UINT_MESSAGE(127, ring_queue_capacity(&a.q_intptr), "capacity must be 127");

  int *w = (int*)65;
  ring_queue_en_no_return(&a.q_int, 256);
  ring_queue_en_no_return(&a.q_intptr, w);

  int x, *y;
  ring_queue_de_no_return(&a.q_int, x);
  TEST_ASSERT_EQUAL_INT_MESSAGE(256, x, "dequeue got value must be 256");

  ring_queue_de_no_return(&a.q_intptr, y);
  TEST_ASSERT_EQUAL_INT_MESSAGE(w, y, "dequeue got value must be 65");

  ring_queue_deinit(&a.q_int);
  ring_queue_deinit(&a.q_intptr);
}

void testintptrqueue()
{

  ring_queue_intptr_t ai;

  ring_queue_init(&ai, 256);
  
  TEST_ASSERT_EQUAL_UINT_MESSAGE(256, ring_queue_capacity(&ai), "capacity must be 256");
  
  int *i = (int*)256, *j  = (int*)64;
  
  ring_queue_en_no_return(&ai, i);
  ring_queue_en_no_return(&ai, j);

  int *k, *h;
  ring_queue_de_no_return(&ai, k);
  TEST_ASSERT_EQUAL_PTR_MESSAGE(i, k, "dequeue got value must be 256");

  ring_queue_de_no_return(&ai, h);
  TEST_ASSERT_EQUAL_PTR_MESSAGE(j, h, "dequeue got value must be 64");

  ring_queue_deinit(&ai);

}

void testLoopQueue()
{
  unsigned int capacity = 3; 
  ring_queue_int_t  ai;
  ring_queue_init(&ai, capacity);

  for(unsigned int i=0; i< (3*capacity); ++i)
  {
     ring_queue_en_no_return(&ai, i); 
     TEST_ASSERT_EQUAL_UINT_MESSAGE(1, ring_queue_len(&ai), "len must be one"); 
     int j;
     ring_queue_de_no_return(&ai, j); 
     TEST_ASSERT_TRUE_MESSAGE(ring_queue_empty(&ai), "queue must be empty"); 
     TEST_ASSERT_EQUAL_UINT_MESSAGE(i, j, "queue value must be equal"); 
  }
  
  ring_queue_deinit(&ai);
}

int main(void)
{
  struct timeval start, end;
  gettimeofday(&start, NULL);

  testMacroRet();
  testMacroRet2();


  UNITY_BEGIN();

  RUN_TEST(testLoopQueue);
  RUN_TEST(testintptrqueue);
  RUN_TEST(testAsStructMbr);
  RUN_TEST(testQueueEmpty);
  RUN_TEST(testQueueFull);

  UNITY_END();


  gettimeofday(&end, NULL);
  printf("\nstartof {0}, time: %ld.%ld \nendof   {0}, time: %ld.%ld \ndiff: %lf\n", start.tv_sec, start.tv_usec, end.tv_sec, end.tv_usec, getDiff(&start, &end) );

  return 0;
}
