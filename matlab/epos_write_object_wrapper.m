function status = epos_write_object_wrapper(handle, index, subindex, ...
                                            nodeid, value)%#codegen
coder.cinclude('../epos.h')

hdl = cast(handle, 'like', coder.opaque('HANDLE', '0'));

status = int64(0);
status = coder.ceval('epos_write_object', hdl, index, subindex, nodeid, ...
                     value);

