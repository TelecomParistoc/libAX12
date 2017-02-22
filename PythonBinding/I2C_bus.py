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
        return cls._instances[cls]




def load_lib_ax12():
    """
        we must specify the return type for each function
        because the default return type is c_int (and then automatically cast
        to int)
    """
    l = ctypes.cdll.LoadLibrary(LIBNAME)

    l.AX12getPosition.restype   = ctypes.c_double
    l.AX12getSpeed.restype      = ctypes.c_double
    l.AX12getLoad.restype       = ctypes.c_double
    l.AX12getStatus.restype     = ctypes.c_int
    l.AX12getVoltage.restype    = ctypes.c_double
    l.AX12getTemperature.restype= ctypes.c_int
    l.AX12isMoving.restype      = ctypes.c_int
    l.AX12setMode.restype       = ctypes.c_int
    l.AX12setSpeed.restype      = ctypes.c_int
    l.AX12setTorque.restype     = ctypes.c_int
    l.AX12setLED.restype        = ctypes.c_int
    l.AX12move.restype          = ctypes.c_int
    l.AX12cancelCallback.restype= None
    l.AX12turn.restype          = ctypes.c_int
    l.AX12resetAll.restype       = None

    return l
    
    

lib_ax12 = load_lib_ax12()

class I2C_bus:

    __metaclass__ = Singleton

    instance = None
    baudrate = None

    def __init__(self, baudrate=115200, init=True):
        if I2C_bus.instance is not None and init:
            print("[-] Uneffective new construction of I2C_bus, please use reset to change baudrate")
            return
        I2C_bus.instance = self

        self.baudrate = baudrate
        I2C_bus.baudrate = baudrate
        if init:
            I2C_bus.init(baudrate)


    @classmethod
    def init(cls, baudrate=115200):
        assert(isinstance(baudrate, int))
        assert(7343 <= baudrate <= 1000000)

        ret = int(lib_ax12.initAX12(ctypes.c_int(baudrate)))
        if ret<0:
            raise Initialisation_Error(-ret)

        cls.instance = I2C_bus(baudrate, False)

        return ret


    @classmethod
    def reset(cls, baudrate):
        if I2C_bus.instance is None:
            I2C_bus.instance = I2C_bus(baudrate)
        elif I2C_bus.instance.baudrate != baudrate:
            if I2C_bus.init(baudrate) == 0:
                I2C_bus.instance.baudrate = baudrate
            else:
                return -1
        return 0


    @classmethod
    def ping(cls, id):
        check_uint8(id)
        return int(lib_ax12.axPing(ctypes.c_int(id), ctypes.c_int(0)))

    @classmethod
    def scan(cls, print_on_fly=None):
        elems = []
        for i in range(254):
            if I2C_bus.ping(i) == 0:
                elems.append(i)
                if callable(print_on_fly):
                    print_on_fly(i)
        return elems
