#include <types.h>
#include "user_config.h"
#include "../adapter.h"

#if USE_PRINTF_MODE
typedef char *  va_list;  
#define _INTSIZEOF(n)	((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))  
#define va_start(ap,v)	(ap = (va_list)&v + _INTSIZEOF(v) )  
#define va_arg(ap,t)	(*(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)))  
#define va_end(ap)		(ap = (va_list)0)

#define M_PRINT_BUFF_SIZE    QUEUE_BUFFER

#define FORMAT_FLAG_LEFT_JUSTIFY   (1u << 0)
#define FORMAT_FLAG_PAD_ZERO       (1u << 1)
#define FORMAT_FLAG_PRINT_SIGN     (1u << 2)
#define FORMAT_FLAG_ALTERNATE      (1u << 3)

void printf_enqueue(u8 byte);
u8 printf_dequeue(void);
void send(void);
int vprintf(char *strBuf, unsigned size, const char * sFormat, va_list * pParamList) ;
int nprintf(unsigned size, const char * sFormat, ...);
int sprintf(char *buf, const char * sFormat, ...);
int snprintf(char *buf, unsigned size, const char * sFormat, ...);
s16 print_init(void);

#define LOG_ERROR(...)    log(__FILE__, __func__, __LINE__, "<error>", __VA_ARGS__)
#define LOG_WARNING(...)  log(__FILE__, __func__, __LINE__, "<warning>", __VA_ARGS__)
#define LOG_INFO(...)     log(__FILE__, __func__, __LINE__, "<info>", __VA_ARGS__)
#define LOG_DEBUG(...)    log(__FILE__, __func__, __LINE__, "<debug>", __VA_ARGS__)

typedef struct {
	volatile u8 ring_buffer[M_PRINT_BUFF_SIZE];
    volatile u16 ring_buffer_head;
    volatile u16 ring_buffer_tail;
    volatile u16 ring_buffer_read;
}print_t;

print_t print = {
    .ring_buffer_head = 0,
    .ring_buffer_tail = 0,
    .ring_buffer_read = 0,
};

static u8 printf_init = 0;

void printf_enqueue(u8 byte)
{
    /** queue is full, discard the oldest byte */
    if(print.ring_buffer_head == ((print.ring_buffer_tail+1)%M_PRINT_BUFF_SIZE))
    {
        print.ring_buffer_head = (print.ring_buffer_head+1)%M_PRINT_BUFF_SIZE;
    }
    
    print.ring_buffer[print.ring_buffer_tail] = byte;
    print.ring_buffer_tail = (print.ring_buffer_tail+1)%M_PRINT_BUFF_SIZE;
}
u8 printf_dequeue(void)
{
    /** queue is empty */
    if(print.ring_buffer_head == print.ring_buffer_tail)
    {
        return 0;
    }
    print.ring_buffer_read = print.ring_buffer_head;
    print.ring_buffer_head = (print.ring_buffer_head+1)%M_PRINT_BUFF_SIZE;
    
    return 1;
}
void send(void)
{
    u8 buffer[M_PRINT_BUFF_SIZE] = {0};
    u16 length = 0;

    while(printf_dequeue() != 0)
    {
        buffer[length++] = print.ring_buffer[print.ring_buffer_read];
    }

    if(length > 0)
    {
        get_driver()->uart->uart_write(buffer, length);
    }
}
int vprintf(char *strBuf, unsigned size, const char * sFormat, va_list * pParamList) 
{
    char hexCharTbl[2][16] = {{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'}, 
                              {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' }};
	unsigned letter = 0; //! 十六进制大小写选择
	signed tmpVal = 0; //! 带符号数值临时变量
	unsigned uTmpVal = 0; //! 不带符号数值临时变量
	signed justify = 0; //! ==0:右对齐；>0:左对齐格数
	char inserChar = ' '; //! 占位符
	int i = 0; //! 必须为带符号类型
	char c; //! 用于逐个读取sFormat内容
	int v; //! 用于逐个读取pParamList参数列表里面的参数值
	unsigned NumDigits; //! 主要用于浮点数（该平台不支持浮点类型）
	unsigned FormatFlags; //! 用于格式化控制的字符
	unsigned FieldWidth = 0; //! 格式化后数据显示的占位数
	char tmpBuf[16]; //! 数值转字符串临时数组
	int len = 0; //! 当使用sprintf时，用于指示存放格式化字符存放的下标
    bool bReady = FALSE; //! 是否有数据格式化完成
    char fmtBuf[M_PRINT_BUFF_SIZE] = {0}; //! 格式化数字为字符串
    unsigned fmtPos = 0; //! 格式化数字到字符串buf的位置
    bool bSize = (size>0)?TRUE:FALSE; //! 如果指定输出字符串长度，那么即使未到字符串结尾，都停止输出（调试发现，蓝牙串口接收字符串buffer不会自动清空，可能会错误输出上一次多余的内容
	
	do
	{
		c = *sFormat;
		sFormat++;
        
        /**
          * 条件1：((c == 0u) && (size == 0)) - 防止数据buffer中间有0值时，被错误终止输出 
          * 条件2：((bSize == TRUE) && (size == 0)) - 当数据buffer有多余字符串时，保证指输出指定长度的字符 
          */
		if (((c == 0u) && (size == 0)) || ((bSize == TRUE) && (size == 0))) break; //! 当遇到字符为0值时，如果长度未结束，则继续输出(当m_nprintf中未使用“%s”格式化时，会直接逐个字符输出)
		if (c == '%')
		{
			FormatFlags = 0u;
			v = 1;
			do
			{
				c = *sFormat;
				switch (c)
				{/** note: 'FormatFlags' did not achive in uart pirnt mode. add by mlw at 20190804 02:32*/
					case '-': FormatFlags |= FORMAT_FLAG_LEFT_JUSTIFY; sFormat++; break;
					case '0': FormatFlags |= FORMAT_FLAG_PAD_ZERO;     sFormat++; break;
					case '+': FormatFlags |= FORMAT_FLAG_PRINT_SIGN;   sFormat++; break;
					case '#': FormatFlags |= FORMAT_FLAG_ALTERNATE;    sFormat++; break;
					default:  v = 0; break;
				}
			} while (v);
			FieldWidth = 0u;
			do
			{
				c = *sFormat;
				if ((c < '0') || (c > '9'))
				{
					break;
				}
				sFormat++;
				FieldWidth = (FieldWidth * 10u) + ((unsigned)c - '0');
			} while (1);
			NumDigits = 0u;
			c = *sFormat;
			if (c == '.')
			{
				sFormat++;
				do
				{
					c = *sFormat;
					if ((c < '0') || (c > '9'))
					{
						break;
					}
					sFormat++;
					NumDigits = NumDigits * 10u + ((unsigned)c - '0');
				} while (1);
			}
			c = *sFormat;
			do
			{
				if ((c == 'l') || (c == 'h'))
				{
					sFormat++;
					c = *sFormat;
				}
				else
				{
					break;
				}
			} while (1);
			switch (c)
			{
				case 'c':
				{
					tmpBuf[0] = (char)va_arg(*pParamList, int);
					inserChar = ' '; //! 打印字符时，占位符使用空格字符填充
					FieldWidth--; //! 总会输出一个字符，所以默认占位数减一
					justify = ((FormatFlags&FORMAT_FLAG_LEFT_JUSTIFY) == FORMAT_FLAG_LEFT_JUSTIFY)?1:0; //! 1 - 左对齐；0 - 右对齐
					i = 0; //! 字符插入，永远插入下标为零的位置
                    bReady = TRUE;
					break;
				}
				case 'd':
				{
					v = va_arg(*pParamList, int);
					i = 0;
					tmpVal = (unsigned)v;
					inserChar = ((FormatFlags&FORMAT_FLAG_PAD_ZERO) == FORMAT_FLAG_PAD_ZERO)?'0':' '; //! 占位符使用空格还是0字符填充
					if(tmpVal < 0)
					{
						if(inserChar != '0')fmtBuf[fmtPos++] = '-';
                        else if(inserChar == '0') //! 如果占位符是0，需要特殊处理
                        {
						    if(strBuf != 0) strBuf[len++] = '-';
						    else printf_enqueue('-');
                        }
						tmpVal = ~tmpVal+1;
					}
					do/** 注意格式化最长顺序为64个字符 */
					{/** 倒叙格式化 */
						tmpBuf[i++] = hexCharTbl[0][tmpVal%10];
						tmpVal /= 10;
					} while(tmpVal != 0);
					FieldWidth = (FieldWidth>(unsigned)i)?(FieldWidth-(unsigned)i):0; //! 计算可插入空格的位域
                    FieldWidth = (FieldWidth>0&&v<0)?(FieldWidth-1):FieldWidth; //! 如果数值为负数，则占位符要减一
					justify = ((FormatFlags&FORMAT_FLAG_LEFT_JUSTIFY) == FORMAT_FLAG_LEFT_JUSTIFY)?1:0; //! 1 - 左对齐；0 - 右对齐
					if(i > 0) i--; //! 上面倒叙格式化时，i的位置最后对了1，故此减一
                    else break; //! 没有任何内容序号格式化输出
                    bReady = TRUE;
					break;
				}
				case 'u':
				{
					v = va_arg(*pParamList, int);
					i = 0;
					uTmpVal = (unsigned)v;
					inserChar = ((FormatFlags&FORMAT_FLAG_PAD_ZERO) == FORMAT_FLAG_PAD_ZERO)?'0':' '; //! 占位符使用空格还是0字符填充
					do/** 注意格式化最长顺序为64个字符 */
					{/** 倒叙格式化 */
						tmpBuf[i++] = hexCharTbl[0][uTmpVal%10];
						uTmpVal /= 10;
					} while(uTmpVal != 0);
					FieldWidth = (FieldWidth>(unsigned)i)?(FieldWidth-(unsigned)i):0; //! 计算可插入空格的位域
					justify = ((FormatFlags&FORMAT_FLAG_LEFT_JUSTIFY) == FORMAT_FLAG_LEFT_JUSTIFY)?1:0; //! 1 - 左对齐；0 - 右对齐
					if(i > 0) i--; //! 上面倒叙格式化时，i的位置最后对了1，故此减一
                    else break; //! 没有任何内容序号格式化输出
                    bReady = TRUE;
					break;
				}
				case 'p': //! 指针跟十六进制数值可以相同处理
				case 'P':
				case 'x':
				case 'X':
				{
					v = va_arg(*pParamList, int);
					i = 0;
					uTmpVal = (unsigned)v;
					inserChar = ((FormatFlags&FORMAT_FLAG_PAD_ZERO) == FORMAT_FLAG_PAD_ZERO)?'0':' '; //! 占位符使用空格还是0字符填充
					if((c == 'x') || (c == 'p')) letter = 1; //! 十六进制数值小写
					else if((c == 'X') || (c == 'P')) letter = 0; //! 十六进制数值大写
					do/** 注意格式化最长顺序为64个字符 */
					{/** 倒叙格式化 */
						tmpBuf[i++] = (unsigned)(hexCharTbl[letter][uTmpVal&0x0000000F]);
						uTmpVal >>= 4;
					} while(uTmpVal != 0);
					FieldWidth = (FieldWidth>(unsigned)i)?(FieldWidth-(unsigned)i):0; //! 计算可插入空格的位域
					justify = ((FormatFlags&FORMAT_FLAG_LEFT_JUSTIFY) == FORMAT_FLAG_LEFT_JUSTIFY)?1:0; //! 1 - 左对齐；0 - 右对齐
					if(i > 0) i--; //! 上面倒叙格式化时，i的位置最后对了1，故此减一
                    else break; //! 没有任何内容序号格式化输出
                    bReady = TRUE;
					break;
				}
				case 's':
				{
					const char * s = va_arg(*pParamList, const char *);
					justify = 0;
                    inserChar = ' '; //! 字符串的占位符强制为空格
					i = 0; //! 字符串长度
                    if(size > 0) while((unsigned)i < size) {if(fmtPos >= M_PRINT_BUFF_SIZE) break; fmtBuf[fmtPos++] = s[i++];} //! 如果指定了按长度格式化，则格式化指定长度的字符
                    else while(s[i] != '\0') {if(fmtPos >= M_PRINT_BUFF_SIZE) break; fmtBuf[fmtPos++] = s[i++];} //! 如果没指定格式化长度，则逐个格式化，直到遇到0值则结束
                    if(size > 0) size = 0; //! 不要忘了当指定长度格式化完毕之后，要把长度值size清零，否则函数将永远跳不出去
					FieldWidth = (FieldWidth>(unsigned)i)?(FieldWidth-(unsigned)i):0; //! 计算可插入空格的位域
					justify = ((FormatFlags&FORMAT_FLAG_LEFT_JUSTIFY) == FORMAT_FLAG_LEFT_JUSTIFY)?1:0; //! 1 - 左对齐；0 - 右对齐
					if(i > 0) i = -1; //! 后面格式化处理是不用倒序，设为-1便会跳过
                    else break; //! 没有任何内容序号格式化输出
                    bReady = TRUE;
					break;
				}
				case '%':
				{
					if(strBuf != 0) strBuf[len++] = '%';
					else printf_enqueue('%');
					break;
					default:
					break;
				}
			}
            if(bReady == TRUE)
            {
				while(i >= 0)
				{
					fmtBuf[fmtPos++] = tmpBuf[i--];
				}
				if((justify == 0) || ((justify == 1) && (inserChar == '0'))) //! 右对齐，则左边插入空格（或者左对齐并且为0填充时，则强制为右对齐，即只在左边填充0，不能在右边填充0，会改变显示数值）
				{
					for(i = 0; i < (int)FieldWidth; i++)
					{
						if(strBuf != 0) strBuf[len++] = inserChar;
						else printf_enqueue(inserChar);
					}
				}
				i = 0;
				while(i < (int)fmtPos)
				{
					if(strBuf != 0) strBuf[len++] = fmtBuf[i++];
					else printf_enqueue(fmtBuf[i++]);
				}
				if((justify == 1) && (inserChar != '0')) //! 左对齐，并且占位符不是数字0时，则右边插入空格（为0不能在右边插入占位符，会改变显示数值）
				{
					for(i = 0; i < (int)FieldWidth; i++)
					{
						if(strBuf != 0) strBuf[len++] = inserChar;
						else printf_enqueue(inserChar);
					}
				}
                
                //for(i = 0; i < (int)sizeof(fmtBuf); i++) fmtBuf[i] = '\0';
                //for(i = 0; i < (int)sizeof(tmpBuf); i++) tmpBuf[i] = '\0';
                fmtPos =  0;
                //len = 0;
                i = 0;
                FieldWidth = 0;
                bReady = FALSE;
            }
			sFormat++;
		}
		else
		{
            if(size > 0) size--; //! 如果是带长度的格式化输出，则每输出一个字符，长度减一(当m_nprintf中未使用“%s”格式化时，会直接逐个字符输出)
			if(strBuf != 0) strBuf[len++] = c;
			else printf_enqueue(c);
		}
	} while(1);

	return len;
}
int printf(const char * sFormat, ...)
{
    int r = 0;
    va_list ParamList;

    if(printf_init == 0) {
        return r;
    }

    va_start(ParamList, sFormat);
    r = va_arg(ParamList, int); //! 测试发现，参数列表的第一个参数是乱码，此处需要做一次读操作相当于去掉第一个参数！！！
    r = va_arg(ParamList, int); //! 测试发现，参数列表的第一个参数是乱码，此处需要做一次读操作相当于去掉第一个参数！！！
    r = vprintf(0, 0, sFormat, &ParamList);
    va_end(ParamList);
    send();
    return r;
}
int nprintf(unsigned size, const char * sFormat, ...)
{
    int r = 0;
    va_list ParamList;

    if(printf_init == 0) {
        return r;
    }

    va_start(ParamList, sFormat);
    r = vprintf(0, size, sFormat, &ParamList);
    va_end(ParamList);
    return r;
}
int sprintf(char *buf, const char * sFormat, ...)
{
    int r = 0;
    va_list ParamList;

    if(printf_init == 0) {
        return r;
    }

    va_start(ParamList, sFormat);
    r = vprintf(buf, 0, sFormat, &ParamList);
    va_end(ParamList);
    buf[r] = '\0';
    return r;
}
int snprintf(char *buf, unsigned size, const char * sFormat, ...)
{
    int r = 0;
    va_list ParamList;

    if(printf_init == 0) {
        return r;
    }

    va_start(ParamList, sFormat);
    r = vprintf(buf, size, sFormat, &ParamList);
    va_end(ParamList);
    buf[r] = '\0';
    return r;
}
int log(const char* file, const char* func, unsigned line, const char* level, const char * sFormat, ...)
{
    int r = 0;
    va_list ParamList;

    if(printf_init == 0) {
        return r;
    }
    
    printf("%-30s%-40s%6d%10s:", file, func, line, level);
    va_start(ParamList, sFormat);
    r = vprintf(0, 0, sFormat, &ParamList);
    va_end(ParamList);
    send();
    return r;
}
s16 print_init(void)
{
    printf_init = 1;

    return 0;
}
#else
int printf(const char * sFormat, ...)
{
    return 0;
}
int log(const char* file, const char* func, unsigned line, const char* level, const char * sFormat, ...)
{
    return 0;
}
#endif

