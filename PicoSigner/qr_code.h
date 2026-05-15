#ifndef __QR_CODE_H__
#define __QR_CODE_H__

#ifdef __cplusplus
extern "C" {
#endif

// qr_code.h
void QRCode_gen_test();
//void QRCode_init();
void QRCode_init(uint8_t* image);
uint8_t RGB2Gray(uint16_t pix);
void QRCode_decode(uint16_t* rgb_image);


#ifdef __cplusplus
}
#endif
#endif