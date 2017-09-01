# -*- coding: utf-8 -*-

# ce fichier contient les memes fonctions que ax12driver.c, mais ecrites en
# python cette fois-ci
# donc cf ax12driver.h pour avoir la doc
# en plus, une verification de la validite des arguments est faite
# ie, si on envoie un float la ou il faut un int, ca va lever une AssertionError
# verifie aussi la coherence en fonction des parametres physiques de la datasheet


from encapsulate_callback import encapsulate_callback
import ctypes


class Initialisation_Error(Exception):

    errors = ["Success", "Cannot open AX12 serial port", "Cannot create mutex"]

    def __init__(self, index):
        if index>=len(errors) or index<0:
            self.msg = "No error matching return code"
        else:
            self.msg = Communication_Exception.errors[index]

    def __str__(self):
        return self.msg


class Communication_Error(Exception):

    errors = ["No error", "Serial port not initialized", "Wrong checksum", "Target and answer ID mismatch", "Timeout", "Callback buffer is full"]

    def __init__(self, index):
        if index>=len(errors) or index<0:
            self.msg = "No error matching return code"
        else:
            self.msg = Communication_Exception.errors[index]

    def __str__(self):
        return self.msg


lib_ax12 = ctypes.cdll.LoadLibrary(LIBNAME)


DEFAULT_MODE    =   0
WHEEL_MODE      =   1


def check_uint8(x):
    assert (isinstance(x, int))
    assert (0 <= identifiant <= 255)


def check_mode(m):
    assert (m in [DEFAULT_MODE, WHEEL_MODE])


def init_AX12(baudrate):
    assert(isinstance(baudrate, int))
    assert (7343 <= baudrate <= 1000000)

    ret = int(lib_ax12.initAX12(ctypes.c_int(baudrate)))
    if ret<0:
        raise Initialisation_Error(-ret)
    return ret


def AX12_get_position(identifiant):
    check_uint8(identifiant)
    return float(lib_ax12.AX12getPosition(ctypes.c_uint8(identifiant)))


def AX12_get_speed(identifiant):
    check_uint8(identifiant)
    return float(lib_ax12.AX12getSpeed(ctypes.c_uint8(identifiant)))


def AX12_get_load(identifiant):
    check_uint8(identifiant)
    return float(lib_ax12.AX12getLoad(ctypes.c_uint8(identifiant)))


def AX12_get_status(identifiant):
    check_uint8(identifiant)
    return int(lib_ax12.AX12getStatus(ctypes.c_uint8(identifiant)))


def AX12_get_voltage(identifiant):
    check_uint8(identifiant)
    return float(lib_ax12.AX12getVoltage(ctypes.c_uint8(identifiant)))


def AX12_get_temperature(identifiant):
    check_uint8(identifiant)
    return int(lib_ax12.AX12getTemperature(ctypes.c_uint8(identifiant)))


def AX12_is_moving(identifiant):
    check_uint8(identifiant)
    return int(lib_ax12.AX12isMoving(ctypes.c_uint8(identifiant)))


def AX12_set_mode(identifiant, mode):
    check_uint8(identifiant)
    check_mode(mode)

    ret = int(lib_ax12.AX12setMode(ctypes.c_uint8(identifiant),
                                   ctypes.c_int(mode)))
    if ret<0:
        raise Communication_Error(-ret)
    return ret


def AX12_set_speed(identifiant, speed):
    check_uint8(identifiant)

    ret = int(lib_ax12.AX12setSpeed(ctypes.c_uint8(identifiant),
                                    ctypes.c_double(speed)))
    if ret<0:
        raise Communication_Error(-ret)
    return ret


def AX12_set_torque(identifiant, torque):
    check_uint8(identifiant)

    ret = int(lib_ax12.AX12setTorque(ctypes.c_uint8(identifiant),
                                     ctypes.c_double(torque)))
    if ret<0:
        raise Communication_Error(-ret)
    return ret


def AX12_set_LED(identifiant, state):
    assert(isinstance(state, int))

    ret = int(lib_ax12.AX12setLED(ctypes.c_uint8(identifiant),
                                  ctypes.c_int(state)))
    if ret<0:
        raise Communication_Error(-ret)
    return ret


def AX12_move(identifiant, position, callback):
    check_uint8(identifiant)
    assert(isinstance(position, float))
    assert(callable(callback))

    ret = int(lib_ax12.AX12move(ctypes.c_uint8(identifiant),
                                ctypes.c_double(position),
                                encapsulate_callback(callback)))
    if ret<0:
        raise Communication_Error(-ret)
    return ret


def AX12_cancel_callback(identifiant):
    check_uint8(identifiant)
    lib_ax12.AX12CancelCallback(ctypes.c_uint8(identifiant))


def AX12_turn(identifiant, speed):
    check_uint8(identifiant)

    ret = int(lib_ax12.AX12turn(ctypes.c_uint8(identifiant),
                                ctypes.c_int(speed)))
    if ret<0:
        raise Communication_Error(-ret)
    return ret
