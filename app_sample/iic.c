#include "iic.h"
#include <tk/tkernel.h>
#define ENABLE       0x500  // I2Cの有効化レジスタ
#define EVENTS_ERROR 0x124  // エラーレジスタ
#define FREQUENCY    0x524  // クロック周波数設定レジスタ
#define INTENCLR     0x308  // 割り込み無効化レジスタ
#define PSEL_SCL     0x508  // SCLピン設定レジスタ
#define PSEL_SDA     0x50C  // SDAピン設定レジスタ


// I2Cコントローラのベースアドレスやオフセットの定義
#define I2C_BASE_ADDRESS  0x40003000  // 例: TWI0ベースアドレス
#define I2C_CHANNEL_OFFSET 0x1000     // 例: チャンネルオフセット
#define I2C_READY 0x01                // 例: I2C準備完了ビット

// I2C転送処理の実装
ER iicxfer(W ch, UH *cmd_dat, W words, W *xwords) {
    tm_printf("Starting iicxfer with ch: %d, words: %d\n", ch, words);
    volatile UW *i2c_base = (UW *)(I2C_BASE_ADDRESS + ch * I2C_CHANNEL_OFFSET);
    W i;
    int timeout;

    // コマンド送信
    for (i = 0; i < words; i++) {
        tm_printf("Sending command: %04x\n", cmd_dat[i]);
        *i2c_base = cmd_dat[i];

        // 送信完了待ち（タイムアウト処理付き）
        timeout = 500000; // 例えば100000カウントでタイムアウト
        while (!(*i2c_base & I2C_READY)) {
            if (--timeout == 0) {
                tm_printf("I2C_READY timeout occurred.\n");
                return E_TMOUT; // タイムアウトエラーを返す
            }
        }
    }

    // 転送完了
    if (xwords != NULL) {
        *xwords = i;
    }

    tm_printf("iicxfer completed.\n");
    return E_OK;
}




// I2Cレジスタ書き込み
ER write_reg(W ch, INT adr, INT reg, UB dat) {
    UH c[4];
    W n;
    ER err;

    tm_printf("Starting write_reg with adr: %d, reg: %d, dat: %d\n", adr, reg, dat);

    c[0] = IIC_START | WR(adr);
    c[1] = IIC_SEND  | IIC_TOPDATA  | reg;
    c[2] = IIC_SEND  | IIC_LASTDATA | dat;
    c[3] = IIC_STOP;

    err = iicxfer(ch, c, sizeof(c) / sizeof(UH), &n);
    if (err != E_OK) {
        tm_printf("write_reg failed with error: %d\n", err);
    } else {
        tm_printf("write_reg successful.\n");
    }

    return err;
}


// I2Cレジスタ読み出し
INT read_reg(W ch, INT adr, INT reg) {
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

// I2C初期化
ER iicsetup(BOOL start) {
    if (start) {
        tm_printf("Starting I2C setup...\n");

        // TWI 初期設定
        out_w(I2C_BASE_ADDRESS + ENABLE, 5);  // I2Cを有効化
        out_w(I2C_BASE_ADDRESS + PSEL_SCL, 0x08);  // SCLピンの設定
        out_w(I2C_BASE_ADDRESS + PSEL_SDA, 0x10);  // SDAピンの設定
        out_w(I2C_BASE_ADDRESS + FREQUENCY, 0x06680000);  // I2C周波数を100kHzに設定
        out_w(I2C_BASE_ADDRESS + INTENCLR, 0xffffffff);  // すべての割り込みを無効化

        tm_printf("I2C setup completed.\n");

        // バスがビジーでないことを確認
        if (in_w(I2C_BASE_ADDRESS + EVENTS_ERROR) != 0) {
            tm_printf("I2C bus error detected.\n");
            return E_IO;
        }
    }
    return E_OK;
}


// I2C書き込み関数
ER iic_write(W ch, INT adr, INT reg, UB dat) {
    return write_reg(ch, adr, reg, dat);
}

// I2C読み出し関数
INT iic_read(W ch, INT adr, INT reg) {
    return read_reg(ch, adr, reg);
}
