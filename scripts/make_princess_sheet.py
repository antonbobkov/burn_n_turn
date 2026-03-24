"""
Combines all princess animation frames into a single horizontal sprite sheet BMP.
Output: assets/princess/princess_sheet.bmp

Layout: regular animation frames first, then die animation frames.
All source frames must be 16x16, 24bpp, uncompressed BMP.

Usage:
    python scripts/make_princess_sheet.py
"""

import struct
import os

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)
FOLDER = os.path.join(PROJECT_ROOT, 'assets', 'princess')

FILES = [
    'princess0000.bmp',
    'princess0001.bmp',
    'princess_die0000.bmp',
    'princess_die0001.bmp',
    'princess_die0002.bmp',
    'princess_die0003.bmp',
    'princess_die0004.bmp',
    'princess_die0005.bmp',
    'princess_die0006.bmp',
]

IMG_W, IMG_H, BPP = 16, 16, 24
BYTES_PER_PIXEL = BPP // 8
SRC_ROW_STRIDE = ((IMG_W * BYTES_PER_PIXEL + 3) // 4) * 4
PIXEL_OFFSET = 54  # standard for 24bpp with no palette


def read_bmp_pixels(path):
    with open(path, 'rb') as f:
        data = f.read()
    assert data[0:2] == b'BM', f'Not a BMP file: {path}'
    rows = []
    for row in range(IMG_H):
        start = PIXEL_OFFSET + row * SRC_ROW_STRIDE
        rows.append(data[start : start + IMG_W * BYTES_PER_PIXEL])
    return rows


def make_sheet(files, out_path):
    n = len(files)
    total_width = IMG_W * n
    dst_row_stride = ((total_width * BYTES_PER_PIXEL + 3) // 4) * 4

    pixels = [read_bmp_pixels(os.path.join(FOLDER, f)) for f in files]

    pixel_data = bytearray()
    for row in range(IMG_H):
        row_bytes = bytearray()
        for img in pixels:
            row_bytes += img[row]
        row_bytes += b'\x00' * (dst_row_stride - len(row_bytes))
        pixel_data += row_bytes

    pixel_data_size = len(pixel_data)
    file_size = PIXEL_OFFSET + pixel_data_size

    bmp_file_header = struct.pack('<2sIHHI', b'BM', file_size, 0, 0, PIXEL_OFFSET)
    dib_header = struct.pack('<IiiHHIIiiII',
        40,
        total_width,
        IMG_H,
        1,
        BPP,
        0,
        pixel_data_size,
        2835, 2835,
        0, 0,
    )

    with open(out_path, 'wb') as f:
        f.write(bmp_file_header)
        f.write(dib_header)
        f.write(pixel_data)

    print(f'Written: {out_path}')
    print(f'  Size: {total_width}x{IMG_H} px, {file_size} bytes')


if __name__ == '__main__':
    out = os.path.join(FOLDER, 'princess_sheet.bmp')
    make_sheet(FILES, out)
