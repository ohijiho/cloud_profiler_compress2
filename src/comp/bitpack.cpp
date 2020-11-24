//
// Created by jiho on 20. 9. 20..
//

#include <log_element.h>
#include "impl_common.h"
#include <cstring>

static int bit_pack(const uint64_t *src, size_t number_of_values,
        char *dst, size_t *dst_size) {
    char *dst_ptr = dst;
    for (; number_of_values--; src++) {
        /* bit packing */
        uint64_t x = *src;
        int max_length = 9;
        while ((x || max_length == 9) && max_length--) {
            auto y = (unsigned char)(x & 0xFFU);
            x >>= 7U;
            if (x)
                y |= 0x80U;
            if (dst_ptr - dst >= (ptrdiff_t)*dst_size)
                return COMP_STATUS_OVERFLOW;
            *dst_ptr++ = (char)y;
        }
    }
    *dst_size = dst_ptr - dst;
    return COMP_STATUS_SUCCESS;
}
static int bit_unpack(const char *src, size_t size,
        uint64_t *dst, size_t *number_of_values) {
    uint64_t *dst_ptr = dst;
    const char *src_ptr = src;

    uint64_t y = 0;
    uint64_t shift = 0;
    for (; size--; src_ptr++) {
        auto x = (unsigned char)*src_ptr;
        y |= (uint64_t)(x & 0x7FU) << shift;
        shift += 7;
        if (shift == 63) {
            /*
             * 9-th byte
             * consume full byte and pretend to be the last byte
             */
            y |= (uint64_t)x << (63U - 7U);
            x = 0;
        }
        if ((x & 0x80U) == 0) {
            if (dst_ptr - dst >= (ptrdiff_t)*number_of_values)
                return COMP_STATUS_ILLEGAL_FORMAT;
            *dst_ptr++ = y;
            y = 0;
            shift = 0;
        }
    }
    if (shift != 0) return COMP_STATUS_ILLEGAL_FORMAT;
    /*
     * shift is increased by 7 whenever a byte is read
     * shift is reset to 0 when the bytes are converted to a 64-bit value and it is written
     * (shift / 7) is the number of bytes not written to dst
     */
    *number_of_values = dst_ptr - dst;
    return COMP_STATUS_SUCCESS;
}

int comp_encoder_delta_bitpack::encode(const void *src, size_t src_size,
        comp_chunk *dst, size_t dst_size) {
    encoder_do_sanity_check;
    size_t size = src_size / sizeof(raw_log_element);
    dst_size = size * sizeof(log_element);
    tmp.resize(comp_encoder_delta::dst_size_upper_bound(src_size));
    auto *tmp_chunk = (comp_chunk*)tmp.data();
    f.encode(src, src_size, tmp_chunk, tmp.size());
    int ret = bit_pack((const uint64_t*)tmp_chunk->data,
            size * (sizeof(log_element) / sizeof(uint64_t)),
            dst->data, &dst_size);
    if (ret == COMP_STATUS_SUCCESS) {
        init_header(comp_decoder_delta_bitpack::CHUNK_TYPE);
    } else if (ret == COMP_STATUS_OVERFLOW) {
        memcpy(dst, tmp_chunk, tmp_chunk->header.chunk_size);
    }
    return ret;
}

int comp_decoder_delta_bitpack::decode(const comp_chunk *src, void *dst) {
    decoder_do_sanity_check;
    size_t size = src->header.raw_size / sizeof(raw_log_element);
    size_t tmp_size = size * sizeof(log_element);
    tmp.resize(COMP_CHUNK_SIZE(tmp_size));
    auto *tmp_chunk = (comp_chunk*)tmp.data();
    tmp_chunk->header = {
            .type = comp_decoder_delta::CHUNK_TYPE,
            .chunk_size = (uint32_t)tmp.size(),
            .raw_size = src->header.raw_size,
    };
    size_t n = size * (sizeof(log_element) / sizeof(uint64_t));
    size_t n2 = n;
    int err = bit_unpack(src->data, COMP_DATA_SIZE(src->header.chunk_size),
            (uint64_t*)tmp_chunk->data, &n);
    if (err != COMP_STATUS_SUCCESS || n != n2) return COMP_STATUS_ILLEGAL_FORMAT;
    return f.decode(tmp_chunk, dst);
}

uint32_t comp_decoder_delta_bitpack::single_type() const {
    return CHUNK_TYPE;
}

int comp_decoder_delta_bitpack::sanity_check(const comp_chunk *src, void *dst) const {
    int err = comp_decoder_single_type::sanity_check(src, dst); // NOLINT(bugprone-parent-virtual-call)
    if (err) return err;
    if (src->header.raw_size % sizeof(raw_log_element) != 0)
        return COMP_STATUS_ILLEGAL_FORMAT;
    return COMP_STATUS_SUCCESS;
}
