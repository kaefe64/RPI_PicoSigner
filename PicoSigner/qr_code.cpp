/*


https://github.com/ricmoo/QRCode/tree/master

A QR code is composed of many little squares, called modules, which represent encoded data, with additional error correction 
(allowing partially damaged QR codes to still be read).
The version of a QR code is a number between 1 and 40 (inclusive), which indicates the size of the QR code. The width and height of a QR code 
are always equal (it is square) and are equal to 4 * version + 17.
The level of error correction is a number between 0 and 3 (inclusive), or can be one of the symbolic names ECC_LOW, ECC_MEDIUM, ECC_QUARTILE and ECC_HIGH. 
Higher levels of error correction sacrifice data capacity, but allow a larger portion of the QR code to be damaged or unreadable.
The mode of a QR code is determined by the data being encoded. Each mode is encoded internally using a compact representation, so lower modes can contain more data.
1   21 x 21   byte mode: 17, 14, 11, 7
2   25 x 25   byte mode: 32, 26, 20, 14
3   29 x 29   byte mode: 53, 42, 32, 24


https://www.hackster.io/mdraber/generate-qr-codes-with-arduino-on-oled-display-53c074

Library:  QRCode by Richard Moore


Decode QR Code :  https://github.com/dlbeer/quirc

*/

#include "Arduino.h"
#include "qr_code.h"
#include <qrcode.h>   //Library:  QRCode by Richard Moore
#include "TFT_eSPI.h"
#include "display_tft.h"


/*
uint16_t qrcode_getBufferSize(uint8_t version);

int8_t qrcode_initText(QRCode *qrcode, uint8_t *modules, uint8_t version, uint8_t ecc, const char *data);
int8_t qrcode_initBytes(QRCode *qrcode, uint8_t *modules, uint8_t version, uint8_t ecc, uint8_t *data, uint16_t length);

bool qrcode_getModule(QRCode *qrcode, uint8_t x, uint8_t y);

*/


//============================================================================
void QRCode_generate(uint8_t *in_16x8bits) 
{
  QRCode qrcode;  // Create a QR code object
  
  //uint16_t Word_byte[WORDS_NUM];  //11 bits / word

  // Define the size of the QR code (1-40, higher means bigger size)
  uint8_t qrcodeData[qrcode_getBufferSize(2)];
  qrcode_initBytes(&qrcode, qrcodeData, 2, 0, in_16x8bits, 16);

  // Calculate the scale factor
  int scale = min(TFT_WIDTH / qrcode.size, TFT_HEIGHT / qrcode.size);
  
  // Calculate horizontal shift
  int shiftX = (TFT_WIDTH - qrcode.size*scale)/2;
  
  // Calculate horizontal shift
  int shiftY = (TFT_HEIGHT - qrcode.size*scale)/2;

  // Draw the QR code on the display
  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        //display.fillRect(shiftX+x * scale, shiftY + y*scale, scale, scale, WHITE);
        // fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color);
        tft.fillRect(shiftX+x * scale, shiftY + y*scale, scale, scale, bk_color);
      }
    }
  }


}





/*
****  Diferenca = 4 bits chksum
12 valores x 11bits = 132 bits
16 valores de 8bits = 128 bits
*/
#define ENTRADA_11BITS_NUM  12
#define SAIDA_8BITS_NUM  16
//============================================================================
void pack_12x11bit_to_16x8bit(const uint16_t* entrada_11bits, uint8_t* saida_8bits) {
    
    // O buffer temporário precisa ser grande o suficiente para guardar o maior valor
    // possível antes de ser transferido. Um uint32_t é suficiente.
    uint32_t bit_buffer = 0;
    int bits_em_buffer = 0;
    int saida_idx = 0;

    // A quantidade de bits de entrada é 12 valores x 11 bits = 132 bits.
    // A quantidade de bits de saída é 16 valores x 8 bits = 128 bits.
    // O loop deve processar apenas os 128 bits que cabem na saída.
    for (int i = 0; i < ENTRADA_11BITS_NUM; ++i) {
        // Adiciona os 11 bits do valor de entrada no buffer temporário.
        bit_buffer = (bit_buffer << 11) | (entrada_11bits[i] & 0x07FF); // 0x07FF = 11 bits
        bits_em_buffer += 11;

        // Enquanto houver pelo menos 8 bits no buffer, transfere para a saída.
        while (bits_em_buffer >= 8) {
            // Se o índice de saída já chegou ao limite (16 bytes = 128 bits),
            // sai do loop para não escrever em posições inválidas.
            if (saida_idx >= SAIDA_8BITS_NUM) {
                return;
            }
            
            // Pega os 8 bits mais significativos do buffer.
            uint8_t byte_para_escrever = (bit_buffer >> (bits_em_buffer - 8)) & 0xFF;
            
            // Armazena o byte na posição correta do vetor de saída.
            saida_8bits[saida_idx++] = byte_para_escrever;
            
            // Remove os 8 bits que já foram processados do buffer.
            bits_em_buffer -= 8;
        }
    }
}


//============================================================================
void QRCode_gen_test() 
{
  // Vetor de entrada com 12 valores de 11 bits (max 2047)
  uint16_t valores_11bit[12] = {1690,	1374,	950,	1377,	608,	1985,	1195,	1370,	437,	1009,	1697,	272 };
  
  // Vetor de saída com 16 bytes
  uint8_t valores_8bit[16];

  pack_12x11bit_to_16x8bit(valores_11bit, valores_8bit);

  Serial.print("Valores de 11 bits (Entrada): ");
  for (int i = 0; i < 12; ++i) 
  {
      Serial.print(valores_11bit[i]);
      Serial.print(" ");
  }
  Serial.println();

  Serial.print("\nValores de 8 bits (Saida): ");
  for (int i = 0; i < 16; ++i) 
  {
      if (valores_8bit[i] < 0x10)  Serial.print("0");
      Serial.print(valores_8bit[i], HEX);
      Serial.print(" ");
  }
  Serial.println();

  QRCode_generate(valores_8bit);
}


#include "quirc.h"

struct quirc* qr;
struct quirc_code code;
struct quirc_data data;
//uint8_t *image;

//============================================================================
void QRCode_init(uint8_t* image) 
{
  qr = quirc_new();   //allocate the object quirc
  if (qr == NULL) 
  {
    Serial.println("can't create quirc object");
  }
  else if (quirc_resize(qr, TFT_HEIGHT, TFT_WIDTH, image) < 0)  //allocate the image buffer
  {
    Serial.println("Failed to allocate video memory");
  }
  else
  {
    Serial.println("Quirc memory allocate OK");
  }
}


/*



#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// d = SQR( (x2-x1)^2 + (y2-y1)^2 + (z2-z1)^2 )


typedef struct {
    int x, y, z;
} Ponto;

// Flood-fill (BFS) para zerar todos pontos do grupo do máximo
void flood_fill(uint16_t v[XTAM][YTAM][ZTAM], int x, int y, int z) {
              Ponto queue[XTAM*YTAM*ZTAM];  //guarda a lista de pontos ativos para visitar (tamanho = max pontos)
              int qout = 0;   //aponta para o "próximo ponto a processar"
              int qin = 0;    //aponta para o "próximo espaço livre" para entrada na fila.
              uint16_t v_min = v[x][y][z]>>1;   // v_max/2 = valor minimo para ser considerado do grupo
 
              queue[qin++] = (Ponto){x, y, z};  //Adiciona o ponto inicial no fim da fila
              v[x][y][z] = 0;  //zera valor para nao contar mais como alto
 
              // Enquanto a fila não estiver vazia:
              while (qout < qin) {
                            Ponto p = queue[qout++];  //Pega um ponto e tira da fila
       
        for (int dx = -1; dx <= 1; dx++)  //olha em volta do ponto
        for (int dy = -1; dy <= 1; dy++)
        for (int dz = -1; dz <= 1; dz++) {
            if (dx == 0 && dy == 0 && dz == 0) continue; // ignora o próprio ponto
            int nx = p.x + dx;
                                          int ny = p.y + dy;
                                          int nz = p.z + dz;
            if ((nx >= 0) && (nx < XTAM) &&   //se dentro da matriz
                                              (ny >= 0) && (ny < YTAM) &&
                                                         (nz >= 0) && (nz < ZTAM)) {
                if (v[nx][ny][nz] >= v_min) {   //se valor alto = pertence ao grupo
                    queue[qin++] = (Ponto){nx, ny, nz};  //achou outro ponto do grupo
                    v[nx][ny][nz] = 0;  //zera valor para nao contar mais como alto
                }
            }
        }
              }
}
 

void encontra_segundo_maior(uint16_t v[XTAM][YTAM][ZTAM], int x1, int y1, int z1,
                            int* sx, int* sy, int* sz) {
 
}
 
int main() {
    uint16_t v[XTAM][YTAM][ZTAM] = {0};
    int x1,y1,z1;
    int x2,y2,z2;
 
   
              uint16_t max = 0;                                                                   
    for (int x=0; x<XTAM; x++)
    for (int y=0; y<YTAM; y++)
    for (int z=0; z<ZTAM; z++) {
        if (v[x][y][z] > max) {
            max = v[x][y][z];
            x1 = x; y1 = y; z1 = z;  //max1 = primeiro ponto de max
        }
    }                                                                                             
                                                                                                  
    flood_fill(v, x1, y1, z1);
 
    max = 0;
    for (int x=0; x<XTAM; x++)
    for (int y=0; y<YTAM; y++)
    for (int z=0; z<ZTAM; z++) {
        if (v[x][y][z] > max) {
            max = v[x][y][z];
            x2 = x; y2 = y; z2 = z;  //max2 = segundo ponto de max
        }
    }
 
 
 
 
    //varrer todos os pontos da imagem
              //calcular x,y,z a partir do RGB
              //calcular a distancia do ponto max1
              //dist1=abs(x-x1)+abs(y-y1)+abs(z-z1)
              //calcular a distancia do ponto max2
              //dist2=abs(x-x2)+abs(y-y2)+abs(z-z2)
              //comparar as distâncias
              //se mais perto do ponto 1 =0
              //se mais perto do ponto 2 =1
 
 
 
    return 0;
}
*/

#define RGBMAX  0x1f
#define RGBSHIFT  2
#define XTAM ((RGBMAX+1)>>RGBSHIFT)   // (31+1)/4 = 8
#define YTAM ((RGBMAX+1)>>RGBSHIFT)   // (31+1)/4 = 8
#define ZTAM ((RGBMAX+1)>>RGBSHIFT)   // (31+1)/4 = 8
#define abs(x) ((x)>0?(x):-(x))

//============================================================================
void RGB2Gray_2(uint8_t *image, int w, int h, uint16_t* rgb_image)
{
  uint16_t color_clusters[XTAM][YTAM][ZTAM]={0};  //matrix of poitns clusters
  //uint16_t i_max, j_max, k_max;
  int r_max,g_max,b_max;

  //consider the RGB colors as a 3D point
  //separate the RGB points on clusters
  for (int n = 0; n < w*h; n++)   // about 36-44ms
  {
    uint16_t pix = (image[n] << 8) | (image[n] >> 8);;

    // low value on rgb = dark
    // high value on rgb = bright
    //bbbb brrr rrrg gggg
    uint8_t b = (pix>>11) & 0x1F;
    uint8_t r = (pix>>5) & 0x3F;
    if(r>RGBMAX) r=RGBMAX;
    uint8_t g = pix & 0x1F;

    color_clusters[r>>RGBSHIFT][g>>RGBSHIFT][b>>RGBSHIFT]++; //how many points are in that color cluster
  }
 
  //search for the cluster with max number of points (color more present)
  uint16_t max = 0;                                                                   
  for (int x=0; x<XTAM; x++)
  for (int y=0; y<YTAM; y++)
  for (int z=0; z<ZTAM; z++) {
      if (color_clusters[x][y][z] > max) {
          max = color_clusters[x][y][z];
          r_max = x; g_max = y; b_max = z;  //cluster with max number of points
      }
  } 
  r_max<<=RGBSHIFT; g_max<<=RGBSHIFT; b_max<<=RGBSHIFT;  //RGB with max number of points

  for (int n = 0; n < w*h; n++)   // about 36-44ms
  {
    uint16_t pix = (image[n] << 8) | (image[n] >> 8);

    // low value on rgb = dark
    // high value on rgb = bright
    //bbbb brrr rrrg gggg
    uint8_t b = (pix>>11) & 0x1F;
    uint8_t r = (pix>>5) & 0x3F;
    if(r>0x1f) r=0x1f;
    uint8_t g = pix & 0x1F;

    //aprox. distance from the cluster with more points
    image[n] =  abs(r_max-r) + abs(g_max-g) + abs(b_max-b);
    //max = 0x1f = 31 * 3 = 93   fits on uint8
  }

}



//============================================================================
void RGB2Gray(uint8_t *image, int w, int h, uint16_t* rgb_image)
{
  for (int n = 0; n < w*h; n++)   // about 36-44ms
  {
    uint16_t pix_inv = rgb_image[n]; //put the image_gray over the image_rgb
  
    // low value on rgb = dark
    // high value on rgb = bright
    //rrrrrggggggbbbbb (RGB565)  turns gggbbbbb rrrrrggg  on uint16_t
    //frame_ov7670[n] &= 0x00F8;   //red
    //frame_ov7670[n] &= 0x1f00;   //blue
    //frame_ov7670[n] &= 0xE007;   //green
    uint16_t pix = (pix_inv << 8) | (pix_inv >> 8);

    //extrai componentes rgb 
    uint8_t b = (pix>>11) & 0x1F;
    uint8_t r = (pix>>5) & 0x3F;
    uint8_t g = pix & 0x1F;

    //expande componentes para 8 bits
    uint16_t r8 = (r<<3) | (r>>2);
    uint16_t g8 = (g<<2) | (g>>4);
    uint16_t b8 = (b<<3) | (b>>2);
    
    //calcula cinza perceptivo
    image[n] =  ((r8 * 77) + (g8 * 150) + (b8 * 29)) >> 8;
  }
}



//============================================================================
void QRCode_decode(uint16_t* rgb_image) 
{
  int w, h;
  uint8_t *image;
  //uint32_t t1;

//t1 = millis();
  image = quirc_begin(qr, &w, &h);   // about 0ms
//Serial.print("*****   quirc_begin "); Serial.println(millis()-t1);

//t1 = millis();
  //for (int n = 0; n < w*h; n++)   // about 36-44ms
  //{
  //  image[n] = RGB2Gray(rgb_image[n]); //put the image_gray over the image_rgb
  //}
  RGB2Gray(image, w, h, rgb_image);
  //RGB2Gray_2(image, w, h, rgb_image);
//Serial.print("RGB2Gray "); Serial.println(millis()-t1);

//t1 = millis();
  quirc_end(qr);  //about 5000-7600 when get the qr code, otherwise 51-100ms
//Serial.print("quirc_end "); Serial.println(millis()-t1);

  int num_codes = quirc_count(qr);
  //debug serial (removido)
  //if(num_codes>0)
  //{
  //  Serial.print("Quirc   num_codes = ");
  //  Serial.println(num_codes);
  //}

  for (int i = 0; i < num_codes; i++) {
    quirc_decode_error_t err;
//t1 = millis();
    quirc_extract(qr, i, &code);  //about 23ms
//Serial.print("quirc_extract "); Serial.println(millis()-t1);
/*    
    Serial.print("Corners: ");
    for(uint16_t i=0; i<4; i++)
    {
      Serial.print(code.corners[i].x);
      Serial.print(" ");
      Serial.print(code.corners[i].y);
      Serial.print("   ");
    }
    Serial.println(" ");
*/
    tft.drawLine(code.corners[0].x, code.corners[0].y, code.corners[1].x, code.corners[1].y, 0xF800);
    tft.drawLine(code.corners[1].x, code.corners[1].y, code.corners[2].x, code.corners[2].y, 0xF800);
    tft.drawLine(code.corners[2].x, code.corners[2].y, code.corners[3].x, code.corners[3].y, 0xF800);
    tft.drawLine(code.corners[3].x, code.corners[3].y, code.corners[0].x, code.corners[0].y, 0xF800);

//t1 = millis();
    err = quirc_decode(&code, &data);  //about 3ms
//Serial.print("quirc_decode "); Serial.println(millis()-t1);
    if (err)
    {
      Serial.print("DECODE FAILED: ");
      Serial.println(quirc_strerror(err));
    }
    else
    {
      //debug serial (removido): envio das palavras lidas via QR
      //Serial.print("Data: ");
      //for(uint16_t i=0; i<40; i++)
      //{
      //  if(data.payload[i]==0)
      //    break;
      //  Serial.print(data.payload[i]);
      //  Serial.print(" ");
      //}
      //Serial.println(" ");
    }
  }
}


//============================================================================
void QRCode_deco_test() 
{
  
}


