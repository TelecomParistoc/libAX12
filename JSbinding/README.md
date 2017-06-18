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

Then just include it like a regular module like `var ax12 = require('AX12');`.

#### ax12(id) ####
Create a new AX12 object.  
You can use it to control real AX12 directly, or create a virtual AX12, to control
a family a servos at once.  
**id** : the ID of the AX12, 0 zero to create a virtual AX12, 1-255 otherwise.

#### ax12.create(id) ####
Create a new AX12 inheriting the all the attributes. Additionally, any action on
ax12 will affect all the servos created from it.

#### ax12.speed() ####
Get the target speed, in %

#### ax12.speed(speed) ####
Set the target speed, in % (0-100)

#### ax12.torque() ####
Get the target torque, in %

#### ax12.torque(torque) ####
Set the target torque, in % (0-100)

#### ax12.moveTo(position, callback) ####
Set AX12 to position control mode and move to the desired position.  
**position** : desired position in deg, -150 to 150  
**callback** : optional. A function to call when target position has been reached.
for a virtual AX12 (ID=0), callback is called when all the children have reached
their target.  

#### ax12.cancelCallback() ####
Cancel moveTo callback.

#### ax12.turn(speed) ####
Set AX12 to wheel mode and turn.  
**speed** : from -100 to 100 in %

#### ax12.preset(presetName, preset, force) ####
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

Preset can be later applied using ax12.presetName()

**force** : optional. When set to true, allows to replace an existing preset with the same name.

#### ax12.LED(status) ####
Turn AX12 LED on or off.  
**status** : 1 for on, 0 for off

#### ax12.position() ####
Get current position in deg, -150 to 150.

#### ax12.moving() ####
Returns true if the AX12 is moving by its own power.

#### ax12.temperature() ####
Returns AX12 temperature in °C.

#### ax12.voltage() ####
Returns AX12 power voltage in volts.

#### ax12.error() ####
Returns 0 for no error, an error flags otherwise. See libAX12 for more info.
