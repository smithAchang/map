#include <stdlib.h>

typedef struct {
      size_t front, rear;
      size_t capacity;
} ring_queue_base_t;

#define ring_queue_t(T)\
struct { ring_queue_base_t base; T* pq; }


#define ring_queue_init(Q, CAPACITY)\
    (Q)->base.front    = (Q)->base.rear = 0;\
    (Q)->base.capacity = (CAPACITY) + 1;\
    (Q)->pq            = malloc((Q)->base.capacity * sizeof(((Q)->pq[0])));\
    if((Q)->pq == NULL) { (Q)->base.capacity = 0; }\
    (Q)->base.capacity = (Q)->base.capacity

#define ring_queue_reset(Q)\
    (Q)->base.front    = (Q)->base.rear = 0

#define ring_queue_deinit(Q)\
    if((Q)->pq != NULL) { free((Q)->pq); }\
    (Q)->base.front    = (Q)->base.rear = (Q)->base.capacity = 0;\
    (Q)->pq            = NULL

#define ring_queue_capacity(Q)\
   ((Q)->base.capacity != 0 ? (Q)->base.capacity - 1 : 0)

#define ring_queue_len(Q)\
   (((Q)->base.capacity + (Q)->base.rear - (Q)->base.front) % (Q)->base.capacity)

#define ring_queue_empty(Q)\
   ((Q)->base.rear == (Q)->base.front)

#define ring_queue_full(Q)\
   (((Q)->base.rear + 1) % (Q)->base.capacity == (Q)->base.front)

#define ring_queue_en(Q, E, FAILURE_RETURN)\
    if(ring_queue_full(Q)) { return FAILURE_RETURN; }\
    (Q)->pq[(Q)->base.rear] = E;\
    (Q)->base.rear = ((Q)->base.rear + 1) % (Q)->base.capacity

#define ring_queue_en_no_return(Q, E) ring_queue_en(Q, E, )

#define ring_queue_de(Q, E, FAILURE_RETURN)\
    if(ring_queue_empty(Q)) { return FAILURE_RETURN; }\
    E               = (Q)->pq[(Q)->base.front];\
    (Q)->base.front = ((Q)->base.front + 1) % (Q)->base.capacity

#define ring_queue_de_no_return(Q, E) ring_queue_de(Q, E, )


typedef ring_queue_t(int)  ring_queue_int_t;
typedef ring_queue_t(int*) ring_queue_intptr_t;
