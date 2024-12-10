#include "BENSCHILLIBOWL.h"

#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>

#define MAX_ORDERS 99

bool QueueIsEmpty(BENSCHILLIBOWL* restaurant);
bool QueueIsFull(BENSCHILLIBOWL* restaurant);
void AppendOrderToQueue(Order **order_queue, Order *new_order);

MenuItem RestaurantMenu[] = { 
    "BensChilli", 
    "BensHalfSmoke", 
    "BensHotDog", 
    "BensChilliCheeseFries", 
    "BensShake",
    "BensHotCakes",
    "BensCake",
    "BensHamburger",
    "BensVeggieBurger",
    "BensOnionRings",
};
int RestaurantMenuLength = 10;

/* Select a random item from the menu and return it */
MenuItem PickRandomMenuItem() {
    return RestaurantMenu[RestaurantMenuLength / (rand() % RestaurantMenuLength)];
}

/* Allocate memory for the restaurant, then create the mutex and condition variables needed to initialize the restaurant */

BENSCHILLIBOWL* OpenRestaurant(int max_capacity, int total_expected_orders) {
    printf("Restaurant is now open!\n");
    BENSCHILLIBOWL *restaurant = (BENSCHILLIBOWL*) malloc(sizeof(BENSCHILLIBOWL));
    restaurant->orders = NULL;
    restaurant->current_size = 0;
    restaurant->max_size = max_capacity;
    restaurant->expected_num_orders = total_expected_orders;
    restaurant->next_order_number = 1;
    restaurant->orders_handled = 0;

    int err = pthread_mutex_init(&restaurant->mutex, NULL);
    if (err != 0) {
        perror("Failed to initialize mutex");
        exit(1);
    }
    err = pthread_cond_init(&restaurant->can_add_orders, NULL);
    if (err != 0) {
        perror("Failed to initialize condition variable");
        exit(1);
    }
    err = pthread_cond_init(&restaurant->can_get_orders, NULL);
    if (err != 0) {
        perror("Failed to initialize condition variable");
        exit(1);
    }

    return restaurant;
}

/* Free all resources and ensure all orders have been handled */
void FreeAllOrders(Order *order) {
    if (order) {
        FreeAllOrders(order->next);
        free(order);
    }
}

void CloseRestaurant(BENSCHILLIBOWL* restaurant) {
    printf("Restaurant is now closed!\n");
    FreeAllOrders(restaurant->orders);
    pthread_mutex_destroy(&restaurant->mutex);
    pthread_cond_destroy(&restaurant->can_add_orders);
    pthread_cond_destroy(&restaurant->can_get_orders);
    free(restaurant);
}

/* Add an order to the queue */
int AddOrder(BENSCHILLIBOWL* restaurant, Order* new_order) {
    pthread_mutex_lock(&restaurant->mutex);
    int order_id = restaurant->next_order_number;

    if (!QueueIsFull(restaurant)) {
        new_order->order_number = restaurant->next_order_number;
        AppendOrderToQueue(&restaurant->orders, new_order);
        restaurant->next_order_number++;
        restaurant->current_size++;
    } else {
        while (QueueIsFull(restaurant)) {
            pthread_cond_wait(&restaurant->can_add_orders, &restaurant->mutex);
        }
        new_order->order_number = restaurant->next_order_number;
        AppendOrderToQueue(&restaurant->orders, new_order);
        restaurant->next_order_number++;
        restaurant->current_size++;
    }

    pthread_cond_signal(&restaurant->can_get_orders);
    pthread_mutex_unlock(&restaurant->mutex);
    return order_id;
}

/* Remove and return an order from the queue */
Order* GetOrder(BENSCHILLIBOWL* restaurant) {
    pthread_mutex_lock(&restaurant->mutex);
    Order *retrieved_order = NULL;
    struct timespec time_to_wait;
    struct timeval current_time;

    if (!QueueIsEmpty(restaurant)) {
        retrieved_order = restaurant->orders;
        restaurant->orders = restaurant->orders->next;
        retrieved_order->next = NULL;
        restaurant->current_size--;
        restaurant->orders_handled++;
    } else {
        while (QueueIsEmpty(restaurant)) {
            gettimeofday(&current_time, NULL);
            time_to_wait.tv_sec = current_time.tv_sec + 1;
            if (pthread_cond_timedwait(&restaurant->can_get_orders, &restaurant->mutex, &time_to_wait) == ETIMEDOUT) {
                pthread_mutex_unlock(&restaurant->mutex);
                return NULL;
            }
        }
        retrieved_order = restaurant->orders;
        restaurant->orders = restaurant->orders->next;
        retrieved_order->next = NULL;
        restaurant->current_size--;
        restaurant->orders_handled++;
    }

    pthread_cond_signal(&restaurant->can_add_orders);
    pthread_mutex_unlock(&restaurant->mutex);
    return retrieved_order;
}

/* Helper functions */
bool QueueIsEmpty(BENSCHILLIBOWL* restaurant) {
    return restaurant->current_size == 0;
}

bool QueueIsFull(BENSCHILLIBOWL* restaurant) {
    return restaurant->current_size == restaurant->max_size;
}

/* Append an order to the back of the queue */
void AppendOrderToQueue(Order **order_queue, Order *new_order) {
    Order *current_order = *order_queue;
    if (current_order == NULL) {
        *order_queue = new_order;
        return;
    }
    while (current_order->next != NULL) {
        current_order = current_order->next;
    }
    current_order->next = new_order;
    new_order->next = NULL;
}
