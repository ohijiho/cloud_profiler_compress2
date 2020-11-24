//
// Created by jiho on 20. 9. 20..
//

#include "impl_common.h"

comp_decoder_universal::comp_decoder_universal()
        : comp_decoder_union({
        new comp_decoder_id(),
        new comp_decoder_inflate(),
        new comp_decoder_delta(),
        new comp_decoder_delta_bitpack(),
        new comp_decoder_composition(this, this),
}) {}

comp_decoder_universal::~comp_decoder_universal() {
    for (auto p : fmap)
        delete p.second;
}

comp_encoder_delta_deflate::comp_encoder_delta_deflate(int level)
        : comp_encoder_composition(&f_buf, &g_buf), f_buf(level), g_buf() {}

comp_encoder_delta_bitpack_deflate::comp_encoder_delta_bitpack_deflate(int level)
        : comp_encoder_composition(&f_buf, &g_buf), f_buf(level), g_buf() {}
