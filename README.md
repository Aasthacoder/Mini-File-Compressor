# Mini File Compressor

A small-to-medium scale **lossless file compression and decompression utility** implemented in **C++17** using **Huffman Coding**.

The project was built from scratch to understand the complete compression pipeline — from byte-level frequency analysis and Huffman tree construction to bit-level encoding, custom binary file formats, and decompression.

> **Project Focus:** C++ • Data Structures & Algorithms • Huffman Coding • Bit Manipulation • Binary File I/O • Memory Management • Testing

---

## 📌 Overview

Most files contain symbols that occur with different frequencies.

For example, in a text file, some characters such as spaces or common letters may appear much more frequently than others.

Instead of representing every byte using a fixed 8-bit representation, this project uses **Huffman Coding** to assign:

- Shorter codes → frequently occurring bytes
- Longer codes → less frequently occurring bytes

The resulting variable-length codes are packed into actual bytes and stored inside a custom `.huf` binary format.

The decompressor reads the metadata, reconstructs the same Huffman tree, reads the compressed bitstream, and restores the original bytes.

The system is **lossless**, meaning:

```text
decompress(compress(file)) == original file
````

---

# 🚀 Features

* Lossless file compression using **Huffman Coding**
* File decompression with exact byte-for-byte reconstruction
* Frequency analysis of input bytes
* Huffman tree construction using a **min-priority queue**
* Variable-length prefix-free code generation
* Custom **BitWriter** for bit-level output
* Custom **BitReader** for bit-level input
* Custom `.huf` binary file format
* Deterministic Huffman tree reconstruction
* Binary file processing using raw bytes
* Empty-file handling
* Single-symbol input handling
* Arbitrary byte-value support (`0–255`)
* Malformed archive validation
* Explicit Huffman tree memory management
* Exception-based error handling
* Automated correctness testing using **GoogleTest**
* Compression statistics using **CompressionStats**

---

# 🏗️ Architecture

## Compression Pipeline

```text
                    INPUT FILE
                        │
                        ▼
                ┌───────────────┐
                │  FileReader   │
                └───────┬───────┘
                        │
                        ▼
              ┌───────────────────┐
              │ Frequency Counter  │
              └─────────┬─────────┘
                        │
                        ▼
              ┌───────────────────┐
              │  Min Priority     │
              │     Queue         │
              └─────────┬─────────┘
                        │
                        ▼
              ┌───────────────────┐
              │   Huffman Tree    │
              └─────────┬─────────┘
                        │
                        ▼
              ┌───────────────────┐
              │  Huffman Codes    │
              └─────────┬─────────┘
                        │
                        ▼
              ┌───────────────────┐
              │    BitWriter      │
              └─────────┬─────────┘
                        │
                        ▼
                  .HUF FILE
```

---

## Decompression Pipeline

```text
                     .HUF FILE
                         │
                         ▼
                ┌─────────────────┐
                │ Header / Metadata│
                └────────┬────────┘
                         │
                         ▼
                ┌─────────────────┐
                │ Frequency Table │
                └────────┬────────┘
                         │
                         ▼
                ┌─────────────────┐
                │ Rebuild Huffman │
                │      Tree       │
                └────────┬────────┘
                         │
                         ▼
                ┌─────────────────┐
                │    BitReader    │
                └────────┬────────┘
                         │
                         ▼
                ┌─────────────────┐
                │ Huffman Tree    │
                │   Traversal     │
                └────────┬────────┘
                         │
                         ▼
                    OUTPUT FILE
```

---

# 🧠 How Huffman Coding Works

Suppose the input is:

```text
BANANA
```

The frequency table becomes:

```text
A → 3
N → 2
B → 1
```

The two least-frequent nodes are repeatedly combined using a min-priority queue.

A possible tree is:

```text
          (6)
         /   \
       A(3)   (3)
             /   \
           B(1)  N(2)
```

Assigning:

```text
Left  → 0
Right → 1
```

produces codes such as:

```text
A → 0
B → 10
N → 11
```

The original input:

```text
BANANA
```

is therefore encoded as:

```text
10 0 11 0 11 0
```

or:

```text
100110110
```

These bits are then packed into actual bytes by `BitWriter`.

---

# 🌳 Huffman Tree Construction

The tree is constructed using a **min-priority queue**.

For `k` unique symbols:

1. Create a leaf node for every symbol.
2. Insert all nodes into a min-priority queue.
3. Remove the two nodes with the smallest frequencies.
4. Create a new parent node whose frequency is their sum.
5. Attach the two removed nodes as children.
6. Insert the new node back into the queue.
7. Repeat until only one node remains.

The final node becomes the root of the Huffman tree.

### Complexity

```text
O(k log k)
```

where:

```text
k = number of unique symbols
```

Since the implementation operates on bytes:

```text
k ≤ 256
```

---

# 🔢 Frequency Analysis

The input is processed as raw bytes rather than assuming text.

Each byte is mapped to its frequency.

Conceptually:

```cpp
frequency[byte]++;
```

A hash-based structure is used for efficient frequency lookup.

Frequency counting is approximately:

```text
O(n)
```

where `n` is the number of input bytes.

---

# 💡 Prefix-Free Codes

Huffman codes are **prefix-free**.

For example:

```text
A → 0
B → 10
C → 11
```

No complete code is the prefix of another code.

This allows the decoder to determine exactly where one symbol ends without requiring separators between codes.

The property follows from storing symbols only at leaf nodes of the Huffman tree.

---

# 🧩 Bit-Level Encoding

Huffman codes are sequences of individual bits.

Writing:

```text
101101
```

as characters would be inefficient because each character `'0'` or `'1'` occupies an entire byte.

Instead, `BitWriter` maintains:

```text
currentByte
bitCount
```

and packs incoming bits into an 8-bit byte.

For example:

```text
Logical bits:

10110101

        ↓

Physical byte:

10110101
```

Once eight bits have been accumulated, the byte is written to the output stream.

This provides actual bit-level storage rather than storing textual representations of bits.

---

# 📖 Bit-Level Decoding

`BitReader` performs the reverse operation.

It reads a byte and exposes its individual bits to the decompressor.

During decoding:

```text
0 → move to left child
1 → move to right child
```

When a leaf node is reached:

```text
leaf → decoded byte
```

The decoder then returns to the root and continues processing the next bits.

---

# 📦 Custom `.huf` File Format

The compressed file uses a custom binary format.

Conceptually:

```text
┌─────────────────────────┐
│ Magic Number            │
├─────────────────────────┤
│ Unique Symbol Count     │
├─────────────────────────┤
│ Original File Size      │
├─────────────────────────┤
│ Frequency Table         │
├─────────────────────────┤
│ Compressed Bitstream    │
└─────────────────────────┘
```

## Magic Number

The archive contains a format identifier such as:

```text
HUF1
```

This allows the decompressor to reject files that do not follow the expected format.

## Frequency Table

The frequency table allows the decompressor to reconstruct the Huffman tree without requiring the original file.

## Original File Size

The final compressed byte may contain padding bits because Huffman codes are not necessarily aligned to byte boundaries.

The original file size provides an explicit stopping condition for decompression.

---

# 🔁 Deterministic Tree Reconstruction

The decompressor reconstructs the Huffman tree from the frequency table stored in the `.huf` file.

Equal frequencies can potentially produce multiple valid Huffman trees.

Therefore, the implementation uses deterministic ordering and tie-breaking so that:

```text
Same frequency table
        +
Same construction rules
        ↓
Same Huffman tree
```

This ensures that the compressor and decompressor agree on the generated codes.

---

# 🧪 Testing

The project uses **GoogleTest** for automated correctness testing.

The main invariant being tested is:

```text
Original File
     │
     ▼
Compress
     │
     ▼
.huf
     │
     ▼
Decompress
     │
     ▼
Restored File

Original == Restored
```

## Test Coverage

The test suite covers cases including:

* Normal text files
* Empty files
* Single-symbol files
* All 256 possible byte values
* Binary patterns
* Larger text inputs
* Invalid/malformed archive headers
* Round-trip compression/decompression

The tests focus on correctness of the complete compression/decompression pipeline rather than only testing individual functions.

---

# 📊 Compression Statistics

The project includes a small `CompressionStats` utility to evaluate compression effectiveness.

It reports:

```text
Original Size
Compressed Size
Compression Ratio
Space Saved
```

### Compression Ratio

```text
compressed_size / original_size
```

### Space Saved

```text
(1 - compressed_size / original_size) × 100
```

For example:

```text
Original Size   : 1,000,000 bytes
Compressed Size :   400,000 bytes

Compression Ratio : 0.40
Space Saved       : 60%
```

Compression effectiveness depends on the input data distribution.

Highly repetitive or statistically skewed data generally provides more opportunity for Huffman compression, while small or already-compressed data may provide little benefit because of archive metadata overhead.

---

# 🧪 Example Usage

## Compress

```bash
compressor compress input.txt output.huf
```

## Decompress

```bash
compressor decompress output.huf restored.txt
```

## Check Compression Statistics

```bash
compression_stats input.txt
```

Example output:

```text
=====================================
       COMPRESSION STATISTICS
=====================================
Original size    : 1000000 bytes
Compressed size  : 420000 bytes
Compression ratio: 0.42
Space saved      : 58.00%
=====================================
```

---

# 🛠️ Build

The project uses **CMake** and requires a C++17-compatible compiler.

## Requirements

* C++17 compatible compiler
* CMake 3.15+
* Git
* GoogleTest (automatically fetched by CMake)

---

## Windows

Configure:

```powershell
cmake -S . -B build
```

Build:

```powershell
cmake --build build --config Release
```

Executables will be generated under:

```text
build/Release/
```

---

# 🧪 Running Tests

Run the GoogleTest executable:

```powershell
.\build\Release\compressor_tests.exe
```

Or use CTest:

```powershell
ctest --test-dir build -C Release --output-on-failure
```

---

# 📁 Project Structure

```text
MiniFileCompressor/
│
├── CMakeLists.txt
│
├── include/
│   ├── FileReader.h
│   ├── FrequencyCounter.h
│   ├── HuffmanNode.h
│   ├── HuffmanTree.h
│   ├── BitWriter.h
│   ├── BitReader.h
│   ├── Compressor.h
│   └── Decompressor.h
│
├── src/
│   ├── main.cpp
│   ├── FileReader.cpp
│   ├── FrequencyCounter.cpp
│   ├── HuffmanTree.cpp
│   ├── BitWriter.cpp
│   ├── BitReader.cpp
│   ├── Compressor.cpp
│   └── Decompressor.cpp
│
├── tests/
│   └── CompressorTests.cpp
│
└── tools/
    └── CompressionStats.cpp
```

---

# 🔧 Design Decisions

## Why Huffman Coding?

Huffman Coding provides a compact and understandable compression algorithm while demonstrating several important concepts:

* Greedy algorithms
* Binary trees
* Priority queues
* Variable-length encoding
* Bit manipulation
* Binary file processing

It was therefore a good fit for a focused C++ systems-oriented project.

---

## Why a Priority Queue?

Huffman construction repeatedly requires the two lowest-frequency nodes.

A min-priority queue provides efficient access to those nodes.

```text
Remove minimum
      ↓
Remove minimum
      ↓
Merge
      ↓
Insert merged node
```

---

## Why Binary File I/O?

The compressor works with raw bytes and packed bits.

Binary mode prevents text-specific transformations and allows the implementation to process arbitrary byte values.

---

## Why a Custom File Format?

The compressed bitstream alone is insufficient for decompression.

The decoder needs metadata to reconstruct the Huffman tree and determine when decoding is complete.

The `.huf` format provides this metadata together with the compressed stream.

---

# ⚠️ Limitations

This project intentionally focuses on learning and understanding the complete compression pipeline rather than competing with production compression tools.

Current limitations include:

* The compressor currently loads the input data into memory.
* The `.huf` format is a project-specific format rather than a standardized archive format.
* Huffman coding alone does not exploit repeated multi-byte patterns like dictionary-based algorithms.
* Small files may become larger because of metadata/header overhead.
* Already-compressed data may provide little additional compression.
* No CRC or cryptographic checksum is currently included for strong integrity verification.
* The implementation is primarily intended as a learning-oriented C++ project.

---

# 🚀 Possible Future Improvements

Potential extensions include:

* Two-pass streaming compression to reduce memory usage
* `std::unique_ptr`-based Huffman tree ownership
* CRC/checksum support
* Improved archive integrity validation
* Additional compression algorithms
* Hybrid compression using dictionary coding + Huffman coding
* More extensive binary-file testing
* Archive metadata versioning

---

# 📚 Concepts Demonstrated

### Algorithms

* Huffman Coding
* Greedy Algorithms
* Depth-First Search
* Tree Traversal
* Prefix Coding

### Data Structures

* Binary Trees
* Min Heaps
* Priority Queues
* Hash Tables
* Vectors

### C++

* C++17
* Classes
* Object-Oriented Programming
* Pointers
* Dynamic Memory Management
* Destructors
* STL
* Exception Handling

### Systems / Low-Level Programming

* Binary File I/O
* Raw Byte Processing
* Bit Manipulation
* Bitwise Operators
* Bit Shifting
* Byte Packing
* Custom Binary File Formats

### Software Engineering

* CMake
* Modular Architecture
* Automated Testing
* GoogleTest
* Input Validation
* Error Handling

---

# 🎯 Learning Outcomes

This project provided hands-on experience with:

1. Designing a complete compression/decompression pipeline.
2. Applying a greedy algorithm to a practical problem.
3. Using priority queues and binary trees to construct Huffman codes.
4. Working directly with bits and bytes.
5. Designing and parsing a custom binary file format.
6. Managing dynamically allocated tree structures.
7. Handling malformed input and edge cases.
8. Writing automated tests for end-to-end correctness.
9. Using CMake to manage a multi-component C++ project.
10. Measuring compression effectiveness using compression ratio and space savings.

# 👨‍💻 Project Focus

This project was designed as a compact **C++ + DSA + low-level systems** project rather than a production replacement for formats such as ZIP.

The primary objective was to understand and implement the complete path from:

```text
Raw Bytes
    ↓
Frequency Analysis
    ↓
Huffman Tree
    ↓
Variable-Length Codes
    ↓
Bit Packing
    ↓
Custom Binary Archive
    ↓
Bit-Level Decoding
    ↓
Original Bytes
```

---

## License

This project is intended for educational and portfolio purposes.
