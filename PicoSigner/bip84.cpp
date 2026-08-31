/*
 * bip84.cpp
 *
 * BIP-39 (12 words) -> BIP-84 (m/84'/0'/0'/0/0) native SegWit (bc1...) address.
 *
 * Uses mbedTLS from the Pico SDK (bundled with the Philhower core):
 *   SHA-512 / HMAC-SHA512 / PBKDF2 (BIP-39 seed)
 *   secp256k1 (BIP-32 CKDpriv / compressed pubkey)
 *   SHA-256 + RIPEMD-160 (hash160) and bech32 (BIP-173)
 *
 * The word list and search come from the existing BitcoinWords module:
 *   BtcWords[][], SearchBtcWords().
 *
 * Air-gapped: only the public address is ever output (display/serial/QR).
 */

#include "Arduino.h"
#include "bip84.h"
#include "BitcoinWords.h"
#include <MBedTLS_Btc84.h>
#include <string.h>

/* Minimal RNG callback for mbedtls_ecp_mul (which requires f_rng != NULL).
 * It is only used for internal scalar blinding; the resulting point is the
 * same regardless of the produced bytes. Pure, deterministic, no entropy. */
static int bip84_dummy_rng(void *ctx, unsigned char *buf, size_t len)
{
    (void)ctx;
    memset(buf, 0x5A, len);
    return 0;
}

static void sha256_one(const unsigned char *d, size_t n, unsigned char *o)
{
    mbedtls_sha256(d, n, o, 0);
}

static void hash160(const unsigned char *d, size_t n, unsigned char *o)
{
    unsigned char m[32];
    sha256_one(d, n, m);
    mbedtls_ripemd160(m, 32, o);
}

/* Compressed secp256k1 public key for scalar k. *olen == 33.
 * Returns 0 on success. */
static int pubkey_compressed(const unsigned char *k, unsigned char *out, size_t *olen)
{
    int ret = -1;
    mbedtls_ecp_group grp;
    mbedtls_ecp_point P;
    mbedtls_mpi kb;

    mbedtls_ecp_group_init(&grp);
    mbedtls_ecp_point_init(&P);
    mbedtls_mpi_init(&kb);

    if (mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_SECP256K1) != 0) goto done;
    if (mbedtls_mpi_read_binary(&kb, k, 32) != 0) goto done;
    /* R = k * G  (generator of the loaded group) */
    if (mbedtls_ecp_mul(&grp, &P, &kb, &grp.G, bip84_dummy_rng, NULL) != 0) goto done;
    if (mbedtls_ecp_point_write_binary(&grp, &P, MBEDTLS_ECP_PF_COMPRESSED,
                                       olen, out, 33) != 0) goto done;
    ret = 0;

done:
    mbedtls_ecp_group_free(&grp);
    mbedtls_ecp_point_free(&P);
    mbedtls_mpi_free(&kb);
    return ret;
}

/* BIP-32 CKDpriv. Non-hardened children use the parent public key (this is
 * the part most implementations get wrong). Returns 0 on success. */
static int ckdpriv(unsigned char *kpar, unsigned char *cpar, uint32_t index)
{
    unsigned char data[65];
    unsigned char pk[33];
    size_t dlen;

    if (index & 0x80000000UL) {
        data[0] = 0;
        memcpy(data + 1, kpar, 32);
        dlen = 33;
    } else {
        if (pubkey_compressed(kpar, pk, &dlen) != 0) return -1;
        memcpy(data, pk, dlen);
    }
    data[dlen++] = (unsigned char)(index >> 24);
    data[dlen++] = (unsigned char)(index >> 16);
    data[dlen++] = (unsigned char)(index >> 8);
    data[dlen++] = (unsigned char)index;

    unsigned char L[64];
    mbedtls_md_hmac(mbedtls_md_info_from_type(MBEDTLS_MD_SHA512),
                    cpar, 32, data, dlen, L);

    /* kchild = (IL + kpar) mod n */
    mbedtls_mpi n, su, kp, sum;
    mbedtls_mpi_init(&n); mbedtls_mpi_init(&su);
    mbedtls_mpi_init(&kp); mbedtls_mpi_init(&sum);
    mbedtls_mpi_read_string(&n, 16,
        "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141");
    mbedtls_mpi_read_binary(&su, L, 32);
    mbedtls_mpi_read_binary(&kp, kpar, 32);
    mbedtls_mpi_add_mpi(&sum, &su, &kp);
    mbedtls_mpi_mod_mpi(&sum, &sum, &n);
    mbedtls_mpi_write_binary(&sum, kpar, 32);
    memcpy(cpar, L + 32, 32);
    mbedtls_mpi_free(&n); mbedtls_mpi_free(&su);
    mbedtls_mpi_free(&kp); mbedtls_mpi_free(&sum);
    return 0;
}

/* ---------------- bech32 (BIP-173) ---------------- */
static const char CHARSET[] = "qpzry9x8gf2tvdw0s3jn54khce6mua7l";

static uint32_t bech32_step(uint32_t pre)
{
    uint8_t b = pre >> 25;
    return ((pre & 0x1FFFFFF) << 5) ^
        (-((b >> 0) & 1) & 0x3b6a57b2UL) ^
        (-((b >> 1) & 1) & 0x26508e6dUL) ^
        (-((b >> 2) & 1) & 0x1ea119faUL) ^
        (-((b >> 3) & 1) & 0x3d4233ddUL) ^
        (-((b >> 4) & 1) & 0x2a1462b3UL);
}

/* Returns length written (incl. NUL) or 0 on overflow. */
static size_t bech32_p2wpkh(char *out, size_t out_size, const unsigned char *hash20)
{
    uint8_t data[34];
    size_t dl = 1;
    int bits = 0;
    uint32_t val = 0;
    data[0] = 0; /* witness version 0 */

    for (int i = 0; i < 20; i++) {
        val = (val << 8) | hash20[i];
        bits += 8;
        while (bits >= 5) { bits -= 5; data[dl++] = (val >> bits) & 0x1f; }
    }
    if (bits) data[dl++] = (val << (5 - bits)) & 0x1f;

    const char *hrp = "bc";
    uint32_t chk = 1;
    size_t i = 0;
    while (hrp[i]) { chk = bech32_step(chk) ^ (hrp[i] >> 5); ++i; }
    chk = bech32_step(chk);
    for (i = 0; hrp[i]; i++) { chk = bech32_step(chk) ^ (hrp[i] & 0x1f); }
    for (i = 0; i < dl; i++) { chk = bech32_step(chk) ^ data[i]; }
    for (i = 0; i < 6; i++) chk = bech32_step(chk);
    chk ^= 1;

    size_t len = 3 + dl + 6; /* "bc1" + data + 6 checksum */
    if (len >= out_size) return 0;

    size_t j = 0;
    out[j++] = 'b'; out[j++] = 'c'; out[j++] = '1';
    for (i = 0; i < dl; i++) out[j++] = CHARSET[data[i]];
    for (i = 0; i < 6; i++) out[j++] = CHARSET[(chk >> ((5 - i) * 5)) & 0x1f];
    out[j] = 0;
    return j;
}

/* Pack 12 indices (11 bits each) -> 16-byte entropy + 4-bit checksum.
 * Returns 0 on success (entropy filled, *cs4 = checksum nibble). */
static int entropy_checksum_from_words(const char *words[12],
                                       unsigned char entropy[16], int *cs4)
{
    unsigned char bits[17] = {0};
    int total = 0;
    for (int w = 0; w < 12; w++) {
        uint16_t idx = SearchBtcWords((char *)words[w]);
        if (idx >= BTCWORDS_NUM) return -1;
        for (int b = 10; b >= 0; b--) {
            if ((idx >> b) & 1) {
                int p = total;
                bits[p / 8] |= (unsigned char)(0x80 >> (p % 8));
            }
            total++;
        }
    }
    memcpy(entropy, bits, 16);
    *cs4 = (bits[16] >> 4) & 0x0F;
    return 0;
}

int bip39_words_checksum_ok(const char *words[12])
{
    unsigned char entropy[16];
    int cs4;
    if (entropy_checksum_from_words(words, entropy, &cs4) != 0) return 0;
    unsigned char h[32];
    sha256_one(entropy, 16, h);
    int expected = h[0] >> 4;
    return expected == cs4 ? 1 : 0;
}

int bip84_address_from_words(const char *words[12],
                             char *address_out, size_t address_out_size)
{
    if (bip39_words_checksum_ok(words) != 1) return -2;

    /* 1. mnemonic string (space separated) */
    char mnemonic[140];
    size_t mlen = 0;
    for (int i = 0; i < 12; i++) {
        size_t l = strlen(words[i]);
        if (mlen + l + 1 >= sizeof(mnemonic)) return -3;
        if (i) mnemonic[mlen++] = ' ';
        memcpy(mnemonic + mlen, words[i], l);
        mlen += l;
    }
    mnemonic[mlen] = 0;

    /* 2. BIP-39 seed = PBKDF2(HMAC-SHA512, password=mnemonic,
     *    salt="mnemonic", iter=2048, len=64) */
    unsigned char seed[64];
    if (mbedtls_pkcs5_pbkdf2_hmac_ext(
            MBEDTLS_MD_SHA512,
            (const unsigned char *)mnemonic, mlen,
            (const unsigned char *)"mnemonic", 8,
            2048, 64, seed) != 0) return -4;

    /* 3. BIP-32 master: HMAC-SHA512("Bitcoin seed", seed) */
    unsigned char I[64];
    mbedtls_md_hmac(mbedtls_md_info_from_type(MBEDTLS_MD_SHA512),
                    (const unsigned char *)"Bitcoin seed", 12, seed, 64, I);
    unsigned char k[32], c[32];
    memcpy(k, I, 32);
    memcpy(c, I + 32, 32);

    /* 4. Child derivation m/84'/0'/0'/0/0 */
    const uint32_t path[5] = {0x80000054UL, 0x80000000UL, 0x80000000UL, 0, 0};
    for (int i = 0; i < 5; i++) {
        if (ckdpriv(k, c, path[i]) != 0) return -5;
    }

    /* 5. pubkey -> hash160 -> bech32 */
    unsigned char pub[33];
    size_t plen;
    if (pubkey_compressed(k, pub, &plen) != 0) return -6;
    unsigned char hh[20];
    hash160(pub, plen, hh);

    if (bech32_p2wpkh(address_out, address_out_size, hh) == 0) return -7;
    return 0;
}
