#ifndef SHA256_HASH_H__
#define SHA256_HASH_H__ 1

#include <stddef.h>
#include "simd-utils.h"
#include "cpuminer-config.h"

// generic interface 

typedef struct
{
   unsigned char buf[64];    /* first field, for alignment */
   uint32_t state[8];
   uint64_t count;
} sha256_context __attribute__((aligned(64)));

static const uint32_t SHA256_IV[8];

void sha256_full( void *hash, const void *data, size_t len );
void sha256_update( sha256_context *ctx, const void *data, size_t len );
void sha256_final( sha256_context *ctx, void *hash );
void sha256_ctx_init( sha256_context *ctx );
void sha256_transform_le( uint32_t *state_out, const uint32_t *data,
                          const uint32_t *state_in );
void sha256_transform_be( uint32_t *state_out, const uint32_t *data,
                          const uint32_t *state_in );

#if defined(__x86_64__) && defined(__SHA__)

void sha256_opt_transform_le( uint32_t *state_out, const void *input,
                           const uint32_t *state_in );

void sha256_opt_transform_be( uint32_t *state_out, const void *input,
                           const uint32_t *state_in );

// 2 way serial with interleaved instructions
void sha256_ni2x_transform_le( uint32_t *out_X, uint32_t*out_Y,
                              const void *msg_X, const void *msg_Y,
                              const uint32_t *in_X, const uint32_t *in_Y );

void sha256_ni2x_transform_be( uint32_t *out_X, uint32_t*out_Y,
                              const void *msg_X, const void *msg_Y,
                              const uint32_t *in_X, const uint32_t *in_Y );

void sha256_ni_prehash_3rounds( uint32_t *ostate, const void *msg,
                              uint32_t *sstate, const uint32_t *istate );

void sha256_ni2x_final_rounds( uint32_t *state_out_X, uint32_t *state_out_Y,
                 const void *msg_X, const void *msg_Y,
                 const uint32_t *state_mid_X, const uint32_t *state_mid_Y,
                 const uint32_t *state_save_X, const uint32_t *state_save_Y );

#define sha256_transform_le         sha256_opt_transform_le
#define sha256_transform_be         sha256_opt_transform_be
#define sha256_2x_transform_le      sha256_ni2x_transform_le
#define sha256_2x_transform_be      sha256_ni2x_transform_be
#define sha256_prehash_3rounds      sha256_ni_prehash_3rounds
#define sha256_2x_final_rounds      sha256_ni2x_final_rounds

#elif defined(__aarch64__) && defined(__ARM_NEON)

void sha256_neon_transform_be( uint32_t *state_out, const void *input,
                               const uint32_t *state_in );
void sha256_neon_transform_le( uint32_t *state_out, const void *input,
                               const uint32_t *state_in );

void sha256_neon2x_transform_le( uint32_t *out_X, uint32_t*out_Y,
                              const void *msg_X, const void *msg_Y,
                              const uint32_t *in_X, const uint32_t *in_Y );

void sha256_neon2x_transform_be( uint32_t *out_X, uint32_t*out_Y,
                              const void *msg_X, const void *msg_Y,
                              const uint32_t *in_X, const uint32_t *in_Y );

void sha256_neon_prehash_3rounds( uint32_t *ostate, const void *msg,
                              uint32_t *sstate, const uint32_t *istate );

void sha256_neon2x_final_rounds( uint32_t *state_out_X, uint32_t *state_out_Y,
                 const void *msg_X, const void *msg_Y,
                 const uint32_t *state_mid_X, const uint32_t *state_mid_Y,
                 const uint32_t *state_save_X, const uint32_t *state_save_Y );

#define sha256_transform_le         sha256_neon_transform_le
#define sha256_transform_be         sha256_neon_transform_be
#define sha256_2x_transform_le      sha256_neon2x_transform_le
#define sha256_2x_transform_be      sha256_neon2x_transform_be
#define sha256_prehash_3rounds      sha256_neon_prehash_3rounds
#define sha256_2x_final_rounds      sha256_neon2x_final_rounds

#else
// without HW acceleration...
#include "sph_sha2.h"

#define sha256_transform_le         sph_sha256_transform_le
#define sha256_transform_be         sph_sha256_transform_be
#define sha256_prehash_3rounds      sph_sha256_prehash_3rounds

#endif

#if defined(__AVX512F__) && defined(__AVX512VL__) && defined(__AVX512DQ__) && defined(__AVX512BW__)

// SHA-256 16 way

typedef struct
{
   __m512i buf[64>>2];
   __m512i val[8];
   uint32_t count_high, count_low;
} sha256_16way_context __attribute__ ((aligned (128)));

void sha256_16way_init( sha256_16way_context *sc );
void sha256_16way_update( sha256_16way_context *sc, const void *data, size_t len );
void sha256_16way_close( sha256_16way_context *sc, void *dst );
void sha256_16way_full( void *dst, const void *data, size_t len );
void sha256_16way_transform_le( __m512i *state_out, const __m512i *data,
                             const __m512i *state_in );
void sha256_16way_transform_be( __m512i *state_out, const __m512i *data,
                             const __m512i *state_in );
void sha256_16way_prehash_3rounds( __m512i *state_mid, __m512i *X,
                                  const __m512i *W, const __m512i *state_in );
void sha256_16way_final_rounds( __m512i *state_out, const __m512i *data,
        const __m512i *state_in, const __m512i *state_mid, const __m512i *X );

int sha256_16way_transform_le_short( __m512i *state_out, const __m512i *data,
                            const __m512i *state_in, const uint32_t *target );

#endif // AVX512

#if defined (__AVX2__)

// SHA-256 8 way

typedef struct
{
   __m256i buf[64>>2];
   __m256i val[8];
   uint32_t count_high, count_low;
} sha256_8way_context __attribute__ ((aligned (64)));

void sha256_8way_init( sha256_8way_context *sc );
void sha256_8way_update( sha256_8way_context *sc, const void *data, size_t len );
void sha256_8way_close( sha256_8way_context *sc, void *dst );
void sha256_8way_full( void *dst, const void *data, size_t len );
void sha256_8way_transform_le( __m256i *state_out, const __m256i *data,
                               const __m256i *state_in );
void sha256_8way_transform_be( __m256i *state_out, const __m256i *data,
                               const __m256i *state_in );

void sha256_8way_prehash_3rounds( __m256i *state_mid, __m256i *X,
                                 const __m256i *W, const __m256i *state_in );
void sha256_8way_final_rounds( __m256i *state_out, const __m256i *data,
        const __m256i *state_in, const __m256i *state_mid, const __m256i *X );
int sha256_8way_transform_le_short( __m256i *state_out, const __m256i *data,
                             const __m256i *state_in, const uint32_t *target );

#endif  // AVX2

// SHA-256 4 way

typedef struct
{
   v128_t buf[64>>2];
   v128_t val[8];
   uint32_t count_high, count_low;
} sha256_4way_context __attribute__ ((aligned (32)));

void sha256_4way_init( sha256_4way_context *sc );
void sha256_4way_update( sha256_4way_context *sc, const void *data,
                         size_t len );
void sha256_4way_close( sha256_4way_context *sc, void *dst );
void sha256_4way_full( void *dst, const void *data, size_t len );
void sha256_4way_transform_le( v128_t *state_out,  const v128_t *data,
                            const v128_t *state_in );
void sha256_4way_transform_be( v128_t *state_out,  const v128_t *data,
                            const v128_t *state_in );
void sha256_4way_prehash_3rounds( v128_t *state_mid, v128_t *X,
                                   const v128_t *W, const v128_t *state_in );
void sha256_4way_final_rounds( v128_t *state_out, const v128_t *data,
        const v128_t *state_in, const v128_t *state_mid, const v128_t *X );
int sha256_4way_transform_le_short( v128_t *state_out, const v128_t *data,
                                   const v128_t *state_in, const uint32_t *target );


#endif
