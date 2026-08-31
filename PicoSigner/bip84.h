/*
 * bip84.h
 *
 * BIP-39 (12 words) -> BIP-84 (m/84'/0'/0'/0/0) native SegWit address.
 *
 * Port of the derivation validated in the PC app "BtcWords" for the
 * RPI PicoSigner, using mbedTLS bundled with the Pico SDK:
 *   - SHA-512 / HMAC-SHA512 / PBKDF2 (BIP-39 seed)
 *   - secp256k1 (BIP-32 child derivation)
 *   - SHA-256 + RIPEMD-160 (hash160) and bech32 -> bc1... address
 *
 * Air-gapped: only the public address leaves the device (display/serial/QR).
 * The words and private key never leave.
 *
 * Author: Klaus Fensterseifer / port
 */

#ifndef BIP84_H
#define BIP84_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Derives the BIP-84 address from 12 BIP-39 words.
 *
 * words[]        : array of 12 NULL-terminated words (lower case)
 * address_out    : caller buffer
 * address_out_size: size of address_out (at least 100)
 *
 * Returns 0 on success, non-zero on failure
 * (wrong count, unknown word, bad checksum, derive/mem error).
 * On success a NULL-terminated bech32 address ("bc1...") is written.
 */
int bip84_address_from_words(const char *words[12],
                             char *address_out, size_t address_out_size);

/*
 * BIP-39 checksum check only: returns 1 if the 12 words pass the checksum,
 * 0 otherwise (also 0 if any word is unknown or count != 12).
 */
int bip39_words_checksum_ok(const char *words[12]);

#ifdef __cplusplus
}
#endif

#endif
