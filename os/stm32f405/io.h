/*
 * Originally from cerf405/io.h
 * used by ../port/devi2c.c and i2c.c
 */

long	i2crecv(I2Cdev*, void*, long, ulong);
long	i2csend(I2Cdev*, void*, long, ulong);
void	i2csetup(int);
