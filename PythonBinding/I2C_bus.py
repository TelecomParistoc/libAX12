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


def check_uint8(x):
    assert (isinstance(x, int))
    assert (0 <= x <= 255)


class Singleton(type):

    _instances = {}

    def __call__(cls, *args, **kwargs):
        if cls not in cls._instances:
            cls._instances[cls] = super(Singleton, cls).__call__(*args, **kwargs)
        return cls._instances[cls]


lib_ax12 = ctypes.cdll.LoadLibrary(LIBNAME)

class I2C_bus:

    __metaclass__ = Singleton

    instance = None

    def __init__(self, baudrate=115200):
        self.baudrate = baudrate

        assert(isinstance(baudrate, int))
        assert(7343 <= baudrate <= 1000000)

        I2C_bus.instance = self
        print "Testing : "+repr(self)

        ret = int(lib_ax12.initAX12(ctypes.c_int(self.baudrate)))
        if ret<0:
            raise Initialisation_Error(-ret)

        return ret

    def ping(self, id):
        assert(check_uint8(id))
        return int(lib_ax12.axPing(ctypes.c_int(id)))

    def scan(self, print_on_fly=None):
        elems = []
        for i in range(254):
            if self.ping(i) == 0:
                elems.append(i)
                if callable(print_on_fly):
                    print_on_fly(i)
        return elems