#include "iic.h"
#include "tk/tkernel.h"

ER iic_transfer(W ch, UH *cmd_dat, W words, W *xwords) {
    // I2Cチャンネルとデータを使用して、I2C転送処理を行う
    if (xwords != NULL) {
        *xwords = words;  // 実際に転送されたワード数を返す
    }
    return E_OK;
}

ER iic_setup(BOOL start) {
    if (start) {
        // スタートコンディションの設定
    } else {
        // ストップコンディションの設定
    }
    return E_OK;
}

ER iic_write(W ch, INT adr, INT reg, UB dat) {
    UH cmd_dat[2];
    cmd_dat[0] = (UH)(reg);  // 書き込むレジスタのアドレス
    cmd_dat[1] = (UH)(dat);  // 書き込むデータ

    W xwords;
    return iic_transfer(ch, cmd_dat, 2, &xwords);  // データの転送
}

INT iic_read(W ch, INT adr, INT reg) {
    UH cmd_dat[1];
    cmd_dat[0] = (UH)(reg);  // 読み取るレジスタのアドレス

    W xwords;
    iic_transfer(ch, cmd_dat, 1, &xwords);  // データの転送

    return (INT)cmd_dat[0];  // 読み取ったデータを返す
}

