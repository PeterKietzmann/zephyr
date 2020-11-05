#include <stdint.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/**
 * @brief   Length of SHA256 digests in bytes
 */
#define SHA256_DIGEST_LENGTH 32

/**
 * @brief 512 Bit (64 Byte) internally used block size for sha256
 */
#define SHA256_INTERNAL_BLOCK_SIZE (64)

/**
 * @brief Context for cipher operations based on sha256
 */
typedef struct {
    /** global state */
    uint32_t state[8];
    /** processed bytes counter */
    uint32_t count[2];
    /** data buffer */
    unsigned char buf[64];
} sha256_context_t;


#ifdef __BIG_ENDIAN__
/* Copy a vector of big-endian uint32_t into a vector of bytes */
#define be32enc_vect memcpy

/* Copy a vector of bytes into a vector of big-endian uint32_t */
#define be32dec_vect memcpy

#else /* !__BIG_ENDIAN__ */

/*
 * Encode a length len/4 vector of (uint32_t) into a length len vector of
 * (unsigned char) in big-endian form.  Assumes len is a multiple of 4.
 */
static void be32enc_vect(void *dst_, const void *src_, size_t len)
{
    if ((uintptr_t)dst_ % sizeof(uint32_t) == 0 &&
        (uintptr_t)src_ % sizeof(uint32_t) == 0) {
        uint32_t *dst = dst_;
        const uint32_t *src = src_;
        for (size_t i = 0; i < len / 4; i++) {
            dst[i] = __builtin_bswap32(src[i]);
        }
    }
    else {
        uint8_t *dst = dst_;
        const uint8_t *src = src_;
        for (size_t i = 0; i < len; i += 4) {
            dst[i] = src[i + 3];
            dst[i + 1] = src[i + 2];
            dst[i + 2] = src[i + 1];
            dst[i + 3] = src[i];
        }
    }
}

/*
 * Decode a big-endian length len vector of (unsigned char) into a length
 * len/4 vector of (uint32_t).  Assumes len is a multiple of 4.
 */
#define be32dec_vect be32enc_vect

#endif /* __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__ */

/* Elementary functions used by SHA256 */
#define Ch(x, y, z) ((x & (y ^ z)) ^ z)
#define Maj(x, y, z)    ((x & (y | z)) | (y & z))
#define SHR(x, n)   (x >> n)
#define ROTR(x, n)  ((x >> n) | (x << (32 - n)))
#define S0(x)       (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define S1(x)       (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define s0(x)       (ROTR(x, 7) ^ ROTR(x, 18) ^ SHR(x, 3))
#define s1(x)       (ROTR(x, 17) ^ ROTR(x, 19) ^ SHR(x, 10))

static const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};

/*
 * SHA256 block compression function.  The 256-bit state is transformed via
 * the 512-bit input block to produce a new state.
 */
static void sha256_transform(uint32_t *state, const unsigned char block[64])
{
    uint32_t W[64];
    uint32_t S[8];

    /* 1. Prepare message schedule W. */
    be32dec_vect(W, block, 64);
    for (int i = 16; i < 64; i++) {
        W[i] = s1(W[i - 2]) + W[i - 7] + s0(W[i - 15]) + W[i - 16];
    }

    /* 2. Initialize working variables. */
    memcpy(S, state, 32);

    /* 3. Mix. */
    for (int i = 0; i < 64; ++i) {
        uint32_t e = S[(68 - i) % 8], f = S[(69 - i) % 8];
        uint32_t g = S[(70 - i) % 8], h = S[(71 - i) % 8];
        uint32_t t0 = h + S1(e) + Ch(e, f, g) + W[i] + K[i];

        uint32_t a = S[(64 - i) % 8], b = S[(65 - i) % 8];
        uint32_t c = S[(66 - i) % 8], d = S[(67 - i) % 8];
        uint32_t t1 = S0(a) + Maj(a, b, c);

        S[(67 - i) % 8] = d + t0;
        S[(71 - i) % 8] = t0 + t1;
    }

    /* 4. Mix local working variables into global state */
    for (int i = 0; i < 8; i++) {
        state[i] += S[i];
    }
}

static unsigned char PAD[64] = {
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
/* Add bytes into the hash */
void sha256_update(sha256_context_t *ctx, const void *data, size_t len)
{
    /* Number of bytes left in the buffer from previous updates */
    uint32_t r = (ctx->count[1] >> 3) & 0x3f;

    /* Convert the length into a number of bits */
    uint32_t bitlen1 = ((uint32_t) len) << 3;
    uint32_t bitlen0 = ((uint32_t) len) >> 29;

    /* Update number of bits */
    if ((ctx->count[1] += bitlen1) < bitlen1) {
        ctx->count[0]++;
    }

    ctx->count[0] += bitlen0;

    /* Handle the case where we don't need to perform any transforms */
    if (len < 64 - r) {
        if (len > 0) {
            memcpy(&ctx->buf[r], data, len);
        }
        return;
    }

    /* Finish the current block */
    const unsigned char *src = data;

    memcpy(&ctx->buf[r], src, 64 - r);
    sha256_transform(ctx->state, ctx->buf);
    src += 64 - r;
    len -= 64 - r;

    /* Perform complete blocks */
    while (len >= 64) {
        sha256_transform(ctx->state, src);
        src += 64;
        len -= 64;
    }

    /* Copy left over data into buffer */
    memcpy(ctx->buf, src, len);
}

/* Add padding and terminating bit-count. */
static void sha256_pad(sha256_context_t *ctx)
{
    /*
     * Convert length to a vector of bytes -- we do this now rather
     * than later because the length will change after we pad.
     */
    unsigned char len[8];

    be32enc_vect(len, ctx->count, 8);

    /* Add 1--64 bytes so that the resulting length is 56 mod 64 */
    uint32_t r = (ctx->count[1] >> 3) & 0x3f;
    uint32_t plen = (r < 56) ? (56 - r) : (120 - r);
    sha256_update(ctx, PAD, (size_t) plen);

    /* Add the terminating bit-count */
    sha256_update(ctx, len, 8);
}


/* SHA-256 initialization.  Begins a SHA-256 operation. */
void sha256_init(sha256_context_t *ctx)
{
    /* Zero bits processed so far */
    ctx->count[0] = ctx->count[1] = 0;

    /* Magic initialization constants */
    ctx->state[0] = 0x6A09E667;
    ctx->state[1] = 0xBB67AE85;
    ctx->state[2] = 0x3C6EF372;
    ctx->state[3] = 0xA54FF53A;
    ctx->state[4] = 0x510E527F;
    ctx->state[5] = 0x9B05688C;
    ctx->state[6] = 0x1F83D9AB;
    ctx->state[7] = 0x5BE0CD19;
}



/*
 * SHA-256 finalization.  Pads the input data, exports the hash value,
 * and clears the context state.
 */
void sha256_final(sha256_context_t *ctx, void *dst)
{
    /* Add padding */
    sha256_pad(ctx);

    /* Write the hash */
    be32enc_vect(dst, ctx->state, 32);

    /* Clear the context state */
    memset((void *) ctx, 0, sizeof(*ctx));
}



#define SEED_SIZE           (32)

static sha256_context_t ctx;
static uint32_t datapos = SEED_SIZE;
static int8_t digestdata[SEED_SIZE];
static int8_t prng_state[SEED_SIZE];

void _updatestate(int8_t *state)
{
    signed last = 1;
    char zf = 0;

    for (int i = 0; i < SEED_SIZE; i++) {
        signed v;
        char t;
        /* add two bytes */
        v = (int)state[i] + (int)digestdata[i] + last;

        /* result is lower 8 bits */
        t = (char)(v & 0xff);

        /* check for state collision */
        zf = zf | (state[i] != t);

        /* store result, */
        state[i] = t;

        /* high 8 bits are carry, store for next iteration */
        last = (v >> 8);
    }

    /* make sure at least one bit changes! */
    if (!zf) {
       state[0]++;
    }

    /* update SHA1 state */
    sha256_update(&ctx, (void *)state, sizeof(state));
}

void _random_bytes(uint8_t *bytes, size_t size) /* TODO: use with global API */
{
    uint32_t loc = 0;
    while (loc < size)
    {
        int copy;
        /* find min between remaining out bytes and so far unused seed bytes */
        if ( (size-loc) < (SEED_SIZE - datapos) )
        {
            copy = size-loc;
        }
        else
        {
            /* in first iteration this will be 0 */
            copy = SEED_SIZE - datapos;
        }
        if (copy > 0)
        {
            /* directly copy data to out buffer */
            memcpy(bytes + loc, digestdata + datapos, copy);
            datapos += copy;
            loc += copy;
        }
        /* no out data ready, (re)fill internal buffer */
        else
        {
            /* reset SHA1 internal state */
            sha256_init(&ctx);

            /* update SHA1 internal state with PRNG state */
            sha256_update(&ctx, (void *)prng_state, sizeof(prng_state));

            /* get the digest */
            sha256_final(&ctx, digestdata);

            /* update PRNG state for next round */
            _updatestate(prng_state);

            /* reset counter for buffer position */
            datapos = 0;
        }
    }
}

void random_init(uint32_t seed)
{
    printf("SHA256PRNG\n");
    sha256_init(&ctx);
    sha256_update(&ctx, (void *)&seed, sizeof(uint32_t));
    sha256_final(&ctx, digestdata);

    /* copy seeded SHA1 state to PRNG state */
    memcpy(prng_state, &ctx.state, SHA256_DIGEST_LENGTH);
}

uint32_t random_uint32(void)
{
    uint32_t ret;
    int8_t bytes[sizeof(uint32_t)];
    _random_bytes((uint8_t *)bytes, sizeof(uint32_t));

    ret = ((bytes[0] & 0xff) << 24)
        | ((bytes[1] & 0xff) << 16)
        | ((bytes[2] & 0xff) <<  8)
        |  (bytes[3] & 0xff);

    return ret;
}