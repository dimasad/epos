/**
 * @file epos2.h
 * @brief Header for Maxon Motor EPOS2 serial port communication driver.
 */

#include <stdint.h>


/**
 * @brief Read from the EPOS object dictionary.
 *
 * It is a non-blocking function call.
 *
 * @returns 0 if success, nonzero otherwise.
 */
int epos_read_object(int fd, uint16_t index, uint8_t subindex, uint8_t nodeid,
                     uint32_t *object);
