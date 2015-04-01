/**
 * @file epos2.c
 * @brief Implementation of Maxon Motor EPOS2 serial port communication driver.
 */

#include "epos2.h"

#include <io.h>

#include <stdio.h>
#include <string.h>


enum {
    TIMEOUT_MS = 100,
    SUCCESS = 0,
    FAIL = -1
};

enum {
    READ_OBJECT_OPCODE = 0x10,
    WRITE_OBJECT_OPCODE = 0x11
} epos_opcode;


//  --- Utility functions --- //

static int fail(const char *msg) {
#ifdef DEBUG
    fprintf(stderr, "%s\n", msg);
#endif//DEBUG
    return FAIL;
}


static int fail_code(const char *msg, uint32_t code) {
#ifdef DEBUG
    fprintf(stderr, "%s: '%08x'\n", msg, code);
#endif//DEBUG
    return FAIL;
}


static int timeout_read(HANDLE file, void *buf, size_t len) {
    DWORD read;
    if (!ReadFile(file, buf, len, &read, 0) || read != len)
        return fail("Not all data received before timeout.");
    
    return SUCCESS;
}


static void flush_buffers(HANDLE file) {
    //TODO
}


static uint16_t pack_le_uint16(uint8_t *buf) {
    uint16_t ret = buf[0];
    ret += (uint16_t) buf[1] << 8;
    
    return ret;
}


static uint32_t pack_le_uint32(uint8_t *buf) {
    uint32_t ret = buf[0];
    ret += (uint32_t) buf[1] << 8;
    ret += (uint32_t) buf[2] << 16;
    ret += (uint32_t) buf[3] << 24;
    
    return ret;
}


//  --- Protocol functions --- //

static uint16_t crc_byte(uint16_t crc, uint8_t data);
static uint16_t crc_data(uint16_t crc, uint8_t* data, int len);


int send_frame(HANDLE file, uint8_t opcode, size_t len, uint8_t *data) {
    DWORD written;
    if (!WriteFile(file, &opcode, sizeof opcode, &written, 0) || written != 1)
        return fail("Error writing opcode.");
    
    uint8_t ready_ack;
    if (timeout_read(file, &ready_ack, sizeof ready_ack))
        return fail("Timeout waiting for ready ack.");
    if (ready_ack == 'F')
        return fail("Epos not ready to receive.");
    if (ready_ack != 'O') {
        return fail_code("Unrecognized ack received", ready_ack);
    }
    
    uint8_t len_minus_1 = len / 2 - 1;
    if (!WriteFile(file, &len_minus_1, 1, &written, 0) || written != 1)
        return fail("Error writing message length.");
    
    if (!WriteFile(file, data, len, &written, 0) || written != len) {
        return fail("Error writing message data.");
    }

    uint16_t crc = crc_byte(0, opcode);
    crc = crc_byte(crc, len_minus_1);
    crc = crc_data(crc, data, len);
    
    uint8_t crc_bytes[2] = {crc, crc >> 8};
    if (!WriteFile(file, crc_bytes, 2, &written, 0) || written != 2)
        return fail("Error writing crc.");
    
    uint8_t end_ack;
    if (timeout_read(file, &end_ack, sizeof end_ack))
        return fail("Timeout waiting for ready ack.");
    
    if (end_ack != 'O')
        return fail("Epos2 acknowledged error in reception.");
    
    return SUCCESS;
}


int recv_frame(HANDLE file, size_t len, uint8_t *data) {
    uint8_t opcode;
    if (timeout_read(file, &opcode, sizeof opcode))
        return fail("Timeout waiting for response opcode.");
    if (opcode)
        return fail("Invalid (non-null) response opcode.");
    
    DWORD written;
    uint8_t ready_ack = 'O';
    if (!WriteFile(file, &ready_ack, 1, &written, 0) || written != 1)
        return fail("Error sending ready ack.");
    
    uint8_t len_minus_1;
    if (timeout_read(file, &len_minus_1, sizeof len_minus_1))
        return fail("Timeout waiting for message length.");
    if (len_minus_1 != len / 2 - 1)
        return fail("Invalid response message length.");
    
    if (timeout_read(file, data, len))
        return fail("Timeout waiting for message data.");
    
    uint8_t recv_crc[2];
    if (timeout_read(file, recv_crc, sizeof recv_crc))
        return fail("Timeout waiting for crc.");
    
    uint16_t crc = crc_byte(0, opcode);
    crc = crc_byte(crc, len_minus_1);
    crc = crc_data(crc, data, len);
    if (crc != pack_le_uint16(recv_crc)) {
        uint8_t end_ack = 'F';
        if (!WriteFile(file, &end_ack, 1, &written, 0) || written != 1)
            return fail("Error sending (failed) end ack.");
        
        return fail("Invalid message crc received.");
    }

    uint8_t end_ack = 'O';
    if (!WriteFile(file, &end_ack, 1, &written, 0) || written != 1)
        return fail("Error sending (okay) end ack.");
    
    return SUCCESS;
}


//  --- Exported module functions --- //

HANDLE epos_open_port(const char *path) {
    HANDLE file = CreateFile(path, GENERIC_READ | GENERIC_WRITE, 0, 0,
                             OPEN_EXISTING, 0, 0);
    if (file == INVALID_HANDLE_VALUE) {
        fail("Error opening port.");
        return INVALID_HANDLE_VALUE;
    }
    
    DCB dcb;
    FillMemory(&dcb, sizeof(dcb), 0);
    dcb.DCBlength = sizeof(dcb);
    if (!BuildCommDCB("115200,n,8,1", &dcb)) {
        CloseHandle(file);
        fail("Error building device control block.");
        return INVALID_HANDLE_VALUE;
    }
    if (!SetCommState(file, &dcb)) {
        CloseHandle(file);
        fail("Error setting port device control block.");
        return INVALID_HANDLE_VALUE;
    }

    COMMTIMEOUTS timeouts = {
        .ReadIntervalTimeout = 0,
        .ReadTotalTimeoutMultiplier = 0,
        .ReadTotalTimeoutConstant = TIMEOUT_MS,
        .WriteTotalTimeoutMultiplier = 0,
        .WriteTotalTimeoutConstant = TIMEOUT_MS
    };
    if (!SetCommTimeouts(file, &timeouts)) {
        CloseHandle(file);
        fail("Error setting port timeouts.");
        return INVALID_HANDLE_VALUE;
    }
    
    return file;
}

int epos_read_object(HANDLE file, uint16_t index, uint8_t subindex,
                     uint8_t nodeid, uint32_t *value_ptr) {
    flush_buffers(file);
    
    uint8_t request[4] = {index, index >> 8, subindex, nodeid};
    if (send_frame(file, READ_OBJECT_OPCODE, sizeof request, request))
        return fail("Error sending ReadObject frame.");
    
    uint8_t response[8];
    if (recv_frame(file, sizeof response, response))
        return fail("Error receiving ReadObject response.");
    
    uint32_t error = pack_le_uint32(response);
    if (error)
        return fail_code("Error in ReadObject", error);

    *value_ptr = pack_le_uint32(response + 4);
    return SUCCESS;
}


int epos_write_object(HANDLE file, uint16_t index, uint8_t subindex,
                     uint8_t nodeid, uint32_t value) {
    flush_buffers(file);
    
    uint8_t request[8] = {
        index, index >> 8, subindex, nodeid,
        value, value >> 8, value >> 16, value >> 24
    };
    if (send_frame(file, WRITE_OBJECT_OPCODE, sizeof request, request))
        return fail("Error sending WriteObject frame.");
    
    uint8_t response[4];
    if (recv_frame(file, sizeof response, response))
        return fail("Error receiving WriteObject response.");
    
    uint32_t error = pack_le_uint32(response);
    if (error)
        return fail_code("Error in WriteObject", error);
    
    return SUCCESS;
}


static uint16_t crc_byte(uint16_t crc, uint8_t data) {
    int i;
    crc ^= (uint16_t)data << 8;
    
    for (i = 0; i < 8; i++)
        if (crc & 0x8000)
            crc = (crc << 1) ^ 0x1021;
        else
            crc <<= 1;
    
    return crc;
}

static uint16_t crc_data(uint16_t crc, uint8_t* data, int len) {
    uint8_t low_byte, high_byte;
    
    len -= (len % 2); //For safety.
    
    while (len > 0) {
        low_byte  = *data++;
        high_byte = *data++;
        
        crc = crc_byte(crc, high_byte);
        crc = crc_byte(crc, low_byte);
        
        len -= 2;
    }
    
    return crc;
}
