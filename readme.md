# ファイル構成
- server
  - サーバー側プログラムを設置
- vehicle
  - 自動運転車プログラムを設置
- utils
  - その他プログラムを設置

# ルール
- include/tk/typedef.hで型を参照すること
  - signed char -> B
  - unsigned short -> UH
  - など
- 変数・関数は小文字のスネークケース
  - X snakeCase
  - X Snake_Case
  - O snake_case
- 変数・関数の名前は短すぎず長すぎず
  - どれぐらい説明するかは、iic_reg.cやnrf5_iic.cなどを参照

# vehicle
- 基本的な動作は以下の3つ
  - 走行機能
  - サーバー通信機能
  - 衝突検知
- 仕様
  - usermain関数で走行機能のみ立ち上げ
  - サーバー通信機能は必要な時に走行機能から立ち上げられる
  - 衝突検知機能は定期的に立ち上げられるようにする？

# server
- 基本的な動作は以下の通り
  1. 車両の受信待ち
    - 受信次第、次のタスクを起動
  2. 経路探索
    - アルゴリズムをどうしよう
    - 単純なものでよさそう
  3. 結果を車両に返す
- 通信の仕組み上、確実に相手に届いているかが分からない可能性がある
  - キャンセルの仕組みなど詳しく設定する必要があるかも
- 最悪makecodeで実装するという手もある

# util
- iic.h
  - I2C入出力インタフェース定義
- nrf5_iic.c
  - nRF5 I2C ドライバー
- iic_reg.c
  - I2Cデバイスのレジスタ操作用の関数（micro:bitのI2C_INT用）
- maqueen.h
- maqueen.c
  - maqueenとのI2C通信用の関数
    - 参考：https://github.com/DFRobot/pxt-DFRobot_MaqueenPlus_v20/blob/26d3ceedbe27398fa450970a57e2fe4e0bbf2b69/maqueenPlusV2.ts#L185
    - これをCに書き換えるイメージでOK
  - 関数群
    - void I2C_init()
    - void control_motor(motor, direction, speed)
    - void control_motor_stop(motor)
    - void control_LED(led, switch)
    - B read_line_sensor_state(sensor)
      - Reg addr 29の方で01を返す
    - UB read_line_sensor_state(sensor)
      - Reg addr 30-38で0-255を返す
    - UB or UH read_ultrasonic()
      - 距離を返す
    - 他にも、iic_reg.cのwrite_reg、read_regのような、外部I2C通信を実装するためのwriteとread関数が必要
      - ER read_maqueen_reg(reg, dat)
      - INT write_maqueen_reg(reg)
  - 別ファイルからアクセスする際に分かりやすくすること
    - 例
      - control_motor(Left, Forward, 100)
      - read_line_sensor_state(L1)
```
typedef enum {
    Forward,
    Backward
} Direction;
```
- radio.h
- radio.c
  - 関数群
    - void radio_nit()
    - void transmit_data(検討中)
    - 戻り値の型 receive_data(検討中)

# 拡張機能
- Serial Monitor（Micro:bitのログを見れる，Tera Term不必要になるよ）