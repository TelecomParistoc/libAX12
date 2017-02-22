# AX12 JS binding #

This library provides high-level control over AX12 digital actuators,
in javascript with nodeJS.

## Install ##

Install of the JS binding is done during libAX12 install. Check out main README
for instructions.

## Usage ##

To use it, after having installed libAX12 you need to create link to the
module in your local node_modules directory. From the root of your project run :

```
ln -s /usr/local/lib/node_modules/AX12/ node_modules/AX12
```

Then just include it like a regular module like `var AX12 = require('AX12');`.

Here is an example showing some of the library features :

```javascript
var AX12 = require('AX12');

var wheel = AX12(0); // let's create a virtual AX12 modelising a wheel

// the left wheel AX12 with ID = 128, "inheriting" from the generic wheel
// any modification of wheel will affect leftWheel
// we could have done : var leftWheel = AX12(128);
var leftWheel = wheel.create(128);

// the right wheel AX12 with ID = 129, "inheriting" from the generic wheel too
var rightWheel = wheel.create(129);

wheel.torque(80); // set leftWheel and rightWheel torque value to 80%

// move leftWheel to 0 deg (output in the )
leftWheel.moveTo(0, function() {console.log('leftWheel at 0 !')});

// move two wheels to 90 deg
wheel.moveTo(90, function() {console.log('wheels at 90 !')})

// setup forward and backward presets (will make the two wheels turn)
wheel.preset('forward', {wheel: true, speed: 60});
wheel.preset('backward', {wheel: true, speed: -60});

wheel.forward(); // equivalent to leftWheel.turn(60); rightWheel.turn(60);
```

#### myax12 = AX12(id) ####
Create a new AX12 object (AX12 being the object returned by `require('AX12')`).
You can use it to control a real AX12 directly, or create a virtual AX12, to
control a family of servos at once.
**id** : the ID of the AX12, 0 zero to create a virtual AX12, 1-255 otherwise.

#### myax12.speed() ####
Get the target speed, in %

#### myax12.speed(speed) ####
Set the target speed, in % (0-100).

#### myax12.torque() ####
Get the target torque, in %.

#### myax12.torque(torque) ####
Set the target torque, in % (0-100).

**Note :** Setting torque to 0 will disable output control (Torque enable register
set to 0), setting it to a positive value will enable output control (Torque enable
register set to 1).

#### myax12.moveTo(position, callback) ####
Set AX12 to position control mode and move to the desired position.  
**position** : desired position in deg, -150 to 150  
**callback** : optional. A function to call when target position has been reached.
for a virtual AX12 (ID=0), callback is called when all the children have reached
their target.  

#### myax12.cancelCallback() ####
Cancel moveTo callback.

#### myax12.turn(speed) ####
Set AX12 to wheel mode and turn.  
**speed** : from -100 to 100 in %

#### myax12.preset(presetName, preset, force) ####
Create a new preset. A preset should have the following structure :

```javascript
var preset = {
    speed: 50,
    torque: 20,
    position: -120,
    wheel: false
}
```

All parameter can be omitted, and AX12 will just remain unchanged.  
Position will be ignored if wheel is set to true.  
If omitted, wheel defaults to false. However, if wheel and position are omitted,
AX12 will remain unchanged.  

Preset can be later applied using myax12.presetName()

**force** : optional. When set to true, allows to replace an existing preset with the same name.

#### newax12 = myax12.create(id) ####
Create a new AX12 object inheriting the all the attributes (speed, torque,
presets). Additionally, any action on myax12 will affect all the servos created
from it.

#### myax12.LED(status) ####
Turn AX12 LED on or off.  
**status** : 1 for on, 0 for off

#### myax12.position() ####
Get current position in deg, -150 to 150.

#### myax12.moving() ####
Returns true if the AX12 is moving by its own power.

#### myax12.temperature() ####
Returns AX12 temperature in °C.

#### myax12.voltage() ####
Returns AX12 power voltage in volts.

#### myax12.error() ####
Returns 0 for no error, an error flags otherwise. See libAX12 for more info.
