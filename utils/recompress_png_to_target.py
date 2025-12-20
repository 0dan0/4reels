#!/usr/bin/env python3
"""
recompress_png_to_target.py

Re-encode a PNG as a JPEG whose file size does not exceed the requested limit
(default: 20 480 bytes ≈ 20 KB).

usage:
    python recompress_png_to_target.py input.png [-o out.jpg] [-t 20480]
"""

import argparse
import io
import os
from PIL import Image

def jpeg_within_size(
    in_path: str,
    out_path: str,
    target_bytes: int = 20480,
    min_q: int = 1,
    max_q: int = 95,
    subsampling: int = 1,      # 0 - 4:4:4 , 1 - 4:2:2
    optimize: bool = True,
) -> None:
    """
    Binary-search JPEG quality so that the output is ≤ target_bytes.
    Saves the best candidate to *out_path*.
    """
    img = Image.open(in_path)
    if img.mode != "RGB":
        img = img.convert("RGB")

    lo, hi = min_q, max_q
    best_quality = None
    best_buf = None

    while lo <= hi:
        mid = (lo + hi) // 2
        buf = io.BytesIO()
        img.save(
            buf,
            format="JPEG",
            quality=mid,
            subsampling=subsampling,
            optimize=optimize,
        )
        size = buf.tell()

        if size <= target_bytes:
            best_quality = mid
            best_buf = buf.getvalue()
            lo = mid + 1          # try a higher (better) quality
        else:
            hi = mid - 1          # need stronger compression

    # If no setting hit the target, fall back to the lowest quality tried
    if best_buf is None:
        print(
            "⚠️  Could not reach the target size even at minimum quality; "
            f"writing with quality={min_q}"
        )
        img.save(
            out_path,
            format="JPEG",
            quality=min_q,
            subsampling=subsampling,
            optimize=optimize,
        )
    else:
        with open(out_path, "wb") as f:
            f.write(best_buf)

    final_size = os.path.getsize(out_path)
    print(
        f"Done.  Chosen quality={best_quality}, "
        f"final size={final_size} bytes ➜ {out_path}"
    )


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Re-compress a PNG to JPEG ≤ target size."
    )
    parser.add_argument("input", help="Path to the source PNG")
    parser.add_argument(
        "-o",
        "--output",
        help="Output file (default: same name, .jpg extension)",
    )
    parser.add_argument(
        "-t",
        "--target",
        type=int,
        default=20480,
        help="Target size in bytes (default 20480)",
    )
    args = parser.parse_args()
    output_path = args.output or os.path.splitext(args.input)[0] + ".jpg"
    jpeg_within_size(args.input, output_path, target_bytes=args.target)


if __name__ == "__main__":
    main()
