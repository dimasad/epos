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
        "  %s read [portpath] [index] [subindex] [nodeid]\n"
        "  %s init [portpath]\n"
        "  %s move [portpath] [target]\n"
        "  %s pos [portpath] [target]\n"
        "  %s vel [portpath] [target]\n";
    
    fprintf(stderr, usage, progname, progname, progname, progname, progname);
}


void write_cmd(char *argv[]) {
    HANDLE file = epos_open_port(argv[2]);
    if (!file) {
        fprintf(stderr, "Error opening port, aborting.\n");
        return;
    }
    
    char *end;
    uint16_t index = strtol(argv[3], &end, 0);
    if (end == argv[3]) {
        fprintf(stderr, "Error parsing index, aborting.\n");
        return;
    }
    
    uint8_t subindex = strtol(argv[4], &end, 0);
    if (end == argv[4]) {
        fprintf(stderr, "Error parsing subindex, aborting.\n");
        return;
    }

    uint8_t nodeid = strtol(argv[5], &end, 0);
    if (end == argv[5]) {
        fprintf(stderr, "Error parsing nodeid, aborting.\n");
        return;
    }

    uint32_t value = strtol(argv[6], &end, 0);
    if (end == argv[6]) {
        fprintf(stderr, "Error parsing value, aborting.\n");
        return;
    }
    
    if (epos_write_object(file, index, subindex, nodeid, value))
        fprintf(stderr, "Error writing object.\n");        
}


void read_cmd(char *argv[]) {
    HANDLE file = epos_open_port(argv[2]);
    if (!file) {
        fprintf(stderr, "Error opening port, aborting.\n");
        return;
    }
    
    char *end;
    uint16_t index = strtol(argv[3], &end, 0);
    if (end == argv[3]) {
        fprintf(stderr, "Error parsing index, aborting.\n");
        return;
    }
    
    uint8_t subindex = strtol(argv[4], &end, 0);
    if (end == argv[4]) {
        fprintf(stderr, "Error parsing subindex, aborting.\n");
        return;
    }

    uint8_t nodeid = strtol(argv[5], &end, 0);
    if (end == argv[5]) {
        fprintf(stderr, "Error parsing nodeid, aborting.\n");
        return;
    }

    int32_t value;
    if (epos_read_object(file, index, subindex, nodeid, (uint32_t*) &value)) {
        fprintf(stderr, "Error read object.\n");
        return;
    }
    printf("read value: %d [0x%08x]\n", value, value);
}


void init_cmd(char *argv[]) {
    HANDLE file = epos_open_port(argv[2]);
    if (!file) {
        fprintf(stderr, "Error opening port, aborting.\n");
        return;
    }
    
    if (epos_fault_reset(file, 0)) {
        fprintf(stderr, "Error in fault reset command.\n");
        return;
    }

    if (epos_shutdown(file, 0)) {
        fprintf(stderr, "Error in shutdown command.\n");
        return;
    }

    if (epos_switch_on(file, 0)) {
        fprintf(stderr, "Error in switch on command.\n");
        return;
    }

    if (epos_enable_operation(file, 0)) {
        fprintf(stderr, "Error in enable operation command.\n");
        return;
    }
}


void move_cmd(char *argv[]) {
    HANDLE file = epos_open_port(argv[2]);
    if (!file) {
        fprintf(stderr, "Error opening port, aborting.\n");
        return;
    }
    
    char *end;
    int32_t target = strtol(argv[3], &end, 0);
    if (end == argv[3]) {
        fprintf(stderr, "Error parsing target position, aborting.\n");
        return;
    }

    if (epos_set_mode(file, 0, EPOS_PROFILE_POSITION_MODE)) {
        fprintf(stderr, "Error setting mode, aborting.\n");
        return;
    }

    if (epos_set_target_position(file, 0, target)) {
        fprintf(stderr, "Error setting target position, aborting.\n");
        return;
    }

    if (epos_goto_position_rel(file, 0)) {
        fprintf(stderr, "Error sending go to position command, aborting.\n");
        return;
    }
}


void pos_cmd(char *argv[]) {
    HANDLE file = epos_open_port(argv[2]);
    if (!file) {
        fprintf(stderr, "Error opening port, aborting.\n");
        return;
    }
    
    char *end;
    int32_t target = strtol(argv[3], &end, 0);
    if (end == argv[3]) {
        fprintf(stderr, "Error parsing position setpoint, aborting.\n");
        return;
    }

    if (epos_set_mode(file, 0, EPOS_POSITION_MODE)) {
        fprintf(stderr, "Error setting mode, aborting.\n");
        return;
    }

    if (epos_write_object(file, EPOS_POSITION_MODE_SP_INDEX, 0, 0, target)) {
        fprintf(stderr, "Error setting position setpoint, aborting.\n");
        return;
    }
}


void vel_cmd(char *argv[]) {
    HANDLE file = epos_open_port(argv[2]);
    if (!file) {
        fprintf(stderr, "Error opening port, aborting.\n");
        return;
    }
    
    char *end;
    int32_t target = strtol(argv[3], &end, 0);
    if (end == argv[3]) {
        fprintf(stderr, "Error parsing target velocity, aborting.\n");
        return;
    }

    if (epos_set_mode(file, 0, EPOS_VELOCITY_MODE)) {
        fprintf(stderr, "Error setting mode, aborting.\n");
        return;
    }

    if (epos_write_object(file, EPOS_VELOCITY_MODE_SP_INDEX, 0, 0, target)) {
        fprintf(stderr, "Error setting velocity setpoint, aborting.\n");
        return;
    }
}


int main(int argc, char *argv[]) {
    if (argc == 7 && strcmp(argv[1], "write") == 0) {
        write_cmd(argv);
    } else if (argc == 6 && strcmp(argv[1], "read") == 0) {
        read_cmd(argv);
    } else if (argc == 3 && strcmp(argv[1], "init") == 0) {
        init_cmd(argv);
    } else if (argc == 4 && strcmp(argv[1], "move") == 0) {
        move_cmd(argv);
    } else if (argc == 4 && strcmp(argv[1], "pos") == 0) {
        pos_cmd(argv);
    } else if (argc == 4 && strcmp(argv[1], "vel") == 0) {
        vel_cmd(argv);
    } else {
        usage(argv[0]);
    }
    
    return 0;
}
