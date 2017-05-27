# -*- coding: utf-8 -*-

# ce fichier contient les memes fonctions que ax12driver.c, mais ecrites en
# python cette fois-ci
# donc cf ax12driver.h pour avoir la doc
# en plus, une verification de la validite des arguments est faite
# ie, si on envoie un float la ou il faut un int, ca va lever une AssertionError
# verifie aussi la coherence en fonction des parametres physiques de la datasheet


from encapsulate_callback import encapsulate_callback
from I2C_bus import *



class Communication_Error(Exception):

    errors = ["No error", "Serial port not initialized", "Wrong checksum", "Target and answer ID mismatch", "Timeout", "Callback buffer is full"]

    def __init__(self, index):
        if index>=len(Communication_Error.errors) or index<0:
            self.msg = "No error matching return code"
        else:
            self.msg = Communication_Error.errors[index]

    def __str__(self):
        return self.msg



DEFAULT_MODE    =   0
WHEEL_MODE      =   1
def check_mode(m):
    assert (m in [DEFAULT_MODE, WHEEL_MODE])


class AX12:

    def __init__(self, id, baudrate=115200):
        check_uint8(id)
        self.id = id
        if I2C_bus.instance is None:
            I2C_bus(baudrate)
        elif I2C_bus.baudrate != baudrate:
            print "[.] Baudrate used to communicate with AX12 id "+str(id)+" ("+str(baudrate)+") does not match previously established baudrate ("+I2C_bus.baudrate+") (changing it)"
            I2C_bus(baudrate)


    @classmethod
    def scan_i2c(cls, print_on_fly=None):
        if I2C_bus.instance is None:
            print "[-] Unable to scan I2C bus because not initialized"
        else:
            I2C_bus.scan(print_on_fly)


    def ping(self):
        return I2C_bus.ping(self.id)


    def get_status(self):
        return int(lib_ax12.AX12getStatus(ctypes.c_uint8(self.id)))


    def get_position(self):
        return lib_ax12.AX12getPosition(ctypes.c_uint8(self.id))


    def get_speed(self):
        return lib_ax12.AX12getSpeed(ctypes.c_uint8(self.id))


    def get_load(self):
        return lib_ax12.AX12getLoad(ctypes.c_uint8(self.id))


    def get_voltage(self):
        return lib_ax12.AX12getVoltage(ctypes.c_uint8(self.id))


    def get_temperature(self):
        return int(lib_ax12.AX12getTemperature(ctypes.c_uint8(self.id)))


    def is_moving(self):
        return int(lib_ax12.AX12isMoving(ctypes.c_uint8(self.id)))


    def set_mode(self, mode):
        check_mode(mode)

        ret = int(lib_ax12.AX12setMode(ctypes.c_uint8(self.id),
                                       ctypes.c_int(mode)))
        if ret<0:
            raise Communication_Error(-ret)
        return ret


    def set_speed(self, speed):
        ret = int(lib_ax12.AX12setSpeed(ctypes.c_uint8(self.id),
                                        ctypes.c_double(speed)))
        if ret<0:
            raise Communication_Error(-ret)
        return ret


    def set_torque(self, torque):
        ret = int(lib_ax12.AX12setTorque(ctypes.c_uint8(self.id),
                                         ctypes.c_double(torque)))
        if ret<0:
            raise Communication_Error(-ret)
        return ret


    def set_LED(self, state):
        assert(isinstance(state, int))

        ret = int(lib_ax12.AX12setLED(ctypes.c_uint8(self.id),
                                      ctypes.c_int(state)))
        if ret<0:
            raise Communication_Error(-ret)
        return ret


    def move(self, position, callback):
        assert(isinstance(position, float) or isinstance(position, int))
        assert(callable(callback))

        ret = int(lib_ax12.AX12move(ctypes.c_uint8(self.id),
                                    ctypes.c_double(position),
                                    encapsulate_callback(callback)))
        if ret<0:
            raise Communication_Error(-ret)
        return ret


    def cancel_callback(self):
        lib_ax12.AX12CancelCallback(ctypes.c_uint8(self.id))


    def turn(self, speed):
        assert(isinstance(speed, float) or isinstance(speed, int))

        ret = int(lib_ax12.AX12turn(ctypes.c_uint8(self.id),
                                    ctypes.c_int(speed)))
        if ret<0:
            raise Communication_Error(-ret)
        return ret
