//
// Created by jiho on 20. 9. 20..
//

#include <log_element.h>
#include <comp.h>
#include "impl_common.h"

static inline log_element operator+(const log_element &a, const log_element &b) {
    return {
            .tuple = a.tuple + b.tuple,
            .nsec = a.nsec + b.nsec,
            .tsc = a.tsc + b.tsc,
    };
}
static inline log_element operator-(const log_element &a, const log_element &b) {
    return {
            .tuple = a.tuple - b.tuple,
            .nsec = a.nsec - b.nsec,
            .tsc = a.tsc - b.tsc,
    };
}
static inline log_element &operator+=(log_element &a, const log_element &b) {
    a.tuple += b.tuple;
    a.nsec += b.nsec;
    a.tsc += b.tsc;
    return a;
}
static inline log_element &operator-=(log_element &a, const log_element &b) {
    a.tuple -= b.tuple;
    a.nsec -= b.nsec;
    a.tsc -= b.tsc;
    return a;
}

static void delta_encode(const struct raw_log_element *src, struct log_element *dst, size_t number_of_elements,
        const log_element *prev_param) {
    struct log_element prev = {0, 0, 0};
    if (prev_param != nullptr)
        prev = *prev_param;
    for (; number_of_elements--; src++, dst++) {
        log_element cur = log_element_from_raw(*src);
        *dst = cur - prev;
        prev = cur;
    }
}
static void delta_decode(const struct log_element *src, struct raw_log_element *dst, size_t number_of_elements,
        const log_element *prev_param) {
    struct log_element prev = {0, 0, 0};
    if (prev_param != nullptr)
        prev = *prev_param;
    for (; number_of_elements--; src++, dst++) {
//        log_element cur = prev + *src;
//        *dst = log_element_to_raw(cur);
//        prev = cur;
        prev += *src;
        *dst = log_element_to_raw(prev);
    }
}

int comp_encoder_delta::encode(const void *src, size_t src_size,
        comp_chunk *dst, size_t dst_size) {
    encoder_do_sanity_check;
    size_t size = src_size / sizeof(raw_log_element);
    dst_size = size * sizeof(log_element);
    init_header(comp_decoder_delta::CHUNK_TYPE);
    delta_encode((const raw_log_element*)src,
            (log_element*)dst->data, size, nullptr);
    return COMP_STATUS_SUCCESS;
}

size_t comp_encoder_delta::dst_size_upper_bound(size_t src_size) const {
    return COMP_CHUNK_SIZE(src_size / sizeof(raw_log_element) * sizeof(log_element));
}

int comp_encoder_delta::sanity_check(const void *src, size_t src_size, comp_chunk *dst, size_t dst_size) const {
    int err = comp_encoder::sanity_check(src, src_size, dst, dst_size);
    if (err) return err;
    if (src_size % sizeof(raw_log_element) != 0) return COMP_STATUS_ILLEGAL_FORMAT;
    return COMP_STATUS_SUCCESS;
}

int comp_decoder_delta::decode(const comp_chunk *src, void *dst) {
    decoder_do_sanity_check;
    size_t size = src->header.raw_size / sizeof(raw_log_element);
    delta_decode((const log_element*)src->data, (raw_log_element*)dst,
            size, nullptr);
    return COMP_STATUS_SUCCESS;
}

uint32_t comp_decoder_delta::single_type() const {
    return CHUNK_TYPE;
}

int comp_decoder_delta::sanity_check(const comp_chunk *src, void *dst) const {
    int err = comp_decoder_single_type::sanity_check(src, dst);
    if (err) return err;
    size_t size = src->header.raw_size / sizeof(raw_log_element);
    if (COMP_DATA_SIZE(src->header.chunk_size) != size * sizeof(log_element) ||
            src->header.raw_size != size * sizeof(raw_log_element))
        return COMP_STATUS_ILLEGAL_FORMAT;
    return COMP_STATUS_SUCCESS;
}
