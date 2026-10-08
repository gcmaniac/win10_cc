#!/usr/bin/env python3
"""
Ekstraktor Aset Chip's Challenge
Mengekstrak bitmap sprite dari CHIPS.EXE (16-bit NE), mengonversi ke RGB standar,
serta menyalin audio WAV/MIDI dan data CHIPS.DAT.
"""

import os
import struct
import shutil
from PIL import Image

def extract_bmp(exe_path, offset, size, out_path):
    with open(exe_path, 'rb') as f:
        f.seek(offset)
        dib = f.read(size)
    biSize, biWidth, biHeight, biPlanes, biBitCount, biCompression, biSizeImage, biXPels, biYPels, biClrUsed, biClrImportant = struct.unpack('<IIIHHIIIIII', dib[:40])
    num_colors = biClrUsed
    if num_colors == 0 and biBitCount <= 8:
        num_colors = 1 << biBitCount
    palette_size = num_colors * 4
    bfOffBits = 14 + biSize + palette_size
    bfSize = 14 + len(dib)
    file_hdr = struct.pack('<2sIHHI', b'BM', bfSize, 0, 0, bfOffBits)
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    with open(out_path, 'wb') as f:
        f.write(file_hdr)
        f.write(dib)
    print(f"[OK] Ekstrak mentah: {out_path} ({biWidth}x{biHeight}, {biBitCount}bpp, kompresi={biCompression})")

def main():
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    exe_path = os.path.join(root, "chips_challenge", "CHIPS.EXE")
    if not os.path.exists(exe_path):
        print(f"[ERROR] Tidak dapat menemukan {exe_path}")
        return 1

    print("=== 1. Mengekstrak Bitmaps Asli dari CHIPS.EXE ===")
    sprites_dir = os.path.join(root, "assets", "sprites")
    os.makedirs(sprites_dir, exist_ok=True)

    raw_bmps = [
        (0xD800,  73728, "tiles_color.bmp"),
        (0x1F800, 67584, "tiles_anim.bmp"),
        (0x30000, 27136, "tiles_mask.bmp"),
        (0x36A00, 6656,  "splash.bmp"),
        (0x38400, 7168,  "digits.bmp"),
        (0x3A000, 6144,  "ui_panel.bmp"),
        (0x3B800, 17408, "viewport_bg.bmp")
    ]

    for off, sz, name in raw_bmps:
        extract_bmp(exe_path, off, sz, os.path.join(sprites_dir, name))

    print("\n=== 2. Mengonversi ke Format RGB 24-bit Standar ===")
    for _, _, name in raw_bmps:
        p = os.path.join(sprites_dir, name)
        try:
            with Image.open(p) as img:
                out_p = os.path.join(sprites_dir, name.replace(".bmp", "_rgb.bmp"))
                img.convert("RGB").save(out_p)
                print(f"[OK] Format RGB: {out_p} ({img.size[0]}x{img.size[1]})")
        except Exception as e:
            print(f"[WARN] Gagal konversi {name}: {e}")

    print("\n=== 3. Mengekstrak Icon Asli (RT_ICON) ===")
    try:
        with open(exe_path, "rb") as f:
            f.seek(0x41200)
            dib_icon = f.read(744)
        ico_path = os.path.join(root, "assets", "chips.ico")
        icondir = struct.pack("<HHH", 0, 1, 1)
        icondirentry = struct.pack("<BBBBHHII", 32, 32, 16, 0, 1, 4, len(dib_icon), 22)
        with open(ico_path, "wb") as f:
            f.write(icondir)
            f.write(icondirentry)
            f.write(dib_icon)
        
        # Simpan dalam variasi multi-resolusi untuk tampilan Windows Explorer tajam
        with Image.open(ico_path) as orig_ico:
            sizes = [(16, 16), (32, 32), (48, 48), (64, 64), (128, 128)]
            orig_ico.save(ico_path, format="ICO", sizes=sizes)
        print(f"[OK] Icon disimpan: {ico_path} (multi-res 16px..128px)")
    except Exception as e:
        print(f"[WARN] Gagal mengekstrak icon: {e}")

    print("\n=== 4. Menyalin Berkas Audio dan Data Level ===")
    audio_dst = os.path.join(root, "assets", "audio")
    data_dst = os.path.join(root, "assets", "data")
    os.makedirs(audio_dst, exist_ok=True)
    os.makedirs(data_dst, exist_ok=True)

    src_dir = os.path.join(root, "chips_challenge")
    for fname in os.listdir(src_dir):
        up = fname.upper()
        full_src = os.path.join(src_dir, fname)
        if up.endswith(".WAV") or up.endswith(".MID"):
            shutil.copy(full_src, os.path.join(audio_dst, up))
            print(f"[OK] Audio: {up}")
        elif up == "CHIPS.DAT":
            shutil.copy(full_src, os.path.join(data_dst, up))
            print(f"[OK] Data: {up}")

    print("\n[SUCCESS] Semua aset siap digunakan untuk Engine C Native!")
    return 0

if __name__ == "__main__":
    main()
