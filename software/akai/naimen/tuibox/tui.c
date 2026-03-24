/*
 * demo_basic.c: a simple tuibox.h demo
 */

#include <a/tuibox.h>
/* Global UI struct */
ui_t u;

int sx = 1, sy = 1;

/* Functions that generate box contents */
void text(ui_box_t *b, char *out)
{
    sprintf(out, "%s", (char *)b->data1);
}

/* Function that runs on box click */
void click(ui_box_t *b, int x, int y)
{
    sx = x - b->x;
    sy = y - b->y;
}

/* Function that runs on box hover */
void hover(ui_box_t *b, int x, int y, int down)
{
    if (down) {
        b->x = x - sx;
        b->y = y - sy;
        ui_draw(&u);
    }
}

void stop()
{
    ui_free(&u);
    exit(0);
}

int main()
{
    char watch = 0;

    /* Initialize UI data structures */
    ui_new(0, &u);

    /* Add new UI elements to screen 0 */
    ui_text(10, 10, "Hello, world!", 0, click, hover, &u);

    /* Register an event on the q key */
    ui_key("q", stop, &u);

    /* Render the screen */
    ui_draw(&u);

 {   ui_loop(&u)
    {
        /* Check for mouse/keyboard events */
        ui_update(&u);
    }
}
    return 0;
}
