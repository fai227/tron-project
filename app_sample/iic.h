#ifndef APP_SAMPLE_IIC_H_
#define APP_SAMPLE_IIC_H_

// 必要なdefineはここ

ER iic_transfer(W ch, UH *cmd_dat, W words, W *xwords);
ER iic_setup(BOOL start);
ER iic_write(W ch, INT adr, INT reg, UB dat);
INT iic_read(W ch, INT adr, INT reg);

#endif /* APP_SAMPLE_IIC_H_ */
