function status = epos_init(hdl, nodeid)

if nargin == 1
  nodeid = uint8(0)
end

status = epos_fault_reset(hdl, nodeid)
if status ~= 0
  return
end

status = epos_shutdown(hdl, nodeid)
if status ~= 0
  return
end

status = epos_switch_on(hdl, nodeid)
if status ~= 0
  return
end

status = epos_enable_operation(hdl, nodeid)
