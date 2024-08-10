#ifndef APP_SAMPLE_IIC_H_
#define APP_SAMPLE_IIC_H_

#include <tk/typedef.h>

// I2C操作に関するマクロ定義
#define IIC_START     0x01
#define IIC_SEND      0x02
#define IIC_TOPDATA   0x04
#define IIC_LASTDATA  0x08
#define IIC_STOP      0x10
#define IIC_RESTART   0x20
#define IIC_RECV      0x40

ER iic_transfer(W ch, UH *cmd_dat, W words, W *xwords);
ER iic_setup(BOOL start);
ER iic_write(W ch, INT adr, INT reg, UB dat);
INT iic_read(W ch, INT adr, INT reg);

// iicxfer関数プロトタイプ
ER iicxfer(W ch, UH *cmd_dat, W words, W *xwords);

#endif /* APP_SAMPLE_IIC_H_ */
