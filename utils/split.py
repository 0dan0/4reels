#!/usr/bin/env python3

import sys
import os

def split_to_pgm(dump_file, width, height, out_prefix, offset):
    """
    Splits `dump_file` into consecutive PGM images of size width x height (8-bit grayscale).
    Each output file is named newname0001.pgm, newname0002.pgm, etc.
    """
    frame_size = width * height  # bytes per image frame
    
    with open(dump_file, 'rb') as f:
        frame_num = 1
        
        data = f.read(offset)
        
        while True:
            data = f.read(frame_size)
            
            # Build filename like newname0001.pgm, newname0002.pgm, ...
            out_filename = f"{out_prefix}{frame_num:04d}.pgm"
            
            new_height = int(len(data) / width)
            
            with open(out_filename, 'wb') as out_pgm:
                # Write minimal PGM header (binary, "P5")
                out_pgm.write(b'P5\n')
                out_pgm.write(f"{width} {new_height}\n".encode('ascii'))
                out_pgm.write(b"255\n")  # Max pixel value for 8-bit
                out_pgm.write(data)      # Write pixel data
            
            print(f"Wrote {out_filename}")
            frame_num += 1
            
            if len(data) < frame_size:
                # Reached EOF or partial data (stop)
                break

def main():
    if len(sys.argv) < 5:
        print("Usage: python split_pgms.py <dump_file> <width> <height> <out_prefix> <offset>")
        sys.exit(1)
    
    offset = 0
    dump_file = sys.argv[1]
    width = int(sys.argv[2])
    height = int(sys.argv[3])
    out_prefix = sys.argv[4]
    if len(sys.argv) > 5:
        offset = int(sys.argv[5])
    
    file_size = os.path.getsize(dump_file)
    frame_size = width * height
    total_frames = file_size // frame_size
    
    print(f"File: {dump_file}")
    print(f"Size: {file_size} bytes")
    print(f"Frame: {width}x{height} => {frame_size} bytes per frame")
    print(f"Total full frames in dump: {total_frames}")
    
    split_to_pgm(dump_file, width, height, out_prefix, offset)

if __name__ == "__main__":
    main()
