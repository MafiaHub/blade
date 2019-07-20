# this generates a new icon from a 32-bit BMP file
# make sure the image is upside-down, since we don't flip it in this bulk

# 1. Load an image first
print('Loading icon...')
f = open('c:/blade/stuff/icon/Quake2.bmp', 'rb')
d = f.read()
f.close()

# 2. Skip the header part and convert data to byte array
print('Skipping the header part...')
c = bytearray(d[53:])

# 3. Rotate ABGR to BGRA
print('Converting from ABGR to RGBA...')
for i in range(0, len(c), 4):
    try:
        r = c[i+3]
        g = c[i+2]
        b = c[i+1]
        a = c[i]
        c[i] = r
        c[i+1] = g
        c[i+2] = b
        c[i+3] = a
    except:
        pass

# 4. print in IDLE
', '.join('0x'+hex(x)[2:] for x in c)
