function [i, c, m] = epos_variables()

i.CONTROL_WORD = uint16(hex2dec('6040'));
i.MODES_OPERATION = uint16(hex2dec('6060'));

c.FAULT_RESET = uint32(hex2dec('0080'));
c.SHUTDOWN = uint32(hex2dec('0006'));

m.HOMING = uint32(hex2dec('06'));