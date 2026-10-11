#!/usr/bin/env python3
"""Generate the OLED font table in firmware/app/screen.c.

The style target is a classic Japanese instrument face (Nippon Seiki / Denso):
tall, condensed, uniform-stroke glyphs. Rather than embedding a font file, we
rasterize a free condensed face (Nimbus Sans Narrow Bold, URW's Helvetica
clone, ghostscript fonts) on a supersampled cell canvas -- all glyphs at one
common size (cap height fills the cell minus descender room), baseline-
anchored, horizontally centered -- then downscale and threshold to 1-bit.
Output is the same `const uint16_t font[]` table the screen driver has always
used (bit 15 = leftmost pixel).

Usage: python3 gen_font.py [font-file]   (then rebuild the firmware)
"""

from PIL import Image, ImageDraw, ImageFont

DEFAULT_FONT = "/usr/share/fonts/gsfonts/NimbusSansNarrow-Bold.otf"
CELL_W = 16          # OLED glyph cell, matches screen.c
CELL_H = 26
CAP_HEIGHT = 22      # px of the cell the caps/digits fill
BASELINE_ROW = 24    # baseline row; leaves 2 px below for descenders
THRESHOLD = 128      # binarization level after downscale
SCALE = 8            # supersample factor
CONDENSE = 1.15      # extra horizontal squeeze: the canvas is wider and gets
                     # squeezed back into the cell -> narrower glyphs
REF_SIZE = 100       # reference size for measuring cap height

FIRST, LAST = 32, 126  # printable ASCII, same range screen.c supports


def cap_height(face):
    img = Image.new("L", (REF_SIZE * 2, REF_SIZE * 2), 0)
    ImageDraw.Draw(img).text((REF_SIZE // 2, REF_SIZE // 2), "H", 255, font=face)
    bbox = img.getbbox()
    return bbox[3] - bbox[1]


def glyph_rows(ch, face, canvas):
    """Render one char on the supersampled canvas, return CELL_H ints (bit15 = left)."""
    big = Image.new("L", canvas, 0)
    ImageDraw.Draw(big).text(
        (canvas[0] // 2, BASELINE_ROW * SCALE), ch, fill=255, font=face, anchor="ms"
    )
    small = big.resize((CELL_W, CELL_H), Image.LANCZOS)
    px = small.load()
    return [
        sum(0x8000 >> x for x in range(CELL_W) if px[x, r] >= THRESHOLD)
        for r in range(CELL_H)
    ]


def main():
    import sys

    path = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_FONT
    ref = ImageFont.truetype(path, REF_SIZE)
    size = round(REF_SIZE * CAP_HEIGHT * SCALE / cap_height(ref))
    face = ImageFont.truetype(path, size)
    canvas = (round(CELL_W * SCALE * CONDENSE), CELL_H * SCALE)

    lines = []
    for code in range(FIRST, LAST + 1):
        ch = chr(code)
        rows = glyph_rows(ch, face, canvas)
        label = ch if ch != "\\" else "backslash"
        vals = ",".join(f"0x{r:04X}" for r in rows)
        lines.append(f"  {vals}, // Ascii = [{label}]")

    table = "// clang-format off\nconst uint16_t font[] = {\n"
    table += "\n".join(lines)
    table += "\n};\n// clang-format on\n"

    src_path = __file__.rsplit("/", 2)[0] + "/app/screen.c"
    src = open(src_path).read()
    start = src.index("// clang-format off")
    end = src.index("// clang-format on") + len("// clang-format on")
    open(src_path, "w").write(src[:start] + table + src[end:])
    print(f"wrote {LAST - FIRST + 1} glyphs ({FIRST}..{LAST}) into {src_path}")


if __name__ == "__main__":
    main()
