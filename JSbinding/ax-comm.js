var ref = require('ref');
var ffi = require('ffi');

var int8ptr = ref.refType(ref.types.uint8);
var int16ptr = ref.refType(ref.types.uint16);

var lib = ffi.Library('libAX12', {
    'initAXcomm': [ 'int', ['int'] ],
    'axWrite8': ['int', ['uint8', 'uint8', 'uint8', int8ptr]],
    'axWrite16': ['int', ['uint8', 'uint8', 'uint16', int8ptr]],
    'axRead8': ['int', ['uint8', 'uint8', int8ptr, int8ptr]],
    'axRead16': ['int', ['uint8', 'uint8', int16ptr, int8ptr]],
    'axPing': ['int', ['uint8', int8ptr]],
    'axFactoryReset': ['int', ['uint8', int8ptr]],
    'enableErrorPrint': ['void', ['int']]
});

module.exports = {
    init: lib.initAXcomm,
    errorLog: function (enable) {
        lib.enableErrorPrint(enable ? 1 : 0);
    },
    write8: function (id, reg, value) {
        var errorPtr = ref.alloc(ref.types.uint8);
        var code = lib.axWrite8(id, reg, Math.round(value)%256, errorPtr);
        return {code: code, error: errorPtr.deref()};
    },
    write16: function (id, reg, value) {
        var errorPtr = ref.alloc(ref.types.uint8);
        var code = lib.axWrite16(id, reg, Math.round(value)%0x10000, errorPtr);
        return {code: code, error: errorPtr.deref()};
    },
    read8: function (id, reg) {
        var errorPtr = ref.alloc(ref.types.uint8), resultPtr = ref.alloc(ref.types.uint8);
        var code = lib.axRead8(id, reg, resultPtr, errorPtr);
        return {code: code, error: errorPtr.deref(), value:resultPtr.deref()};
    },
    read16: function (id, reg) {
        var errorPtr = ref.alloc(ref.types.uint8), resultPtr = ref.alloc(ref.types.uint16);
        var code = lib.axRead16(id, reg, resultPtr, errorPtr);
        return {code: code, error: errorPtr.deref(), value:resultPtr.deref()};
    },
    ping: function (id) {
        var errorPtr = ref.alloc(ref.types.uint8);
        var code = lib.axPing(id, errorPtr);
        return {code: code, error: errorPtr.deref()};
    },
    factoryReset: function (id) {
        var errorPtr = ref.alloc(ref.types.uint8);
        var code = lib.axFactoryReset(id, errorPtr);
        return {code: code, error: errorPtr.deref()};
    }
};
