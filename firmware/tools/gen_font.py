#!/usr/bin/env python3
"""Generate the OLED font tables in firmware/app/screen.c.

Both fonts are classic bitmap terminal fonts (Terminus, SIL OFL) rendered
pixel-perfect by FreeType at their native size via bilevel getmask2, so every
pixel is exactly as the font designer drew it -- no antialiasing, no downscale.

  font[]        Terminus u32 bold: 16x32 cells (the full screen height) for
                values -- tall, chunky gauge digits.
  font_small[]  Terminus u14: 8x14 cells for units ("km/h", "km", ...),
                baseline-aligned with the big digits.

Usage: python3 gen_font.py   (then rebuild the firmware)
"""

from PIL import Image, ImageFont

FIRST, LAST = 32, 126  # printable ASCII, same range screen.c supports

BIG = dict(name="font", c_type="uint16_t", cell_w=16, cell_h=32, baseline=26,
           font_path="/usr/share/fonts/misc/ter-u32b.otb", font_size=32)
SMALL = dict(name="font_small", c_type="uint8_t", cell_w=8, cell_h=14,
             baseline=12,
             font_path="/usr/share/fonts/misc/ter-u14n.otb", font_size=14)


def glyph_rows(ch, face, spec):
    """Render one char into its cell, as cell_h ints (MSB = leftmost pixel)."""
    cell_w, cell_h = spec["cell_w"], spec["cell_h"]
    mask, off = face.getmask2(ch, mode="1")
    w, h = mask.size
    data = bytes(mask)
    ascent = face.getmetrics()[0]
    paste_x = (cell_w - w) // 2 - off[0]
    paste_y = spec["baseline"] - ascent + off[1]
    msb = 1 << (cell_w - 1)
    rows = [0] * cell_h
    for mr in range(h):
        y = paste_y + mr
        if not 0 <= y < cell_h:
            continue
        val = 0
        for mc in range(w):
            if data[mr * w + mc]:
                x = paste_x + mc
                if 0 <= x < cell_w:
                    val |= msb >> x
        rows[y] = val
    return rows


def build_table(spec, face, nl):
    lines = []
    for code in range(FIRST, LAST + 1):
        ch = chr(code)
        rows = glyph_rows(ch, face, spec)
        label = ch if ch != "\\" else "backslash"
        vals = ",".join(f"0x{r:02X}" if spec["cell_w"] <= 8 else f"0x{r:04X}"
                        for r in rows)
        lines.append(f"  {vals}, // Ascii = [{label}]")
    body = nl.join(lines)
    return (f"// clang-format off{nl}"
            f"static const {spec['c_type']} {spec['name']}[] = {{{nl}"
            f"{body}{nl}}};{nl}// clang-format on")


def splice(src, spec, table):
    import re

    pat = (r"// clang-format off\r?\nstatic const \w+ "
           + re.escape(spec["name"]) + r"\[\] = \{")
    found = re.search(pat, src)
    if not found:
        # New table: insert after the last existing clang-format block.
        anchor = src.rindex("// clang-format on")
        end = anchor + len("// clang-format on")
        nl = "\r\n" if "\r\n" in src else "\n"
        return src[:end] + nl + nl + table + src[end:]
    start = found.start()
    end = src.index("// clang-format on", start) + len("// clang-format on")
    return src[:start] + table + src[end:]


def main():
    src_path = __file__.rsplit("/", 2)[0] + "/app/screen.c"
    src = open(src_path).read()
    nl = "\r\n" if "\r\n" in src else "\n"

    for spec in (BIG, SMALL):
        face = ImageFont.truetype(spec["font_path"], spec["font_size"])
        src = splice(src, spec, build_table(spec, face, nl))

    open(src_path, "w").write(src)
    print(f"wrote {LAST - FIRST + 1} glyphs ({FIRST}..{LAST}) x 2 tables into {src_path}")


if __name__ == "__main__":
    main()
