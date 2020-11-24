//
// Created by jiho on 20. 9. 20..
//

#include "impl_common.h"
#include <zlib.h>

static int comp_deflate(const void *src, size_t src_size,
        void *dst, size_t *dst_size, int level) {
    int err;
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    err = deflateInit(&strm, level);
    if (err < 0)
        return COMP_STATUS_ZLIB_ERROR + err;
    strm.avail_in = src_size;
    strm.next_in = (typeof(strm.next_in)) src;
    strm.avail_out = *dst_size;
    strm.next_out = (typeof(strm.next_out)) dst;
    err = deflate(&strm, Z_FINISH);
    int err2 = deflateEnd(&strm);
    if (err < 0 && err != Z_BUF_ERROR) return COMP_STATUS_ZLIB_ERROR + err;
    if (err2 < 0) return COMP_STATUS_ZLIB_ERROR + err2;
    if (err == Z_BUF_ERROR || err == Z_OK) return COMP_STATUS_OVERFLOW;
    *dst_size -= strm.avail_out;
    return COMP_STATUS_SUCCESS;
}

static int comp_inflate(const void *src, size_t src_size,
        void *dst, size_t *dst_size) {
    int err;
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    err = inflateInit(&strm);
    if (err < 0)
        return COMP_STATUS_ZLIB_ERROR + err;
    strm.avail_in = src_size;
    strm.next_in = (typeof(strm.next_in)) src;
    strm.avail_out = *dst_size;
    strm.next_out = (typeof(strm.next_out)) dst;
    err = inflate(&strm, Z_FINISH);
    int err2 = inflateEnd(&strm);
    if (err < 0 && err != Z_BUF_ERROR) return COMP_STATUS_ZLIB_ERROR + err;
    if (err2 < 0) return COMP_STATUS_ZLIB_ERROR + err2;
    if (err == Z_BUF_ERROR) return COMP_STATUS_OVERFLOW;
    *dst_size -= strm.avail_out;
    return COMP_STATUS_SUCCESS;
}

comp_encoder_deflate::comp_encoder_deflate(int level)
        : m_level(level) {}

int comp_encoder_deflate::encode(const void *src, size_t src_size,
        comp_chunk *dst, size_t dst_size) {
    encoder_do_sanity_check;
    dst_size = COMP_DATA_SIZE(dst_size_upper_bound(src_size));
    int ret = comp_deflate(src, src_size, dst->data, &dst_size, m_level);
    if (ret == COMP_STATUS_SUCCESS) {
        init_header(comp_decoder_inflate::CHUNK_TYPE);
    } else if (ret == COMP_STATUS_OVERFLOW) {
        return comp_encoder_id::encode(src, src_size, dst, dst_size);
    }
    return ret;
}

int comp_decoder_inflate::decode(const comp_chunk *src, void *dst) {
    decoder_do_sanity_check;
    size_t dst_size = src->header.raw_size;
    int ret = comp_inflate(src->data, COMP_DATA_SIZE(src->header.chunk_size),
            dst, &dst_size);
    if (ret == COMP_STATUS_SUCCESS) {
        if (dst_size != src->header.raw_size)
            ret = COMP_STATUS_ILLEGAL_FORMAT;
    }
    return ret;
}

uint32_t comp_decoder_inflate::single_type() const {
    return CHUNK_TYPE;
}

