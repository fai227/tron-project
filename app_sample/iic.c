#include "iic.h"
#include <tk/tkernel.h>

// 必要なハードウェアレジスタ定義
#define ENABLE 0x500
#define FREQUENCY 0x524
#define INTENCLR 0x308
#define PSEL_SCL 0x508
#define PSEL_SDA 0x50C

// IICマクロの定義
#define IIC(cb, reg)  ((cb)->iob + (reg))

// 割り込みハンドラの定義
LOCAL void iic_inthdr(UINT dintno) {
    // 割り込み処理の実装
}

// I2Cコントローラのベースアドレスやオフセットの定義
#define I2C_BASE_ADDRESS  0x40003000  // 例: TWI0ベースアドレス
#define I2C_CHANNEL_OFFSET 0x1000     // 例: チャンネルオフセット
#define I2C_READY 0x01                // 例: I2C準備完了ビット

// その他のコード（以前のコードと同様）

// 排他制御用ロック
LOCAL FastMLock IICLock;

// 割込通知用イベントフラグ
LOCAL ID IICFlgID;

#define IICMAX 2  // 対応チャンネル数

typedef struct iiccb {
    UW iob;        // IIC レジスターアドレス
    UH *cmddat;    // コマンド現在位置
    UH *end;       // コマンド終了位置
} IICCB;
LOCAL IICCB iiccb[IICMAX];

#define IRQ(cb) INTNO((cb)->iob)  // I2C 割込番号
#define IRQ_LEVEL 3               // I2C 割込優先レベル

LOCAL struct iic_conf {
    UW iob;        // I/O ベースアドレス
    UW psel_scl;   // SCL ピン設定
    UW psel_sda;   // SDA ピン設定
    UW freq;       // クロック設定
} const iic_conf[IICMAX] = {
    { 0x40003000, 0x08, 0x10, 0x06680000 },  // TWI0 I2C_INT 400KHz
    { 0x40004000, 0x1a, 0x20, 0x06680000 }   // TWI1 I2C_EXT 400KHz
};

// IIC ドライバ起動/終了
EXPORT ER iicsetup(BOOL start) {
    ER err;
    T_CFLG cflg;
    T_DINT dint;
    W ch;
    IICCB *cb;

    if (!start) {
        // 終了処理
        err = E_OK;
        goto finish;
    }

    // 排他制御用ロック生成
    err = CreateMLock(&IICLock, "IIC_");
    if (err < E_OK) goto err_ret1;

    // 割込通知用イベントフラグの作成
    SetOBJNAME(cflg.exinf, "IIC_");
    cflg.flgatr  = TA_TFIFO | TA_WMUL;
    cflg.iflgptn = 0;
    err = tk_cre_flg(&cflg);
    if (err < E_OK) goto err_ret2;
    IICFlgID = err;

    for (ch = 0; ch < IICMAX; ++ch) {
        cb = &iiccb[ch];
        cb->iob = iic_conf[ch].iob;

        // 割込ハンドラ登録
        dint.intatr = TA_HLNG;
        dint.inthdr = iic_inthdr;
        err = tk_def_int(DINTNO(IRQ(cb)), &dint);
        if (err < E_OK) goto err_ret3;

        // TWI 初期設定
        out_w(IIC(cb, ENABLE), 5);
        out_w(IIC(cb, INTENCLR), 0xffffffff);
        out_w(IIC(cb, PSEL_SCL), iic_conf[ch].psel_scl);
        out_w(IIC(cb, PSEL_SDA), iic_conf[ch].psel_sda);
        out_w(IIC(cb, FREQUENCY), iic_conf[ch].freq);

        EnableInt(IRQ(cb), IRQ_LEVEL);
    }

    return E_OK;

  finish:
    // 終了処理
    ch = IICMAX;
  err_ret3:
    while (ch-- > 0) {
        cb = &iiccb[ch];
        MLock(&IICLock, ch);
        DisableInt(IRQ(cb));
        tk_def_int(DINTNO(IRQ(cb)), NULL);
    }
    tk_del_flg(IICFlgID);
  err_ret2:
    DeleteMLock(&IICLock);
  err_ret1:
    return err;
}

// I2C転送処理の実装
ER iicxfer(W ch, UH *cmd_dat, W words, W *xwords) {
    volatile UW *i2c_base = (UW *)(I2C_BASE_ADDRESS + ch * I2C_CHANNEL_OFFSET);
    W i;
    int timeout;

    tm_printf("Starting I2C transfer on channel %d\n", ch);

    // コマンド送信
    for (i = 0; i < words; i++) {
        tm_printf("Sending command: 0x%04x to I2C\n", cmd_dat[i]);
        *i2c_base = cmd_dat[i];

        // 送信完了待ち（簡略化のためポーリング）
        timeout = 100000;  // タイムアウト値を設定
        while (!(*i2c_base & I2C_READY) && --timeout);

        if (timeout == 0) {
            tm_printf("I2C_READY timeout occurred on command %d\n", i);
            return E_TMOUT;  // タイムアウトエラーを返す
        }
    }

    // 転送完了
    if (xwords != NULL) {
        *xwords = i;
    }

    tm_printf("I2C transfer completed successfully.\n");

    return E_OK;
}



// I2Cレジスタ書き込み
ER write_reg(W ch, INT adr, INT reg, UB dat) {
    UH c[4];
    W n;
    ER err;

    tm_printf("Preparing to write to I2C register...\n");

    c[0] = IIC_START | WR(adr);
    c[1] = IIC_SEND  | IIC_TOPDATA  | reg;
    c[2] = IIC_SEND  | IIC_LASTDATA | dat;
    c[3] = IIC_STOP;

    tm_printf("Sending I2C command...\n");

    err = iicxfer(ch, c, sizeof(c) / sizeof(UH), &n);

    if (err != E_OK) {
        tm_printf("iicxfer failed with error: %d\n", err);
    } else {
        tm_printf("I2C command sent successfully. Bytes transferred: %d\n", n);
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

// I2C書き込み関数
ER iic_write(W ch, INT adr, INT reg, UB dat) {
    return write_reg(ch, adr, reg, dat);
}

// I2C読み出し関数
INT iic_read(W ch, INT adr, INT reg) {
    return read_reg(ch, adr, reg);
}

