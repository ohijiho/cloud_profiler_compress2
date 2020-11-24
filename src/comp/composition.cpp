//
// Created by jiho on 20. 9. 20..
//

#include "impl_common.h"
#include <cstring>
#include <comp.h>


comp_encoder_composition::comp_encoder_composition(comp_encoder *outer, comp_encoder *inner)
        : f(outer), g(inner) {}

int comp_encoder_composition::encode(const void *src, size_t src_size, comp_chunk *dst, size_t dst_size) {
    tmp.resize(g->dst_size_upper_bound(src_size));
    auto *tmp_chunk = (comp_chunk*)tmp.data();
    int err = g->encode(src, src_size,
            tmp_chunk, tmp.size());
    if (err) return err;
    auto *dst_data_chunk = reinterpret_cast<comp_chunk *>(dst->data);
    err = f->encode(tmp.data(), tmp_chunk->header.chunk_size,
            dst_data_chunk, dst_size);
    if (err == COMP_STATUS_SUCCESS) {
        dst_size = dst_data_chunk->header.chunk_size;
        init_header(comp_decoder_composition::CHUNK_TYPE);
    }
    return err;
}

int comp_encoder_composition::sanity_check(const void *src, size_t src_size, comp_chunk *dst, size_t dst_size) const {
    return g->sanity_check(src, src_size, dst, dst_size);
}

size_t comp_encoder_composition::dst_size_upper_bound(size_t src_size) const {
    return COMP_CHUNK_SIZE(f->dst_size_upper_bound(g->dst_size_upper_bound(src_size)));
}


comp_decoder_composition::comp_decoder_composition(comp_decoder *outer, comp_decoder *inner)
        : f(outer), g(inner) {}

int comp_decoder_composition::decode(const comp_chunk *src, void *dst) {
    decoder_do_sanity_check;
    auto f_src = reinterpret_cast<const comp_chunk *>(src->data);
    tmp.resize(f_src->header.raw_size);
    int err = f->decode(f_src, tmp.data());
    if (err) return err;
    auto tmp_chunk = reinterpret_cast<const comp_chunk *>(tmp.data());
    if (tmp_chunk->header.raw_size != src->header.raw_size)
        return COMP_STATUS_ILLEGAL_FORMAT;
    return f->decode(tmp_chunk, dst);
}

int comp_decoder_composition::sanity_check(const comp_chunk *src, void *dst) const {
    int err = comp_decoder_single_type::sanity_check(src, dst);
    if (err) return err;
    auto f_src = reinterpret_cast<const comp_chunk *>(src->data);
    if (COMP_DATA_SIZE(src->header.chunk_size) != f_src->header.chunk_size)
        return COMP_STATUS_ILLEGAL_FORMAT;
    return f->sanity_check(f_src, dst);
}

uint32_t comp_decoder_composition::single_type() const {
    return CHUNK_TYPE;
}
