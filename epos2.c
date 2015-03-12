/**
 * @file epos2.c
 * @brief Implementation of Maxon Motor EPOS2 serial port communication driver.
 */

#include "epos2.h"

#include <io.h>

#include <stdio.h>


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

static inline int fail(const char *msg) {
#ifdef DEBUG
    fprintf(stderr, "%s\n", msg);
#endif//DEBUG
    return FAIL;
}


static inline int fail_code(const char *msg, uint32_t code) {
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

static uint16_t crc_ccitt(uint8_t *buf, size_t len, uint16_t crc);


int send_frame(HANDLE file, uint8_t opcode, size_t len, uint8_t *data) {
    DWORD written;
    if (!WriteFile(file, &opcode, 1, &written, 0) || written != 1)
        return fail("Error writing opcode.");
    
    uint8_t ready_ack;
    if (timeout_read(file, &ready_ack, 1))
        return fail("Timeout waiting for ready ack.");
    if (ready_ack == 'F')
        return fail("Epos not ready to receive.");
    if (ready_ack != 'O') {
        return fail_code("Unrecognized ack received", ready_ack);
    }
    
    uint8_t len_minus_1 = len - 1;
    if (!WriteFile(file, &len_minus_1, 1, &written, 0) || written != 1)
        return fail("Error writing message length.");
    
    if (!WriteFile(file, &data, len, &written, 0) || written != len)
        return fail("Error writing message data.");
    
    uint16_t crc = crc_ccitt(&opcode, 1, 0);
    crc = crc_ccitt(&len_minus_1, 1, crc);
    crc = crc_ccitt(data, len, crc);
    
    uint8_t crc_bytes[2] = {crc, crc >> 8};
    if (!WriteFile(file, &crc_bytes, 2, &written, 0) || written != 2)
        return fail("Error writing crc.");

    uint8_t end_ack;
    if (timeout_read(file, &end_ack, 1))
        return fail("Timeout waiting for ready ack.");
    if (ready_ack != 'O')
        return fail("Epos2 acknowledged error in reception.");    
    
    return SUCCESS;
}


int recv_frame(HANDLE file, size_t len, uint8_t *data) {
    uint8_t opcode;
    if (timeout_read(file, &opcode, 1))
        return fail("Timeout waiting for response opcode.");
    if (opcode)
        return fail("Invalid (non-null) response opcode.");
    
    DWORD written;
    uint8_t ready_ack = 'O';
    if (!WriteFile(file, &ready_ack, 1, &written, 0) || written != 1)
        return fail("Error sending ready ack.");
    
    uint8_t len_minus_1;
    if (timeout_read(file, &len_minus_1, 1))
        return fail("Timeout waiting for message length.");
    if (len_minus_1 != len - 1)
        return fail("Invalid response message length.");
    
    if (timeout_read(file, data, len))
        return fail("Timeout waiting for message data.");
    
    uint8_t recv_crc[2];
    if (timeout_read(file, recv_crc, 2))
        return fail("Timeout waiting for crc.");
    
    uint16_t crc = crc_ccitt(&opcode, 1, 0);
    crc = crc_ccitt(&len_minus_1, 1, crc);
    crc = crc_ccitt(data, len, crc);
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


/// CRC-CCITT of every byte
static const uint16_t crc_ccitt_tbl[256] = {
    0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
    0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
    0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
    0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
    0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
    0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
    0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
    0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
    0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
    0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
    0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
    0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
    0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
    0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
    0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
    0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
    0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
    0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
    0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
    0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
    0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
    0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
    0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
    0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
    0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
    0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
    0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
    0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
    0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
    0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
    0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
    0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};


static uint16_t crc_ccitt(uint8_t *buf, size_t len, uint16_t crc) {
    for (int i=0; i < len; i++)
        crc = crc_ccitt_tbl[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);
    
    return crc;
}
