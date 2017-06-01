function handle = epos_open_port_wrapper(str)%#codegen
coder.cinclude('../epos.h')

hdl = coder.opaque('HANDLE');
hdl = coder.ceval('epos_open_port', str);

handle = cast(hdl, 'uint64');
