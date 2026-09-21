#include <iostream>
#include <fstream>
#include <queue>
#include <unordered_map>
#include <vector>
#include <bitset>
#include <cstdint>
#include <filesystem> // C++17

using namespace std;
namespace fs = std::filesystem;

struct Node {
    unsigned char ch;
    int freq;
    Node *left, *right;

    Node(unsigned char c, int f) {
        ch = c;
        freq = f;
        left = right = nullptr;
    }
};

struct Compare {
    bool operator()(Node* a, Node* b) {
        return a->freq > b->freq;
    }
};

void buildCode(Node* root, string str, unordered_map<unsigned char, string> &huffmanCode) {
    if (!root) return;
    if (!root->left && !root->right) huffmanCode[root->ch] = str;
    buildCode(root->left, str + "0", huffmanCode);
    buildCode(root->right, str + "1", huffmanCode);
}

void encode(string inputPath, string outputPath) {
    ifstream in(inputPath, ios::binary);
    if (!in.is_open()) {
        cout << "File not found!\n";
        return;
    }

    unordered_map<unsigned char, int> freq;
    vector<unsigned char> inputBytes;

    unsigned char ch;
    while (in.read(reinterpret_cast<char*>(&ch), 1)) {
        freq[ch]++;
        inputBytes.push_back(ch);
    }
    in.close();

    priority_queue<Node*, vector<Node*>, Compare> pq;
    for (auto p : freq)
        pq.push(new Node(p.first, p.second));

    while (pq.size() > 1) {
        Node *left = pq.top(); pq.pop();
        Node *right = pq.top(); pq.pop();
        Node *merged = new Node('\0', left->freq + right->freq);
        merged->left = left;
        merged->right = right;
        pq.push(merged);
    }

    Node* root = pq.top();
    unordered_map<unsigned char, string> huffmanCode;
    buildCode(root, "", huffmanCode);

    // Encode data
    string encoded = "";
    for (unsigned char byte : inputBytes)
        encoded += huffmanCode[byte];

    // Bit padding
    uint8_t padding = static_cast<uint8_t>((8 - (encoded.size() % 8)) % 8);
    encoded.append(padding, '0');

    // Convert to bytes
    vector<unsigned char> encodedBytes;
    for (size_t i = 0; i < encoded.size(); i += 8) {
        string byteStr = encoded.substr(i, 8);
        bitset<8> bits(byteStr);
        encodedBytes.push_back(static_cast<unsigned char>(bits.to_ulong()));
    }

    // Store dictionary
    vector<unsigned char> header;
    uint32_t dictSize = huffmanCode.size();
    header.insert(header.end(), reinterpret_cast<unsigned char*>(&dictSize), reinterpret_cast<unsigned char*>(&dictSize) + sizeof(dictSize));

    for (auto& pair : huffmanCode) {
        header.push_back(pair.first);
        uint8_t len = static_cast<uint8_t>(pair.second.size());
        header.push_back(len);
        header.insert(header.end(), pair.second.begin(), pair.second.end());
    }

    header.push_back(padding);

    size_t totalCompressedSize = header.size() + encodedBytes.size();
    size_t originalSize = inputBytes.size();

    if (totalCompressedSize >= originalSize) {
        cout << "Compression not effective. Copying original file.\n";
        fs::copy_file(inputPath, outputPath, fs::copy_options::overwrite_existing);
        return;
    }

    ofstream out(outputPath, ios::binary);
    out.write(reinterpret_cast<char*>(header.data()), header.size());
    out.write(reinterpret_cast<char*>(encodedBytes.data()), encodedBytes.size());
    out.close();

    cout << "Compression completed.\n";
    cout << "Original Size: " << originalSize << " bytes\n";
    cout << "Compressed Size: " << totalCompressedSize << " bytes\n";
    double ratio = (1.0 - ((double)totalCompressedSize / originalSize)) * 100.0;
    cout << "Compression Ratio: " << ratio << "%\n";
}

void decode(string inputPath, string outputPath) {
    ifstream in(inputPath, ios::binary);
    if (!in.is_open()) {
        cout << "File not found!\n";
        return;
    }

    uint32_t dictSize;
    in.read(reinterpret_cast<char*>(&dictSize), sizeof(dictSize));

    unordered_map<string, unsigned char> codeToChar;
    for (uint32_t i = 0; i < dictSize; i++) {
        unsigned char ch;
        uint8_t len;
        in.read(reinterpret_cast<char*>(&ch), 1);
        in.read(reinterpret_cast<char*>(&len), 1);
        string code(len, ' ');
        in.read(&code[0], len);
        codeToChar[code] = ch;
    }

    uint8_t padding;
    in.read(reinterpret_cast<char*>(&padding), 1);

    string bits = "";
    unsigned char byte;
    while (in.read(reinterpret_cast<char*>(&byte), 1)) {
        bitset<8> b(byte);
        bits += b.to_string();
    }

    if (padding > 0)
        bits = bits.substr(0, bits.size() - padding);

    ofstream out(outputPath, ios::binary);
    string curr = "";
    for (char bit : bits) {
        curr += bit;
        if (codeToChar.count(curr)) {
            out.write(reinterpret_cast<char*>(&codeToChar[curr]), 1);
            curr.clear();
        }
    }

    in.close();
    out.close();
    cout << "Decompression completed.\n";
}

int main() {
    int choice;
    string inputFile, outputFile;

    cout << "\nFile Compression Tool using Huffman Coding\n";
    cout << "1. Compress File\n";
    cout << "2. Decompress File\n";
    cout << "Enter your choice: ";
    cin >> choice;

    if (choice == 1) {
        cout << "Enter input file name: ";
        cin >> inputFile;
        cout << "Enter output (compressed) file name: ";
        cin >> outputFile;
        encode(inputFile, outputFile);
    } else if (choice == 2) {
        cout << "Enter compressed file name: ";
        cin >> inputFile;
        cout << "Enter output (decompressed) file name: ";
        cin >> outputFile;
        decode(inputFile, outputFile);
    } else {
        cout << "Invalid choice.\n";
    }

    return 0;
}
