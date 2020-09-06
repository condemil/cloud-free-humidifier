from serial import serial_for_url, SerialException, CR  # pip install pyserial

port = '/dev/cu.SLAB_USBtoUART'

try:
    s = serial_for_url(port, 115200, parity='N', rtscts=False, xonxoff=False, timeout=1)
except SerialException as e:
    sys.stderr.write("could not open port %r: %s\n" % (port, e))
    exit(1)

try:
    f = open('test.hex', 'wb')
    offset = 0
    size = 32768
    res = ''

    while offset < 4096 * 1024:  # 4MB
        s.write('fota r {} {}\n'.format(offset, size).encode())
        print(s.read_until(terminator=CR))

        while True:
            ch = s.read(1).decode()
            # print(ch)

            if ch in (' ', '-', '\n', '\r', '$'):
                continue

            res += ch

            if len(res) == 2:
                # print(res)
                f.write(bytes.fromhex(res))
                res = ''

            if ch == '':
                offset += size
                break
except KeyboardInterrupt:
    pass

f.close()
