from AX12 import *


a = AX12(129)
print a.get_status()
print a.get_position()

b = I2C_bus(9600)
I2C_bus.init(115200)
c = I2C_bus(100000)
d = I2C_bus(209302)
d.reset(115200)

print c.baudrate
print d.baudrate
print b.baudrate
