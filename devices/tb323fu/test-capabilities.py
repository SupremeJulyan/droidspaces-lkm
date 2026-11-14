import ctypes, errno
c = ctypes.CDLL(None, use_errno=True)
class H(ctypes.Structure):
 _fields_ = [('version',ctypes.c_uint),('pid',ctypes.c_int)]
class D(ctypes.Structure):
 _fields_ = [('effective',ctypes.c_uint),('permitted',ctypes.c_uint),('inheritable',ctypes.c_uint)]
assert c.prctl(24, 999, 0, 0, 0) == -1 and ctypes.get_errno() == errno.EINVAL
assert c.prctl(24, 0, 0, 0, 0) == 0
assert c.prctl(23, 0, 0, 0, 0) == 0
print('PASS valid drop and invalid-cap rejection', flush=True)
h=H(0x20080522,0); d=(D*2)()
assert c.capget(ctypes.byref(h),d) == 0
for x in d: x.effective=0
assert c.capset(ctypes.byref(h),d) == 0
assert c.prctl(24, 1, 0, 0, 0) == -1 and ctypes.get_errno() == errno.EPERM
print('PASS missing effective CAP_SETPCAP is denied', flush=True)
