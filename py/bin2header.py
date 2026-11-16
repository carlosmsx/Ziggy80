import sys
import os

def bin_to_c_array(bin_file, header_file, array_name):
    with open(bin_file, 'rb') as f:
        data = f.read()

    with open(header_file, 'w') as f:
        f.write(f"// Generated from {os.path.basename(bin_file)}\n")
        f.write(f"const unsigned char {array_name}[] = {{\n")
        for i, byte in enumerate(data):
            if i % 12 == 0:
                f.write("    ")
            f.write(f"0x{byte:02x}, ")
            if (i + 1) % 12 == 0:
                f.write("\n")
        f.write("\n};\n")
        f.write(f"const unsigned int {array_name}_len = {len(data)};\n")

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: bin2header.py <input.bin> <output.h> <array_name>")
        sys.exit(1)

    bin_to_c_array(sys.argv[1], sys.argv[2], sys.argv[3])
