#ifndef BitcoinWords_H
#define BitcoinWords_H

#ifdef __cplusplus
extern "C" {
#endif


#define BTCWORDS_NUM   2048
#define BTCWORDS_MAX_LEN   8

extern const char BtcWords[BTCWORDS_NUM][BTCWORDS_MAX_LEN+1];

uint16_t SearchBtcWords(char *w);

void test_sha256();

#ifdef __cplusplus
}
#endif

#endif
