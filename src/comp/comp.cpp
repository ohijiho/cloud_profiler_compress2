//
// Created by jiho on 20. 9. 20..
//

#include "impl_common.h"
#include <cstring>
#include <comp.h>


int comp_encoder::sanity_check(const void *src, size_t src_size,
        comp_chunk *dst, size_t dst_size) const {
    if (src == nullptr || dst == nullptr) return COMP_STATUS_FATAL_ERROR;
    if (dst_size < dst_size_upper_bound(src_size))
        return COMP_STATUS_FATAL_ERROR;
    return COMP_STATUS_SUCCESS;
}

size_t comp_encoder::dst_size_upper_bound(size_t src_size) const {
    return COMP_CHUNK_SIZE(src_size);
}


int comp_encoder_id::encode(const void *src, size_t src_size,
        comp_chunk *dst, size_t dst_size) {
    encoder_do_sanity_check;
    dst_size = src_size;
    init_header(comp_decoder_id::CHUNK_TYPE);
    memcpy(dst->data, src, src_size);
    return COMP_STATUS_SUCCESS;
}

int comp_decoder::sanity_check(const comp_chunk *src, void *dst) const {
    if (src == nullptr || dst == nullptr) return COMP_STATUS_FATAL_ERROR;
    return COMP_STATUS_SUCCESS;
}

int comp_decoder_single_type::sanity_check(const comp_chunk *src, void *dst) const {
    int err = comp_decoder::sanity_check(src, dst);
    if (err) return err;
    if (src->header.type != single_type()) return COMP_STATUS_UNSUPPORTED_FORMAT;
    return COMP_STATUS_SUCCESS;
}

int comp_decoder_id::decode(const comp_chunk *src, void *dst) {
    decoder_do_sanity_check;
    memcpy(dst, src->data, src->header.raw_size);
    return COMP_STATUS_SUCCESS;
}

uint32_t comp_decoder_id::single_type() const {
    return CHUNK_TYPE;
}

int comp_decoder_id::sanity_check(const comp_chunk *src, void *dst) const {
    int err = comp_decoder_single_type::sanity_check(src, dst);
    if (err) return err;
    if (COMP_DATA_SIZE(src->header.chunk_size) != src->header.raw_size)
        return COMP_STATUS_ILLEGAL_FORMAT;
    return COMP_STATUS_SUCCESS;
}

int comp_decoder_union::decode(const comp_chunk *src, void *dst) {
    auto it = fmap.find(src->header.type);
    if (it == fmap.end()) return COMP_STATUS_UNSUPPORTED_FORMAT;
    auto f = it->second;
    return f->decode(src, dst);
}

int comp_decoder_union::sanity_check(const comp_chunk *src, void *dst) const {
    auto it = fmap.find(src->header.type);
    if (it == fmap.end()) return COMP_STATUS_UNSUPPORTED_FORMAT;
    auto f = it->second;
    return f->sanity_check(src, dst);
}

comp_decoder_union::comp_decoder_union(const std::initializer_list<comp_decoder_single_type *> &single_type_list) {
    for (auto *f : single_type_list) {
        fmap[f->single_type()] = f;
    }
}

comp_decoder_union::comp_decoder_union(std::map<uint32_t, comp_decoder *> decoder_map)
        : fmap(std::move(decoder_map)) {}

//void comp_decoder_union::add_decoder(uint32_t type, comp_decoder *decoder) {
//    fmap[type] = decoder;
//}
//
//void comp_decoder_union::add_decoder(comp_decoder_single_type *decoder) {
//    fmap[decoder->single_type()] = decoder;
//}
