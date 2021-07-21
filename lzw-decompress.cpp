
#include <iostream>
#include <cstring>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <queue>
using namespace std;

static bool lzw_decompress(FILE *fin, FILE *fout);

struct code_reader {
    FILE *fin;
    queue<bool> buffer;

    code_reader(FILE *_fin) {
        fin = _fin;
    }

    int read(int bit_count) {
        while (buffer.size() < bit_count && !feof(fin)) {
            int c = fgetc(fin);
            for (int i = 0; i < 8; i++) {
                buffer.push(c & (1 << i));
            }
        }
        int result = 0;
        for (int i = 0; i < bit_count && !buffer.empty(); i++) {
            if (buffer.front()) {
                result += (1 << i);
            }
            buffer.pop();
        }
        return result;
    }

    bool done() {
        return feof(fin) && buffer.empty();
    }
};

int main() {
    lzw_decompress(stdin, stdout);
}

static bool lzw_decompress(FILE *fin, FILE *fout) {
    string code_map[65536];

    /* read & check magic */
    int magic_1 = fgetc(fin);
    int magic_2 = fgetc(fin);
    if (magic_1 != 0x1f && magic_2 != 0x9d) {
        cerr << "Invalid Header Flags Byte: Incorrect magic bytes" << endl;
        return false;
    }

    /* read flags & set max bits */
    int flags = fgetc(fin);
    if (flags & 0x60) {
        cerr << "Invalid Header Flags Byte: Flag byte contains invalid data" << endl;
        return false;
    }

    int max_bits = flags & 0x1f;
    if (max_bits < 9 || max_bits > 16) {
        cerr << "invalid head flags byte: flag byte contains invalid data "
             << "(max_bits=" << max_bits << ")" << endl;
    }

    bool block_mode = (flags & 0x80);

    int bits = 9;
    int end = (block_mode ? 256 : 255);

    for (int i = 0; i < 256; i++) {
        code_map[i] += (char) i;
    }

    code_reader reader(fin);

    int old_code = reader.read(bits);
    char character = old_code;

    // output first character
    printf("%s", code_map[old_code].c_str());

    int next = end + 1;

    for (int i = 0; i < 500; i++) {
        if (next >= (1 << bits)) {
            bits ++;
        }

        int new_code = reader.read(bits);

        bool in_translation_table = (code_map[new_code].length() != 0);
        string s;
        if (!in_translation_table) {
            s = code_map[old_code];
            s += character;
        } else {
            s = code_map[new_code];
        }

        printf("%s", s.c_str());
        
        character = s[0];
        code_map[next++] = code_map[old_code] + character;
        old_code = new_code;
    }

    return true;
}

