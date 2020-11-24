//
// Created by jiho on 20. 9. 20..
//

#ifndef COMPRESS2_COMP_H
#define COMPRESS2_COMP_H

#include <cstddef>
#include <cstdint>
#include <vector>
#include <map>

struct comp_chunk {
    struct comp_chunk_header {
        uint32_t type;
        uint32_t chunk_size;
        uint32_t raw_size;
    } header;
    char data[1];
};

#define COMP_CHUNK_SIZE(data_size) (sizeof(comp_chunk::comp_chunk_header) + (data_size))
#define COMP_DATA_SIZE(chunk_size) ((chunk_size) - sizeof(comp_chunk::comp_chunk_header))

class comp_encoder {
public:
    virtual ~comp_encoder() = default;
    virtual int encode(const void *src, size_t src_size,
            comp_chunk *dst, size_t dst_size) = 0;
    virtual int sanity_check(const void *src, size_t src_size,
            comp_chunk *dst, size_t dst_size) const;
    virtual size_t dst_size_upper_bound(size_t src_size) const;
};

class comp_decoder {
public:
    virtual ~comp_decoder() = default;
    virtual int decode(const comp_chunk *src, void *dst) = 0;
    virtual int sanity_check(const comp_chunk *src, void *dst) const;
};

#define COMP_STATUS_SUCCESS 0
#define COMP_STATUS_OVERFLOW 1
#define COMP_STATUS_UNSUPPORTED_FORMAT 2
#define COMP_STATUS_FATAL_ERROR -1
#define COMP_STATUS_ILLEGAL_FORMAT -2
#define COMP_STATUS_ZLIB_ERROR -100

class comp_decoder_single_type : public comp_decoder {
public:
    int sanity_check(const comp_chunk *src, void *dst) const override;
    virtual uint32_t single_type() const = 0;
};

class comp_encoder_composition : public comp_encoder {
public:
    comp_encoder_composition(comp_encoder *outer, comp_encoder *inner);
    int encode(const void *src, size_t src_size,
            comp_chunk *dst, size_t dst_size) override;
    int sanity_check(const void *src, size_t src_size,
            comp_chunk *dst, size_t dst_size) const override;
    size_t dst_size_upper_bound(size_t src_size) const override;
private:
    comp_encoder *f, *g;
    std::vector<char> tmp;
};

class comp_decoder_composition : public comp_decoder_single_type {
public:
    comp_decoder_composition(comp_decoder *outer, comp_decoder *inner);
    int decode(const comp_chunk *src, void *dst) override;
    int sanity_check(const comp_chunk *src, void *dst) const override;
    static const uint32_t CHUNK_TYPE = 10;
    uint32_t single_type() const override;
private:
    comp_decoder *f, *g;
    std::vector<char> tmp;
};

class comp_encoder_id : public comp_encoder {
public:
    int encode(const void *src, size_t src_size,
            comp_chunk *dst, size_t dst_size) override;
};

class comp_decoder_id : public comp_decoder_single_type {
public:
    int decode(const comp_chunk *src, void *dst) override;
    int sanity_check(const comp_chunk *src, void *dst) const override;
    static const uint32_t CHUNK_TYPE = 0;
    uint32_t single_type() const override;
};

class comp_encoder_deflate : public comp_encoder_id {
public:
    explicit comp_encoder_deflate(int level);
    int encode(const void *src, size_t src_size,
            comp_chunk *dst, size_t dst_size) override;
private:
    int m_level;
};

class comp_decoder_inflate : public comp_decoder_single_type {
public:
    int decode(const comp_chunk *src, void *dst) override;
    static const uint32_t CHUNK_TYPE = 1;
    uint32_t single_type() const override;
};

class comp_encoder_delta : public comp_encoder {
public:
    int encode(const void *src, size_t src_size,
            comp_chunk *dst, size_t dst_size) override;
    int sanity_check(const void *src, size_t src_size,
            comp_chunk *dst, size_t dst_size) const override;
    size_t dst_size_upper_bound(size_t src_size) const override;
};

class comp_decoder_delta : public comp_decoder_single_type {
public:
    int decode(const comp_chunk *src, void *dst) override;
    int sanity_check(const comp_chunk *src, void *dst) const override;
    static const uint32_t CHUNK_TYPE = 2;
    uint32_t single_type() const override;

};

class comp_encoder_delta_bitpack : public comp_encoder_delta {
public:
    int encode(const void *src, size_t src_size,
            comp_chunk *dst, size_t dst_size) override;
private:
    std::vector<char> tmp;
    comp_encoder_delta f;
};

class comp_decoder_delta_bitpack : public comp_decoder_delta {
public:
    int decode(const comp_chunk *src, void *dst) override;
    int sanity_check(const comp_chunk *src, void *dst) const override;
    static const uint32_t CHUNK_TYPE = 3;
    uint32_t single_type() const override;
private:
    std::vector<char> tmp;
    comp_decoder_delta f;
};

class comp_decoder_union : public comp_decoder {
public:
    comp_decoder_union(const std::initializer_list<comp_decoder_single_type *> &single_type_list);
    explicit comp_decoder_union(std::map<uint32_t, comp_decoder *> decoder_map);
    int decode(const comp_chunk *src, void *dst) override;
    int sanity_check(const comp_chunk *src, void *dst) const override;
//    void add_decoder(uint32_t type, comp_decoder *decoder);
//    void add_decoder(comp_decoder_single_type *decoder);
protected:
    std::map<uint32_t, comp_decoder *> fmap;
};

class comp_encoder_delta_deflate : public comp_encoder_composition {
public:
    explicit comp_encoder_delta_deflate(int level);
private:
    comp_encoder_deflate f_buf;
    comp_encoder_delta g_buf;
};

class comp_encoder_delta_bitpack_deflate : public comp_encoder_composition {
public:
    explicit comp_encoder_delta_bitpack_deflate(int level);
private:
    comp_encoder_deflate f_buf;
    comp_encoder_delta_bitpack g_buf;
};

class comp_decoder_universal : public comp_decoder_union {
public:
    comp_decoder_universal();
    ~comp_decoder_universal() override;
};

//int comp_deflate(const void *src, size_t src_size,
//        void *dst, size_t *dst_size, int level);
//int comp_inflate(const void *src, size_t src_size,
//        void *dst, size_t *dst_size);

#endif //COMPRESS2_COMP_H
