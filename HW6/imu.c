#include "imu.h"
#include "i2c_master_noint.h"

void imu_setup(){
    unsigned char who = 0;

    // read from IMU_WHOAMI
    i2c_master_start();
    i2c_master_send(IMU_ADDR);
    i2c_master_send(IMU_WHOAMI);
    i2c_master_restart();
    i2c_master_send(0b11010111);
    who = i2c_master_recv();
    i2c_master_ack(1);
    i2c_master_stop();
    
    if (who != 0b01101001){
        while(1){}
    }

    // init IMU_CTRL1_XL
    i2c_master_start();
    i2c_master_send(IMU_ADDR);
    i2c_master_send(IMU_CTRL1_XL);
    i2c_master_send(0x82);
    i2c_master_stop();
    
    // init IMU_CTRL2_G
    i2c_master_start();
    i2c_master_send(IMU_ADDR);
    i2c_master_send(IMU_CTRL2_G);
    i2c_master_send(0x88);
    i2c_master_stop();
    
    // init IMU_CTRL3_C
    i2c_master_start();
    i2c_master_send(IMU_ADDR);
    i2c_master_send(IMU_CTRL3_C);
    i2c_master_send(0x04);
    i2c_master_stop();
    
}

void imu_read(unsigned char reg, signed short * s_data, int len){
    unsigned char data[14];
    int i, p;
    p = 0;
    // read multiple from the imu, each data takes 2 reads so you need len*2 chars
    i2c_read_multiple(IMU_ADDR, 0b11010111, reg, data, 2*len);

    // turn the chars into the shorts
    for (i = 0; i < len; ++i) {
        s_data[i] = (data[p+1] << 8) | data[p];
        p = p+2;
    }
}
