
#ifndef _USER_DEF_H
#define _USER_DEF_H

//位操作定义
#define TESTBIT(a,b)  ((a)&(1<<(b)))
#define SETBIT(a,b)  ((a)|=(1<<(b)))
#define CLRBIT(a,b)  ((a)&=~(1<<(b))) 

#endif
