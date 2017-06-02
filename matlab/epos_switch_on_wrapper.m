function status = epos_switch_on_wrapper(handle, nodeid) %#codegen
coder.cinclude('../epos.h')

hdl = cast(handle, 'like', coder.opaque('HANDLE', '0'));

status = int64(0);
status = coder.ceval('epos_switch_on', hdl, nodeid);

