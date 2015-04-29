#!/usr/local/bin/python3

import sys, base64
import qrcode

if __name__ == '__main__':
    if len(sys.argv) < 2:
        exit()
    with open(sys.argv[1], "rb") as in_file:
        data = in_file.read()
        base64str = base64.b64encode(data)
        print(base64str)

        qr = qrcode.QRCode(version = 1,
                           error_correction = qrcode.constants.ERROR_CORRECT_L,
                           box_size = 10,
                           border = 4,)
        qr.add_data(base64str)
        qr.make(fit = True)
        img = qr.make_image()
        img.save("res.png")
