# linux-ttytest
linux 下用于测试tty串口的简单应用程序，将常用的串口操作封装成API。
## int libtty_setopt(int fd, int speed, int databits, int stopbits, char parity)
串口设定
## int libtty_open
打开串口
## int libtty_close
关闭串口
读写仍然使用底层write read即可；
