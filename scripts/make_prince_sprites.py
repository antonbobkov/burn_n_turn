"""
Draws all prince animation frames, pixel by pixel, based on the princess frames.
Output: assets/princess/prince*.bmp, plus prince.txt and prince_die.txt, the
lists of frames the game reads. Also the tiny tower figures in assets/castle/.

The two walking frames are drawn by hand below, as little text pictures that
keep her pose: same head, same feet, same arm swing. Her long hair became a
short cape, which moves the way her hair does.

The burning frames are worked out from hers: her flames stay exactly where they
are, and whatever is left of her body is swapped for the same spot on the prince.

All frames are 16x16, 24bpp, uncompressed BMP. Pure black means see-through in
the game, so the prince must never use it. Nothing but Python itself is needed.

Usage:
    python scripts/make_prince_sprites.py
    python scripts/make_prince_sprites.py --preview some/folder/sheet.png
    python scripts/make_prince_sprites.py --out-dir some/folder
"""

import argparse
import os
import struct
import zlib

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)
ASSETS = os.path.join(PROJECT_ROOT, 'assets')
FOLDER = os.path.join(ASSETS, 'princess')
CASTLE_FOLDER = os.path.join(ASSETS, 'castle')

IMG_W, IMG_H, BPP = 16, 16, 24
BYTES_PER_PIXEL = BPP // 8
ROW_STRIDE = ((IMG_W * BYTES_PER_PIXEL + 3) // 4) * 4
PIXEL_OFFSET = 54  # standard for 24bpp with no palette

BLACK = (0, 0, 0)

# The prince's colors, all picked from the same few brightness steps the other
# sprites use (0, 63, 127, 191, 255).
COLORS = {
    '.': BLACK,
    'T': (0, 127, 255),    # tunic: bright blue, lighter than the mage's robe
    'Y': (255, 255, 0),    # crown and belt: the same gold as her hair
    'S': (255, 191, 127),  # skin: the same as hers
    'H': (191, 127, 0),    # short hair
    'C': (191, 0, 0),      # cape
    'P': (191, 191, 191),  # trousers
    'B': (127, 63, 0),     # boots
}

# Facing right, just like her frames. The arm, the back and the feet are in the
# same spots as hers on each frame, so the two walk in step.
WALK_FRAMES = [
    [
        '................',
        '................',
        '................',
        '.....Y.Y.Y......',
        '.....YYYYY......',
        '......HHH.......',
        '.....HHSS.......',
        '......HT........',
        '....CCTTTT......',
        '....CCTTT.S.....',
        '....CCTTT.......',
        '....CCYYY.......',
        '.....CTTT.......',
        '......PPP.......',
        '......P.P.......',
        '......B.B.......',
    ],
    [
        '................',
        '................',
        '................',
        '.....Y.Y.Y......',
        '.....YYYYY......',
        '......HHH.......',
        '.....HHSS.......',
        '......HT........',
        '.....CTTT.......',
        '....CCTTTT......',
        '....CCTTTS......',
        '....CCYYY.......',
        '....CCTTT.......',
        '......PPP.......',
        '......P.P.......',
        '.....B...B......',
    ],
]

# The tiny figure shown on the tower for a captured prince.
PRINCE_FIGURE = [
    '..',
    'YY',
    'HS',
    'TT',
    'TT',
]

# Where the first figure sits in the castle pictures.
FIGURE_LEFT, FIGURE_TOP, FIGURE_W, FIGURE_H = 8, 9, 2, 5

WALK_NAMES = ['prince0000.bmp', 'prince0001.bmp']
DIE_COUNT = 7

# Her fire colors. These are copied over as they are.
FLAME_COLORS = {(255, 0, 0), (191, 0, 0), (127, 0, 0)}

# On the very first burning frame the prince is still whole, with fire around him.
# After that, only the parts of him that survive on her frame are left.
FRAMES_STILL_WHOLE = 1


def read_bmp(path):
    """Gives back the 54 header bytes and the rows of (r, g, b), top row first."""
    with open(path, 'rb') as f:
        data = f.read()
    assert data[0:2] == b'BM', f'Not a BMP file: {path}'
    width, height, _, bpp = struct.unpack('<iiHH', data[18:30])
    assert (width, height, bpp) == (IMG_W, IMG_H, BPP), f'Unexpected format: {path}'
    rows = []
    for row in range(IMG_H):
        # The file keeps its rows from the bottom up, and each pixel as b, g, r.
        start = PIXEL_OFFSET + (IMG_H - 1 - row) * ROW_STRIDE
        rows.append([(data[start + x * 3 + 2], data[start + x * 3 + 1], data[start + x * 3])
                     for x in range(IMG_W)])
    return data[:PIXEL_OFFSET], rows


def write_bmp(path, header, rows):
    pixel_data = bytearray()
    for row in reversed(rows):
        row_bytes = bytearray()
        for r, g, b in row:
            row_bytes += bytes((b, g, r))
        row_bytes += b'\x00' * (ROW_STRIDE - len(row_bytes))
        pixel_data += row_bytes
    with open(path, 'wb') as f:
        f.write(header)
        f.write(pixel_data)


def grid_to_rows(grid):
    assert len(grid) == IMG_H and all(len(line) == IMG_W for line in grid)
    return [[COLORS[ch] for ch in line] for line in grid]


def make_die_frame(index, her_rows, body_rows):
    """Her fire stays in place, and what's left of her turns into him."""
    rows = []
    for y in range(IMG_H):
        row = []
        for x in range(IMG_W):
            her = her_rows[y][x]
            body = body_rows[y][x]
            if her in FLAME_COLORS:
                row.append(her)
            elif her == BLACK and index >= FRAMES_STILL_WHOLE:
                row.append(BLACK)
            else:
                row.append(body)
        rows.append(row)
    return rows


def make_all_frames():
    """Gives back a list of (file name, header, rows, the matching princess rows)."""
    frames = []
    for i, grid in enumerate(WALK_FRAMES):
        header, her_rows = read_bmp(os.path.join(FOLDER, f'princess{i:04d}.bmp'))
        frames.append((WALK_NAMES[i], header, grid_to_rows(grid), her_rows))
    body_rows = grid_to_rows(WALK_FRAMES[0])
    for i in range(DIE_COUNT):
        header, her_rows = read_bmp(os.path.join(FOLDER, f'princess_die{i:04d}.bmp'))
        rows = make_die_frame(i, her_rows, body_rows)
        frames.append((f'prince_die{i:04d}.bmp', header, rows, her_rows))
    return frames


def read_any_bmp(path):
    """Gives back (width, height, rows of (r, g, b), top row first), any 24bpp size."""
    with open(path, 'rb') as f:
        data = f.read()
    width, height, _, bpp = struct.unpack('<iiHH', data[18:30])
    assert bpp == BPP, f'Unexpected format: {path}'
    stride = ((width * BYTES_PER_PIXEL + 3) // 4) * 4
    rows = []
    for row in range(height):
        start = PIXEL_OFFSET + (height - 1 - row) * stride
        rows.append([(data[start + x * 3 + 2], data[start + x * 3 + 1], data[start + x * 3])
                     for x in range(width)])
    return width, height, rows


def write_figure_bmp(path, rows):
    """Saves a tiny tower figure, using the same plain 24bpp format as all the others."""
    height, width = len(rows), len(rows[0])
    stride = ((width * BYTES_PER_PIXEL + 3) // 4) * 4
    pixel_data = bytearray()
    for row in reversed(rows):
        row_bytes = bytearray()
        for r, g, b in row:
            row_bytes += bytes((b, g, r))
        pixel_data += row_bytes + b'\x00' * (stride - len(row_bytes))
    header = b'BM' + struct.pack('<IHHI', PIXEL_OFFSET + len(pixel_data), 0, 0, PIXEL_OFFSET)
    header += struct.pack('<IiiHHIIiiII', 40, width, height, 1, BPP, 0, len(pixel_data),
                          2835, 2835, 0, 0)
    with open(path, 'wb') as f:
        f.write(header + pixel_data)


def make_tower_figures():
    """Gives back the princess and prince figures shown on the tower.

    Hers is exactly what castle0001.bmp adds to castle0000.bmp, so the tower looks
    the same as it did before for a tower of princesses.
    """
    _, _, empty = read_any_bmp(os.path.join(CASTLE_FOLDER, 'castle0000.bmp'))
    _, _, one = read_any_bmp(os.path.join(CASTLE_FOLDER, 'castle0001.bmp'))
    princess = [[one[y][x] if one[y][x] != empty[y][x] else BLACK
                 for x in range(FIGURE_LEFT, FIGURE_LEFT + FIGURE_W)]
                for y in range(FIGURE_TOP, FIGURE_TOP + FIGURE_H)]
    prince = [[COLORS[ch] for ch in line] for line in PRINCE_FIGURE]
    return princess, prince


def write_list(path, names):
    # Same as her lists: Windows line ends, and no line end after the last name.
    with open(path, 'wb') as f:
        f.write('\r\n'.join(names).encode('ascii'))


def write_png(path, width, height, pixels):
    """Saves a list of rows of (r, g, b) as a PNG, so the sheet opens anywhere."""
    def chunk(kind, body):
        crc = zlib.crc32(kind + body) & 0xffffffff
        return struct.pack('>I', len(body)) + kind + body + struct.pack('>I', crc)

    raw = bytearray()
    for row in pixels:
        raw.append(0)
        for pixel in row:
            raw += bytes(pixel)
    with open(path, 'wb') as f:
        f.write(b'\x89PNG\r\n\x1a\n')
        f.write(chunk(b'IHDR', struct.pack('>IIBBBBB', width, height, 8, 2, 0, 0, 0)))
        f.write(chunk(b'IDAT', zlib.compress(bytes(raw), 9)))
        f.write(chunk(b'IEND', b''))


def paste(canvas, rows, left, top, zoom, bg):
    # Each sprite pixel becomes a zoom-by-zoom block, with no smoothing at all.
    for y, row in enumerate(rows):
        for x, color in enumerate(row):
            shown = bg if color == BLACK else color
            for dy in range(zoom):
                for dx in range(zoom):
                    canvas[top + y * zoom + dy][left + x * zoom + dx] = shown


def make_preview(path, frames):
    """Princess above prince at big size, then everyone in a line at small sizes."""
    zoom = 8
    gap = 6
    cell = IMG_W * zoom + gap
    black = (0, 0, 0)
    road = (39, 39, 39)  # the road's gray, to check he shows up on it as well
    page = (70, 70, 80)

    lineup = [read_bmp(os.path.join(ASSETS, name))[1] for name in (
        'knight/knight0000.bmp', 'mage/mage0000.bmp', 'trader/trader0000.bmp',
        'skelly/skelly0000.bmp')]
    lineup += [frames[0][3], frames[0][2], frames[1][3], frames[1][2]]
    small_sizes = (4, 2, 1)  # 2 is the size the game shows them at

    width = len(frames) * cell + gap
    height = gap + 2 * cell + sum(IMG_H * size + gap for size in small_sizes)
    canvas = [[page] * width for _ in range(height)]

    # Hers on top, his below, with each frame lined up with its match.
    for i, (_, _, rows, her_rows) in enumerate(frames):
        paste(canvas, her_rows, gap + i * cell, gap, zoom, black)
        paste(canvas, rows, gap + i * cell, gap + cell, zoom, black)

    # The line of sprites, on the black field on the left, on the road on the right.
    top = gap + 2 * cell
    for size in small_sizes:
        for side, bg in enumerate((black, road)):
            for k, rows in enumerate(lineup):
                left = gap + side * (width // 2) + k * (IMG_W * size + 2)
                paste(canvas, rows, left, top, size, bg)
        top += IMG_H * size + gap

    write_png(path, width, height, canvas)
    print(f'Preview: {path}')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Draw the prince frames.')
    parser.add_argument('--out-dir', default=FOLDER, help='where to save the frames')
    parser.add_argument('--preview', help='also save a zoomed picture of all frames here')
    args = parser.parse_args()

    all_frames = make_all_frames()
    os.makedirs(args.out_dir, exist_ok=True)
    for name, frame_header, frame_rows, _ in all_frames:
        write_bmp(os.path.join(args.out_dir, name), frame_header, frame_rows)
    frame_names = [frame[0] for frame in all_frames]
    write_list(os.path.join(args.out_dir, 'prince.txt'), frame_names[:len(WALK_NAMES)])
    write_list(os.path.join(args.out_dir, 'prince_die.txt'), frame_names[len(WALK_NAMES):])
    print(f'Written: {len(all_frames)} frames and 2 lists to {args.out_dir}')

    # The list order matters: the game picks a figure by its place in this list.
    figure_names = ['royal_figure_princess.bmp', 'royal_figure_prince.bmp']
    figures_dir = CASTLE_FOLDER if args.out_dir == FOLDER else args.out_dir
    for figure_name, figure_rows in zip(figure_names, make_tower_figures()):
        write_figure_bmp(os.path.join(figures_dir, figure_name), figure_rows)
    write_list(os.path.join(figures_dir, 'royal_figures.txt'), figure_names)
    print(f'Written: 2 tower figures and a list to {figures_dir}')

    if args.preview:
        make_preview(args.preview, all_frames)
