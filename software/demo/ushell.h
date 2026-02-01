#ifndef USHELL_H
#define USHELL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "libsirius/types.h"

#define USHELL_ARGC_MAX       10
#define USHELL_RX_BUFFER_SIZE 255
#define USHELL_PROMPT         "> "
#define EOL                   "\n"

typedef struct {
        const char *name;
        const char *description;
        void (*const func)(i32 argc, char *argv[]);
} ushell_command_t;

void ushell_putchar(char chr); /* define this function */
void ushell_init(const ushell_command_t *commands, usize commands_cnt);
void ushell_process(char chr);

#ifdef __cplusplus
}
#endif

#endif /* USHELL_H */
