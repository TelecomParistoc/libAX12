# libAX12 #

This library provides high level access to AX12 digital actuators.
It uses Raspberry Pi's serial port to control the AX12. This requires some
additional electronics to communicate with the AX12's serial half duplex link.
An example of such electronics can be found [here](https://github.com/TelecomParistoc/robothat).

## Installation ##

This library is designed for Raspberry Pi with Raspbian.

* First, you need [wiringPi](http://wiringpi.com/download-and-install/) and
[librobotutils](https://github.com/TelecomParistoc/librobotutils).

* Once dependencies are installed, get back the sources :
`git clone git@github.com:TelecomParistoc/libAX12.git`

* cd to the root of the repository and enter `make`

* finally enter `sudo make install`

## Usage ##

Don't forget to compile (actually, link) your C/C++ app with `-lAX12` option.
You can include all the headers you need with :
```c
#include <AX12/driver.h>
```
It is also possible to include headers individually.

To generate a documentation of this file, you can use Doxygen.
Go in the __/doc__ directory and run
```
doxygen Doxyfile
```

### AX12 driver ###
Most of the AX12 features can be controlled via easy-to-use functions, including
move to a given position and calling a callback upon completion. A CLI also gives
access to the whole AX12 memory (see below).

For more info on AX12 driver API, see
[ax12driver.h](https://github.com/TelecomParistoc/libAX12/blob/master/src/ax12driver.h).


## AX12 console utility ##

A utility is provided for configuration and diagnostic purposes. It can read and
write an AX12 memory and scan for AX12 on the bus. To use it, launch :

```
$  AX12
```

The serial baurate can be set with `-b` (default is 115200).

## Tests ##

Several simple programs are provided to check that the library is working properly.

To run the tests, run `make tests` from the root of the repository, and run
some of the programs created in tests/.

## Bindings ##

Bindings are provided to use libAX12 with other languages than C/C++.

### Javascript ###

Most of the reasonable programmers use and love Javascript/Coffeescript.
That's why a simple JS binding is provided, allowing to call AX12 driver functions
from javascript. Check [JSbinding/README.md](https://github.com/TelecomParistoc/libAX12/blob/master/JSbinding/README.md)
for more info.

### Python ###

Not as fun as JS but some programmers seems to use it anyway ...
