/**
 * @file epos2.h
 * @brief Header for Maxon Motor EPOS2 serial port communication driver.
 */


#include <windows.h>

#include <stdint.h>


/**
 * @brief Read from the EPOS object dictionary.
 *
 * It is a blocking function call.
 *
 * @returns 0 if success, nonzero otherwise.
 */
int epos_read_object(HANDLE file, uint16_t index, uint8_t subindex,
                     uint8_t nodeid, uint32_t *value_ptr);


/**
 * @brief Write to the EPOS object dictionary.
 *
 * It is a blocking function call.
 *
 * @returns 0 if success, nonzero otherwise.
 */
int epos_write_object(HANDLE file, uint16_t index, uint8_t subindex,
                       uint8_t nodeid, uint32_t value);


/**
 * @brief Open EPOS serial port.
 *
 * It is a blocking function call.
 *
 * @returns file handle
 */
HANDLE epos_open_port(const char *path);
