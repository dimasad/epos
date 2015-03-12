/**
 * @file epos2cmd.c
 * @brief Send commands to EPOS2 70/10.
 */

#include "epos2.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void usage(const char *progname) {
    const char *usage =
        "Usage: \n"
        "  %s write [portpath] [index] [subindex] [nodeid] [value]\n"
        "  %s read [portpath] [index] [subindex] [nodeid]\n";
    fprintf(stderr, usage, progname, progname);
}


void write_cmd(char *argv[]) {
    HANDLE file = epos_open_port(argv[2]);
    if (file == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error opening port, aborting.\n");
        return;
    }
    
    char *end;
    uint16_t index = strtol(argv[3], &end, 10);
    if (end == argv[3]) {
        fprintf(stderr, "Error parsing index, aborting.\n");
        return;
    }
    
    uint8_t subindex = strtol(argv[4], &end, 10);
    if (end == argv[4]) {
        fprintf(stderr, "Error parsing subindex, aborting.\n");
        return;
    }

    uint8_t nodeid = strtol(argv[5], &end, 10);
    if (end == argv[5]) {
        fprintf(stderr, "Error parsing nodeid, aborting.\n");
        return;
    }

    uint32_t value = strtol(argv[6], &end, 10);
    if (end == argv[6]) {
        fprintf(stderr, "Error parsing value, aborting.\n");
        return;
    }
    
    if (epos_write_object(file, index, subindex, nodeid, value))
        fprintf(stderr, "Error writing object.\n");        
}


void read_cmd(char *argv[]) {
    HANDLE file = epos_open_port(argv[2]);
    if (file == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error opening port, aborting.\n");
        return;
    }
    
    char *end;
    uint16_t index = strtol(argv[3], &end, 10);
    if (end == argv[3]) {
        fprintf(stderr, "Error parsing index, aborting.\n");
        return;
    }
    
    uint8_t subindex = strtol(argv[4], &end, 10);
    if (end == argv[4]) {
        fprintf(stderr, "Error parsing subindex, aborting.\n");
        return;
    }

    uint8_t nodeid = strtol(argv[5], &end, 10);
    if (end == argv[5]) {
        fprintf(stderr, "Error parsing nodeid, aborting.\n");
        return;
    }

    int32_t value;
    if (epos_read_object(file, index, subindex, nodeid, (uint32_t*) &value)) {
        fprintf(stderr, "Error read object.\n");
        return;
    }
    printf("read value: %d [%08x]\n", value, value);
}


int main(int argc, char *argv[]) {
    if (argc == 7 && strcmp(argv[1], "write") == 0) {
        write_cmd(argv);
    } else if (argc == 6 && strcmp(argv[1], "read") == 0) {
        read_cmd(argv);
    } else {
        usage(argv[0]);
    }
    
    return 0;
}
