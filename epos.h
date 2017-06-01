/**
 * @file epos.h
 * @brief Header for Maxon Motor EPOS serial port communication driver.
 */


#include <windows.h>

#include <stdint.h>

enum {
  EPOS_CONTROL_WORD_INDEX=0x6040,
  EPOS_MODES_OPERATION_INDEX=0x6060,
  EPOS_VELOCITY_MODE_SP_INDEX=0x206B,
  EPOS_POSITION_MODE_SP_INDEX=0x2062,
  EPOS_TARGET_VELOCITY_INDEX=0x60FF,
  EPOS_TARGET_POSITION_INDEX=0x607A,
  EPOS_POSITION_ACTUAL_VALUE_INDEX=0x6064
} epos_obj_index_t;

enum {
  EPOS_FAULT_RESET_CMD=0x0080,
  EPOS_SHUTDOWN_CMD=0x0006,
  EPOS_SWITCH_ON_CMD=0x0007,
  EPOS_ENABLE_OPERATION_CMD=0x000F,
  EPOS_HALT_CMD=0x0102,
  EPOS_GOTO_POSITION_REL_CMD=0x007F,
  EPOS_GOTO_POSITION_ABS_CMD=0x003F,
  EPOS_GOTO_VELOCITY_CMD=0x000F
} epos_commands_t;

typedef enum {
  EPOS_HOMING_MODE=0x06,
  EPOS_PROFILE_VELOCITY_MODE=0x03,
  EPOS_PROFILE_POSITION_MODE=0x01,
  EPOS_POSITION_MODE=0xFF,
  EPOS_VELOCITY_MODE=0xFE,
  EPOS_CURRENT_MODE=0xFD,
  EPOS_DIAGNOSTIC_MODE=0xFC,
  EPOS_MASTER_ENCODER_MODE=0xFB,
  EPOS_STEP_DIRECTION_MODE=0xFA
} epos_mode_t;


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


/**
 * @brief Send FAULT RESET command to epos.
 *
 * It is a blocking function call.
 *
 * @returns 0 if success, nonzero otherwise.
 */
int epos_fault_reset(HANDLE file, uint8_t nodeid);


/**
 * @brief Send SHUTDOWN command to epos.
 *
 * It is a blocking function call.
 *
 * @returns 0 if success, nonzero otherwise.
 */
int epos_shutdown(HANDLE file, uint8_t nodeid);


/**
 * @brief Send SWITCH ON command to epos.
 *
 * It is a blocking function call.
 *
 * @returns 0 if success, nonzero otherwise.
 */
int epos_switch_on(HANDLE file, uint8_t nodeid);


/**
 * @brief Send ENABLE OPERATION command to epos.
 *
 * It is a blocking function call.
 *
 * @returns 0 if success, nonzero otherwise.
 */
int epos_enable_operation(HANDLE file, uint8_t nodeid);


/**
 * @brief Send HALT command to epos.
 *
 * It is a blocking function call.
 *
 * @returns 0 if success, nonzero otherwise.
 */
int epos_halt(HANDLE file, uint8_t nodeid);


/**
 * @brief Send GO TO RELATIVE POSITION command to epos.
 *
 * It is a blocking function call.
 *
 * @returns 0 if success, nonzero otherwise.
 */
int epos_goto_position_rel(HANDLE file, uint8_t nodeid);


/**
 * @brief Send GO TO ABSOLUTE POSITION command to epos.
 *
 * It is a blocking function call.
 *
 * @returns 0 if success, nonzero otherwise.
 */
int epos_goto_position_abs(HANDLE file, uint8_t nodeid);


/**
 * @brief Send GO TO VELOCITY command to epos.
 *
 * It is a blocking function call.
 *
 * @returns 0 if success, nonzero otherwise.
 */
int epos_goto_velocity(HANDLE file, uint8_t nodeid);


/**
 * @brief Set epos mode of operation.
 *
 * It is a blocking function call.
 *
 * @returns 0 if success, nonzero otherwise.
 */
int epos_set_mode(HANDLE file, uint8_t nodeid, epos_mode_t mode);


int epos_set_target_position(HANDLE file, uint8_t nodeid, int32_t val);
