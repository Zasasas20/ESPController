import serial

ser = serial.Serial("/dev/ttyACM1", 115200)

def openValve(valveID):
    ser.write(bytes(f'P{valveID}', encoding='utf8'))
    status = ser.read(1).decode() # Will receive a single bit representing 0 - Success or 1 - Fail

    return status

def getSingleValveStatus(valveID):
    ser.write(bytes(f'S{valveID}', encoding='utf8'))
    status = ser.read(1).decode() # Will receive a single bit representing 0 - In use or 1 - Available

    return status

def openAllValves():
    ser.write(b'PA')
    status = ser.read(1).decode() # Will receive a single bit representing 0 - Success or 1 - Fail

    return status

def getAllValvesStatus():
    ser.write(b'SA')
    statusString = ser.readline().decode() # Will receive a string (1,1,1,1,0\n), idx:0, 0 means valve 1 is unavailable
    valveStatus = [] # return only availability, ID should be idx+1

    for status in statusString.replace('\n', '').replace('\r', '').split(','):
        valveStatus.append(status)

    return valveStatus

def updateValveCount(amount): # Ideally based on camera from ROI, count valves and use to update ESP32 detection
    ser.write(bytes(f'U{amount}', encoding='utf8'))
    status = ser.read().decode() # Will receive amount of valves back

    return status

def setAllValvesTimer(timeout):
    ser.write(b'TA')
    ser.write(bytes(f' {timeout}\n', encoding='utf8'))
    status = ser.readline().decode() # Will receive new timer

    return status

def setValveTimer(valveID, timeout):
    ser.write(bytes(f'T{valveID}', encoding='utf8'))
    ser.write(bytes(f' {timeout}\n', encoding='utf8'))
    status = ser.readline().decode() # Will receive new timer

    return status