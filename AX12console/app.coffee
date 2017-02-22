nesh = require 'nesh'
Table = require 'cli-table'
colors = require 'colors'
program = require('commander')

myParse = (val) ->
    return parseInt val
program.version('0.1.0')
    .option('-b, --baudrate <n>', 'set AX12 communication baudrate (default 115200)', myParse, 115200)
    .parse(process.argv)

commands = new Table({
  chars: { 'top': '' , 'top-mid': '' , 'top-left': '' , 'top-right': ''
         , 'bottom': '' , 'bottom-mid': '' , 'bottom-left': '' , 'bottom-right': ''
         , 'left': '' , 'left-mid': '' , 'mid': '' , 'mid-mid': ''
         , 'right': '' , 'right-mid': '' , 'middle': ' ' },
  style: { 'padding-left': 2, 'padding-right': 0}
})
commands.push(
    ['write'.red, '<ID>, <address>, <value>', 'write data in memory'],
    ['read'.red, '<ID>, <address>', 'read memory register'],
    ['dump'.red, '<ID>', 'dump the whole memory (EEPROM + RAM)'],
    ['status'.red, '<ID>', 'print the error flags'],
    ['scan()'.red, '', 'list all the connected AX12']
)
registers =
    MODEL: {size: 16, write: no, address: 0x00}
    VERSION: {size: 8, write: no, address: 0x02}
    ID: {size: 8, write: yes, address: 0x03}
    BAUDRATE: {size: 8, write: yes, address: 0x04}
    RETURN_DELAY: {size: 8, write: yes, address: 0x05}
    CW_LIMIT: {size: 16, write: yes, address: 0x06}
    CCW_LIMIT: {size: 16, write: yes, address: 0x08}
    MAX_TEMP: {size: 8, write: yes, address: 0x0B}
    MIN_VOLTAGE: {size: 8, write: yes, address: 0x0C}
    MAX_VOLTAGE: {size: 8, write: yes, address: 0x0D}
    MAX_TORQUE_EE: {size: 16, write: yes, address: 0x0E}
    RETURN_LEVEL: {size: 8, write: yes, address: 0x10}
    ALARM_LED: {size: 8, write: yes, address: 0x11}
    ALARM_SHUTDOWN: {size: 8, write: yes, address: 0x12}
    TORQUE_ENABLE: {size: 8, write: yes, address: 0x18}
    LED: {size: 8, write: yes, address: 0x19}
    GOAL_POSITION: {size: 16, write: yes, address: 0x1E}
    GOAL_SPEED: {size: 16, write: yes, address: 0x20}
    MAX_TORQUE: {size: 16, write: yes, address: 0x22}
    POSITION: {size: 16, write: no, address: 0x24}
    SPEED: {size: 16, write: no, address: 0x26}
    LOAD: {size: 16, write: no, address: 0x28}
    VOLTAGE: {size: 8, write: no, address: 0x2A}
    TEMPERATURE: {size: 8, write: no, address: 0x2B}
    MOVING: {size: 8, write: no, address: 0x2E}
    LOCK: {size: 8, write: yes, address: 0x2F}
registersTable = new Table(
    chars: { 'top': '' , 'top-mid': '' , 'top-left': '' , 'top-right': ''
        , 'bottom': '' , 'bottom-mid': '' , 'bottom-left': '' , 'bottom-right': ''
        , 'left': '' , 'left-mid': '' , 'mid': '' , 'mid-mid': ''
        , 'right': '' , 'right-mid': '' , 'middle': ' ' },
    style: { 'padding-left': 2, 'padding-right': 0}
)
registersTable.push([('0x' + reg.address.toString(16).toUpperCase()).blue, name, reg.size,
    if reg.write then 'read/write' else 'read only'.red]) for name, reg of registers

context = ->
    colors = require __dirname + '/node_modules/colors'
    Table = require __dirname + '/node_modules/cli-table'
    comm = require __dirname + '/../walkingdriver/ax-comm.js'

    comm.init 115200
    # force baudrate
    stty = require('child_process').spawn('stty', ['-F', '/dev/ttyS0', baudrate])
    stty.stdout.on 'data', (data) ->
        console.log("stdout: #{data}")

    __checkID = (id) ->
            console.log 'ERROR : Wrong ID (must be between 0 and 253)'.red unless 0 <= id < 254
            return 0 <= id < 254
    __checkAddress = (address) ->
        if typeof address is 'number'
            reg = register for name, register of registers when register.address == address
        else if typeof address is 'string' and registers[address]?
            reg = registers[address]
        else
            console.log 'ERROR : Wrong register'.red
            return
        return reg

    write = (id, address, value) ->
        unless 0 <= id <= 254
            console.log 'ERROR : Wrong ID (must be between 0 and 253)'.red
            return -1
        reg = __checkAddress(address)
        return -1 unless reg?

        unless typeof value is 'number' and not isNaN(value)
            console.log 'ERROR : value must be a number'.red
            return -1
        unless reg.write
            console.log 'ERROR : register #{address} is read-only'.red
            return -1

        result = comm.write8(id, reg.address, value) if reg.size == 8
        result = comm.write16(id, reg.address, value) if reg.size == 16
        return result.code

    read = (id, address) ->
        reg = __checkAddress(address)
        return -1 unless reg? && __checkID(id)

        result = comm.read8(id, reg.address) if reg.size == 8
        result = comm.read16(id, reg.address) if reg.size == 16
        return if result.code < 0 then result.code else result.value

    dump = (id) ->
        return -1 unless __checkID(id)
        return "AX12 #{id} not responding" if comm.ping(id).code < 0

        regsTable = new Table(
            chars: {'mid': '' , 'mid-mid': '', 'left-mid': '' , 'right-mid': ''},
            style: { 'padding-left': 2, 'padding-right': 0}
            head: ['', 'name'.red, 'size'.red, 'value'.red]
        )
        regsTable.push(['', '', '', ''])
        regsTable.push([('0x' + reg.address.toString(16).toUpperCase()).blue, name,
            reg.size, read(id, name)]) for name, reg of registers
        console.log "\n          AX12 #{id} memory dump".bold
        console.log regsTable.toString()
        return 0

    scan = ->
        ax12s = []
        comm.errorLog off
        for id in [0..253]
            ax12s.push id if comm.ping(id).code == 0
        comm.errorLog on
        console.log "Found #{(ax12s.length+'').bold.blue} AX12."
        return ax12s

    status = (id) ->
        return -1 unless __checkID(id)

        result = comm.ping(id)
        status = result.error.toString(2)
        status = '0' + status while status.length < 8
        console.log 'Instruction error flag set' if result.error & 0x40
        console.log 'Overload error flag set' if result.error & 0x20
        console.log 'Checksum error flag set' if result.error & 0x10
        console.log 'Range error flag set' if result.error & 0x08
        console.log 'Overheating error flag set' if result.error & 0x04
        console.log 'Angle limit error flag set' if result.error & 0x02
        console.log 'Input voltage error flag set' if result.error & 0x01
        return if result.code < 0 then result.code else status

    return 0

registersString = 'registers = ' + JSON.stringify(registers) + '\n'
registersString += "#{name} = '#{name}'\n" for name, reg of registers
registersString += "__dirname = '#{__dirname}'\n"
evalLines = (context + '').split('\n')
evalLines[0] = registersString + "baudrate = #{program.baudrate}\n"
evalLines.pop()
evalLines.pop()

options =
    welcome: """
                                 AX12 control console v1.0
        baudrate : #{(program.baudrate+'').green}
        available commands :
        #{commands.toString()}
        register names :
        #{registersTable.toString()}
        """
    prompt: 'AX12> '
    evalData: evalLines.join('\n')

nesh.loadLanguage 'coffee'
nesh.start options, (err) ->
    nesh.log.error err if err
