import ctypes


class Initialisation_Error(Exception):

    errors = ["Success", "Cannot open AX12 serial port", "Cannot create mutex"]

    def __init__(self, index):
        if index>=len(Initialisation_Error.errors) or index<0:
            self.msg = "No error matching return code"
        else:
            self.msg = Initialisation_Error.errors[index]

    def __str__(self):
        return self.msg


def check_uint8(x):
    assert (isinstance(x, int))
    assert (0 <= x <= 255)


class Singleton(type):

    _instances = {}

    def __call__(cls, *args, **kwargs):
        if cls not in cls._instances:
            cls._instances[cls] = super(Singleton, cls).__call__(*args, **kwargs)
        print("Returning "+repr(cls._instances[cls]))
        return cls._instances[cls]


lib_ax12 = ctypes.cdll.LoadLibrary(LIBNAME)

class I2C_bus:

    __metaclass__ = Singleton

    instance = None

    def __init__(self, baudrate=115200):
        if I2C_bus.instance is not None:
            print("[-] Uneffective new construction of I2C_bus, please use reset to change baudrate")
            return
        I2C_bus.instance = self

        self.baudrate = baudrate
        I2C_bus.init_i2c(baudrate)


    @classmethod
    def init_i2c(cls, baudrate):
        assert(isinstance(baudrate, int))
        assert(7343 <= baudrate <= 1000000)

        ret = int(lib_ax12.initAX12(ctypes.c_int(baudrate)))
        if ret<0:
            raise Initialisation_Error(-error)
        return ret


    @classmethod
    def reset(cls, baudrate):
        if I2C_bus.instance is None:
            I2C_bus.instance = I2C_bus(baudrate)
        elif I2C_bus.instance.baudrate != baudrate:
            if I2C_bus.init_i2c(baudrate) == 0:
                I2C_bus.instance.baudrate = baudrate
            else:
                return -1
        return 0


    @classmethod
    def ping(cls, id):
        check_uint8(id)
        return int(lib_ax12.axPing(ctypes.c_int(id)))

    @classmethod
    def scan(cls, print_on_fly=None):
        elems = []
        for i in range(254):
            if I2C_bus.ping(i) == 0:
                elems.append(i)
                if callable(print_on_fly):
                    print_on_fly(i)
        return elems
