#include "iic.h"
#include "tk/tkernel.h"  // 必要に応じて追加
#include "nrf5_iic.c"    // I2Cドライバの実装を利用
#include "iic_reg.c"     // レジスタ操作関数を利用

#define I2C_CH 1  // 使用するI2Cチャンネル（例として1を使用）

// I2Cデータ転送の実装
ER iic_transfer(W ch, UH *cmd_dat, W words, W *xwords) {
    if (ch != I2C_CH) {
        return E_PAR;  // 不正なチャンネルの場合
    }

    IICCB *cb = &iiccb[ch];  // チャンネルのコントロールブロックを取得
    cb->cmddat = cmd_dat;
    cb->end = cmd_dat + words;

    // I2C送信のための初期設定
    ER er = setup_iic_transfer(cb);
    if (er != E_OK) {
        return er;
    }

    // 転送処理を実行
    while (cb->cmddat < cb->end) {
        er = perform_iic_transfer(cb);
        if (er != E_OK) {
            return er;
        }
    }

    if (xwords != NULL) {
        *xwords = cb->cmddat - cmd_dat;  // 実際に転送したワード数を格納
    }

    return E_OK;
}

// I2Cの初期設定
ER iic_setup(BOOL start) {
    if (start) {
        return start_iic();
    } else {
        return stop_iic();
    }
}

// I2Cデバイスへの書き込み
ER iic_write(W ch, INT adr, INT reg, UB dat) {
    UH cmd_dat[2];
    cmd_dat[0] = (UH)(reg);    // レジスタアドレス
    cmd_dat[1] = (UH)(dat);    // 書き込むデータ

    W xwords;
    return iic_transfer(ch, cmd_dat, 2, &xwords);
}

// I2Cデバイスからの読み取り
INT iic_read(W ch, INT adr, INT reg) {
    UH cmd_dat[1];
    cmd_dat[0] = (UH)(reg);    // レジスタアドレス

    W xwords;
    iic_transfer(ch, cmd_dat, 1, &xwords);

    // 読み取ったデータを返す
    return (INT)cmd_dat[0];
}
