//
// Created by jiho on 20. 9. 20..
//

#ifndef COMPRESS2_IMPL_COMMON_H
#define COMPRESS2_IMPL_COMMON_H

#include <comp.h>

#define encoder_do_sanity_check do { \
    int do_sanity_check_err = sanity_check(src, src_size, dst, dst_size); \
    if (do_sanity_check_err) return do_sanity_check_err;                                  \
} while (0)

#define decoder_do_sanity_check do { \
    int do_sanity_check_err = sanity_check(src, dst); \
    if (do_sanity_check_err) return do_sanity_check_err;                                  \
} while (0)

/*
 * dst_size must be data size
 */
#define init_header(chunk_type) do {\
    dst->header = {                 \
        .type = chunk_type,         \
        .chunk_size = (uint32_t)COMP_CHUNK_SIZE(dst_size), \
        .raw_size = (uint32_t)src_size,                    \
    };                              \
} while (0)

#endif //COMPRESS2_IMPL_COMMON_H
