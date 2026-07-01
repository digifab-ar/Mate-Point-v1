#!/usr/bin/env python3
"""Convert PNG to LVGL 8 RGB565 TRUE_COLOR C array (LV_COLOR_DEPTH=16, LV_COLOR_16_SWAP=0)."""

import argparse
import os
import re
import sys

try:
    from PIL import Image
except ImportError:
    print("pip install Pillow", file=sys.stderr)
    sys.exit(1)


def rgb888_to_rgb565(r: int, g: int, b: int) -> int:
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)


def c_name_from_path(path: str) -> str:
    base = os.path.splitext(os.path.basename(path))[0]
    base = re.sub(r"[^a-zA-Z0-9]+", "_", base).strip("_").lower()
    return f"img_{base}"


def emit_c(path: str, out_path: str) -> None:
    im = Image.open(path).convert("RGBA")
    w, h = im.size
    px = im.load()
    name = c_name_from_path(path)
    map_name = f"{name}_map"
    dsc_name = name

    # LVGL TRUE_COLOR @ LV_COLOR_DEPTH=16 stores little-endian RGB565 bytes in uint8_t[].
    pixels = []
    for y in range(h):
        for x in range(w):
            r, g, b, a = px[x, y]
            if a < 128:
                r, g, b = 0, 0, 0
            word = rgb888_to_rgb565(r, g, b)
            pixels.append(word & 0xFF)
            pixels.append((word >> 8) & 0xFF)

    with open(out_path, "w", encoding="utf-8") as f:
        f.write("#include \"lvgl.h\"\n\n")
        f.write(f"const LV_ATTRIBUTE_MEM_ALIGN uint8_t {map_name}[] = {{\n")
        for i in range(0, len(pixels), 16):
            chunk = pixels[i : i + 16]
            line = ", ".join(f"0x{b:02x}" for b in chunk)
            f.write(f"  {line},\n")
        f.write("};\n\n")
        f.write(f"const lv_img_dsc_t {dsc_name} = {{\n")
        f.write("  .header.cf = LV_IMG_CF_TRUE_COLOR,\n")
        f.write("  .header.always_zero = 0,\n")
        f.write("  .header.reserved = 0,\n")
        f.write(f"  .header.w = {w},\n")
        f.write(f"  .header.h = {h},\n")
        f.write(f"  .data_size = {len(pixels)},\n")
        f.write(f"  .data = {map_name},\n")
        f.write("};\n")

    print(f"{path} -> {out_path} ({w}x{h}, {len(pixels)} bytes)")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("inputs", nargs="+")
    parser.add_argument("-o", "--out-dir", required=True)
    args = parser.parse_args()
    os.makedirs(args.out_dir, exist_ok=True)
    for inp in args.inputs:
        out = os.path.join(args.out_dir, c_name_from_path(inp) + ".c")
        emit_c(inp, out)


if __name__ == "__main__":
    main()
