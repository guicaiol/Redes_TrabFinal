#include <stdio.h>
#include <stdlib.h>

#include "messenger.h"

int main() {

    MESSENGER messenger;
    messenger_init(&messenger);
    messenger_start(&messenger); // blocking call
    messenger_destroy(&messenger);

    return 0;
}

