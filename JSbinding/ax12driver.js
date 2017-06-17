var ffi = require('ffi');
var simpleCallback = ffi.Function('void', []);

var lib = ffi.Library('libAX12', {
    'initAX12': [ 'int', ['int'] ],
    'AX12getPosition': [ 'double', ['uint8'] ],
    'AX12getSpeed': [ 'double', ['uint8'] ],
    'AX12getLoad': [ 'double', ['uint8'] ],
    'AX12getStatus': [ 'int', ['uint8'] ],
    'AX12getVoltage': [ 'double', ['uint8'] ],
    'AX12getTemperature': [ 'int', ['uint8'] ],
    'AX12isMoving': [ 'int', ['uint8'] ],

    'AX12setMode': [ 'int', ['uint8', 'int'] ],
    'AX12setSpeed': [ 'int', ['uint8', 'double'] ],
    'AX12setTorque': [ 'int', ['uint8', 'double'] ],
    'AX12setLED': [ 'int', ['uint8', 'int'] ],
    'AX12move': [ 'int', ['uint8', 'double', simpleCallback] ],
    'AX12cancelCallback': [ 'void', ['uint8'] ],
    'AX12turn': [ 'int', ['uint8', 'double'] ],
    'AX12resetAll': [ 'void', [] ]
});

var callbacks = [];
module.exports = {
    init: lib.initAX12,
    position: function (id) {
        return Math.round(100*lib.AX12getPosition(id))/100;
    },
    speed: function (id) {
        return Math.round(100*lib.AX12getSpeed(id))/100;
    },
    load: function (id) {
        return Math.round(100*lib.AX12getLoad(id))/100;
    },
    status: lib.AX12getStatus,
    voltage: function (id) {
        return Math.round(100*lib.AX12getVoltage(id))/100;
    },
    temperature: lib.AX12getTemperature,
    moving: function (id) {
        return lib.AX12isMoving(id) == 1;
    },
    mode: lib.AX12setMode,
    goalSpeed: lib.AX12setSpeed,
    torque: lib.AX12setTorque,
    LED: lib.AX12setLED,
    move: function (id, position, callback) {
        callbacks[id] = ffi.Callback('void', [], callback);
        lib.AX12move(id, position, callbacks[id]);
    },
    cancelCallback: lib.AX12cancelCallback,
    turn: lib.AX12turn,
    reset: lib.AX12resetAll
};
