function [value, status] = ...
    epos_read_object_wrapper(handle, index, subindex, nodeid) %#codegen

coder.cinclude('../epos.h')

hdl = cast(handle, 'like', coder.opaque('HANDLE', '0'));

status = int64(0);
value = uint32(0);
status = coder.ceval('epos_read_object', hdl, index, subindex, nodeid, ...
                     coder.ref(value));
