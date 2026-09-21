>> FILE COMPRESSOR USING HUFFMAN CODING

Overview:

This project implements a universal file compression and decompression tool in C++, based on Huffman Encoding. It allows users to compress and decompress any file format (e.g., `.txt`, `.pdf`, `.jpg`, `.mp4`, etc.) while preserving original file content.

Features:

- Compress any type of file using Huffman coding
- Decompress and restore the original file
- Binary-safe (works on both text and binary files)
- Simple terminal interface
- Efficient storage using frequency-based encoding
- Custom compressed file format with dictionary and padding information



How It Works:

> Compression Process
1. Read the input file byte-by-byte.
2. Calculate frequency of each byte.
3. Build a Huffman Tree and generate unique binary codes.
4. Write a custom header with:
   - Huffman code dictionary
   - Padding info
5. Write encoded binary data to the output compressed file.

> Decompression Process
1. Read the header and recreate the Huffman code mapping.
2. Read encoded bits and decode using the prefix tree.
3. Write original data to the decompressed file.


Requirements:

- C++11 or later
- G++ compiler or any modern C++ compiler

How to  compile:

"g++ file_compressor.cpp -o file_compressor"
"./file_compressor"
