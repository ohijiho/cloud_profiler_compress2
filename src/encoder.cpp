//
// Created by jiho on 20. 8. 13..
//

/*
 * A minimal utility program for the deltacoding
 *
 * -e, --encode: encode (default)
 * -d, --decode: decode
 *
 * --(no-)strict: (not) abort on exception
 * --block-size=<n>: n log elements per block for encoding (default: 1024)
 *
 * delta-encoding:
 * Converts the sequence of logTS elements into a sequence of chunks containing the delta-encoded values.
 *
 * bit-packing:
 * Converts the sequence of unsigned 64-bit values into a sequence of bytes.
 */

#include <time_tsc.h>
#include <comp.h>
#include <thcomp.h>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <vector>
#include <memory>
#include <functional>
#include <log_element.h>

class encoder : public thcomp_handler {
public:
    encoder(const std::function<comp_encoder *()> &init, std::ostream *p_out)
            : f(init()), out(p_out) {}
    ~encoder() override {
        delete f;
    }
    void compress(std::vector<char> &&x, std::vector<char> &y) override {
        y.resize(f->dst_size_upper_bound(x.size()));
        auto *chunk = reinterpret_cast<comp_chunk *>(y.data());
        int err = f->encode(x.data(), x.size(), chunk, y.size());
        if (err != COMP_STATUS_SUCCESS) {
            throw std::runtime_error("encode error: " + std::to_string(err));
        }
        y.resize(chunk->header.chunk_size);
    }
    void write(std::vector<char> &&x) override {
        out->write(x.data(), x.size());
    }
private:
    comp_encoder *f;
    std::ostream *out;
};

class decoder : public thcomp_handler {
public:
    explicit decoder(std::ostream *p_out)
            : out(p_out) {}
    void compress(std::vector<char> &&x, std::vector<char> &y) override {
        auto *chunk = reinterpret_cast<comp_chunk *>(x.data());
        y.resize(chunk->header.raw_size);
        int err = f.decode(chunk, y.data());
        if (err != COMP_STATUS_SUCCESS) {
            throw std::runtime_error("decode error: " + std::to_string(err));
        }
    }
    void write(std::vector<char> &&x) override {
        out->write(x.data(), x.size());
    }
private:
    comp_decoder_universal f;
    std::ostream *out;
};

static void encode_stream(std::istream &in, encoder &enc, size_t block_size, bool strict);
static void decode_stream(std::istream &in, decoder &dec, bool strict);

static bool prefix_skip(const char *p, const char *&s) {
    size_t n = strlen(p);
    if (strncmp(p, s, n) == 0) {
        s += n;
        return true;
    }
    return false;
}

int main(int argc, char **argv) {
    bool decode = false, help = false, strict = true, null = false;
    size_t block_size = 4096;
    const char *method = "df";
    int deflate_level = 6;
    for (int i = 1; i < argc; i++) {
        const char *a = argv[i];
        if (a[0] == '-') {
            if (a[1] == '-') {
                a += 2;
                if (strcmp("encode", a) == 0) {
                    decode = false;
                } else if (strcmp("decode", a) == 0) {
                    decode = true;
                } else if (strcmp("strict", a) == 0) {
                    strict = true;
                } else if (strcmp("no-strict", a) == 0) {
                    strict = false;
                } else if (prefix_skip("block-size=", a)) {
                    block_size = strtol(a, nullptr, 0);
                    //TODO: exception handling
                } else if (prefix_skip("deflate-level=", a)) {
                    deflate_level = (int)strtol(a, nullptr, 0);
                    //TODO: exception handling
                } else if (prefix_skip("method=", a)) {
                    method = a;
                } else if (strcmp("null", a) == 0) {
                    null = true;
                } else if (strcmp("help", a) == 0) {
                    help = true;
                } else {
                    std::cerr << "invalid arguments" << std::endl;
                    return 1;
                }
            } else {
                a += 1;
                for (; *a; a++) {
                    switch (*a) {
                    case 'e':
                        decode = false;
                        break;
                    case 'd':
                        decode = true;
                        break;
                    case 'h':
                        help = true;
                        break;
                    default:
                        std::cerr << "invalid arguments" << std::endl;
                        return 1;
                    }
                }
            }
        }
    }
    if (help) {
        //TODO: help
        printf("Usage: %s [OPTIONS]...\n", argv[0]);
        printf("Encode or decode standard input and print to standard output\n");
        printf("\n");
        printf("  -e, --encode           encode\n");
        printf("  -d, --decode           decode\n");
        printf("      --block-size=SIZE  encode each block with SIZE log elements\n");
        return 0;
    }
    if (decode) {
        std::istream *in = &std::cin;
        std::ostream *out = &std::cout;
        std::ofstream fout;
        if (null) {
            fout.open("/dev/null");
            out = &fout;
        }
        decoder dec(out);
        decode_stream(*in, dec, strict);
        dec.close();
    } else {
        std::istream *in = &std::cin;
        std::ostream *out = &std::cout;
        std::ofstream fout;
        if (null) {
            fout.open("/dev/null");
            out = &fout;
        }
        std::function<comp_encoder*()> init_f;
        if (strcmp("i", method) == 0) {
            init_f = []() {
                return new comp_encoder_id();
            };
        } else if (strcmp("d", method) == 0) {
            init_f = []() {
                return new comp_encoder_delta();
            };
        } else if (strcmp("b", method) == 0) {
            init_f = []() {
                return new comp_encoder_delta_bitpack();
            };
        } else if (strcmp("f", method) == 0) {
            init_f = [deflate_level]() {
                return new comp_encoder_deflate(deflate_level);
            };
        } else if (strcmp("df", method) == 0) {
            init_f = [deflate_level]() {
                return new comp_encoder_delta_deflate(deflate_level);
            };
        } else if (strcmp("bf", method) == 0) {
            init_f = [deflate_level]() {
                return new comp_encoder_delta_bitpack_deflate(deflate_level);
            };
        } else {
            std::cerr << "Unknwon method: " << method << std::endl;
            return 1;
        }
        encoder enc(init_f, out);
        encode_stream(*in, enc, block_size, strict);
        enc.close();
    }
    return 0;
}

static void decode_stream(std::istream &in, decoder &dec,
        bool strict) {
    std::vector<char> tmp;
    for (;;) {
        tmp.clear();
        tmp.resize(sizeof(comp_chunk::comp_chunk_header));
        in.read(tmp.data(), tmp.size());
        if (in.gcount() == 0) break;
        if (!in.good()) {
            if (strict)
                throw std::runtime_error("additional bytes detected");
            break;
        }
        auto *chk = (comp_chunk*)tmp.data();
        size_t chunk_size = chk->header.chunk_size;
        tmp.resize(chunk_size);
        chk = (comp_chunk *) tmp.data();
        in.read(chk->data, chunk_size - sizeof(chk->header));
        if (!in.good()) {
            if (strict)
                throw std::runtime_error("chunk size > remaining bytes");
            break;
        }
        dec.input(std::move(tmp));
    }
}

static void encode_stream(std::istream &in, encoder &enc,
        size_t block_size, bool strict) {
    std::vector<char> tmp;
    while (!in.eof()) {
        tmp.clear();
        tmp.resize(block_size * sizeof(raw_log_element));
        in.read(tmp.data(), tmp.size());
        size_t read_size = in.gcount();
        if (read_size == 0) break;
        tmp.resize(read_size);
        enc.input(std::move(tmp));
    }
}
