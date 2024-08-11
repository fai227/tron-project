#include "iic.h"
#include <tk/tkernel.h>


ER iicxfer(W ch, UH *cmd_dat, W words, W *xwords) {
    // I2C転送処理を実装
    return E_OK;
}

ER iic_write(W ch, INT adr, INT reg, UB dat) {
    UH c[4];
    W n;
    ER err;

    c[0] = IIC_START | WR(adr);
    c[1] = IIC_SEND  | IIC_TOPDATA  | reg;
    c[2] = IIC_SEND  | IIC_LASTDATA | dat;
    c[3] = IIC_STOP;

    err = iicxfer(ch, c, sizeof(c) / sizeof(UH), &n);
    return err;
}

INT iic_read(W ch, INT adr, INT reg) {
    UH c[5];
    W n;
    ER err;
    UB data = 0;

    c[0] = IIC_START   | WR(adr);
    c[1] = IIC_SEND    | IIC_TOPDATA | IIC_LASTDATA | reg;
    c[2] = IIC_RESTART | RD(adr);
    c[3] = IIC_RECV    | IIC_TOPDATA | IIC_LASTDATA;
    c[4] = IIC_STOP;

    err = iicxfer(ch, c, sizeof(c) / sizeof(UH), &n);
    if (err == E_OK) {
        data = c[3];  // 読み取ったデータ
    }

    return data;
}
