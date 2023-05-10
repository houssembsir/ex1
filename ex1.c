#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <zephyr.h>

#define QUEUE_SIZE 10

static bool stop_flag = false;
static uint8_t item_count = 0;
K_SEM_DEFINE(prod_sem, QUEUE_SIZE, QUEUE_SIZE);
K_SEM_DEFINE(cons_sem, 0, QUEUE_SIZE);

struct item {
    uint8_t a;
    uint8_t b;
};

static struct item item_queue[QUEUE_SIZE];
static uint8_t queue_head = 0;
static uint8_t queue_tail = 0;

static void read_from_uart_blocking(char *buf)
{
    /* This function waits (blocks) until a string
     * ending with W is received on a UART bus.
     * This code is pseudo-code.
     * Do not look for errors in this function.
     */
    do {
        wait_for_character();
        read_character(buf++);
    } while (!new_line_received());
}

static void enqueue_item(const struct item *item)
{
    k_sem_take(&prod_sem, K_FOREVER);
    item_queue[queue_tail] = *item;
    queue_tail = (queue_tail + 1) % QUEUE_SIZE;
    k_sem_give(&cons_sem);
}

static void dequeue_item(struct item *item)
{
    k_sem_take(&cons_sem, K_FOREVER);
    *item = item_queue[queue_head];
    queue_head = (queue_head + 1) % QUEUE_SIZE;
    k_sem_give(&prod_sem);
}

static void producteur(void *p1, void *p2, void *p3)
{
    char msg[256];
    struct item new_item;
    while (!stop_flag) {
        read_from_uart_blocking(msg);
        create_item_from_str(msg, &new_item);
        enqueue_item(&new_item);
    }
}

static void producteur_start(void)
{
    /* This function starts a thread that executes
     * the function producteur(void*, void*, void*)
     * Do not look for errors in this function.
     */
    start_thread(producteur, NULL, NULL, NULL);
}

int main(void)
{
    int err;
    struct item next_item;
    producteur_start();
    while (item_count < 5) {
        dequeue_item(&next_item);
        printf("New item received: a = %d, b = %d\n",
               next_item.a, next_item.b);
        processing_item();
        item_count++;
    }
    stop_flag = true;
    return 0;
}
