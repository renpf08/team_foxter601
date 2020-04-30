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
	unsigned letter = 0; //! ʮ�����ƴ�Сдѡ��
	signed tmpVal = 0; //! ��������ֵ��ʱ����
	unsigned uTmpVal = 0; //! ����������ֵ��ʱ����
	signed justify = 0; //! ==0:�Ҷ��룻>0:��������
	char inserChar = ' '; //! ռλ��
	int i = 0; //! ����Ϊ����������
	char c; //! ���������ȡsFormat����
	int v; //! ���������ȡpParamList�����б�����Ĳ���ֵ
	unsigned NumDigits; //! ��Ҫ���ڸ���������ƽ̨��֧�ָ������ͣ�
	unsigned FormatFlags; //! ���ڸ�ʽ�����Ƶ��ַ�
	unsigned FieldWidth = 0; //! ��ʽ����������ʾ��ռλ��
	char tmpBuf[16]; //! ��ֵת�ַ�����ʱ����
	int len = 0; //! ��ʹ��sprintfʱ������ָʾ��Ÿ�ʽ���ַ���ŵ��±�
    bool bReady = FALSE; //! �Ƿ������ݸ�ʽ�����
    char fmtBuf[M_PRINT_BUFF_SIZE] = {0}; //! ��ʽ������Ϊ�ַ���
    unsigned fmtPos = 0; //! ��ʽ�����ֵ��ַ���buf��λ��
    bool bSize = (size>0)?TRUE:FALSE; //! ���ָ������ַ������ȣ���ô��ʹδ���ַ�����β����ֹͣ��������Է��֣��������ڽ����ַ���buffer�����Զ���գ����ܻ���������һ�ζ��������
	
	do
	{
		c = *sFormat;
		sFormat++;
        
        /**
          * ����1��((c == 0u) && (size == 0)) - ��ֹ����buffer�м���0ֵʱ����������ֹ��� 
          * ����2��((bSize == TRUE) && (size == 0)) - ������buffer�ж����ַ���ʱ����ָ֤���ָ�����ȵ��ַ� 
          */
		if (((c == 0u) && (size == 0)) || ((bSize == TRUE) && (size == 0))) break; //! �������ַ�Ϊ0ֵʱ���������δ��������������(��m_nprintf��δʹ�á�%s����ʽ��ʱ����ֱ������ַ����)
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
					inserChar = ' '; //! ��ӡ�ַ�ʱ��ռλ��ʹ�ÿո��ַ����
					FieldWidth--; //! �ܻ����һ���ַ�������Ĭ��ռλ����һ
					justify = ((FormatFlags&FORMAT_FLAG_LEFT_JUSTIFY) == FORMAT_FLAG_LEFT_JUSTIFY)?1:0; //! 1 - ����룻0 - �Ҷ���
					i = 0; //! �ַ����룬��Զ�����±�Ϊ���λ��
                    bReady = TRUE;
					break;
				}
				case 'd':
				{
					v = va_arg(*pParamList, int);
					i = 0;
					tmpVal = (unsigned)v;
					inserChar = ((FormatFlags&FORMAT_FLAG_PAD_ZERO) == FORMAT_FLAG_PAD_ZERO)?'0':' '; //! ռλ��ʹ�ÿո���0�ַ����
					if(tmpVal < 0)
					{
						if(inserChar != '0')fmtBuf[fmtPos++] = '-';
                        else if(inserChar == '0') //! ���ռλ����0����Ҫ���⴦��
                        {
						    if(strBuf != 0) strBuf[len++] = '-';
						    else printf_enqueue('-');
                        }
						tmpVal = ~tmpVal+1;
					}
					do/** ע���ʽ���˳��Ϊ64���ַ� */
					{/** �����ʽ�� */
						tmpBuf[i++] = hexCharTbl[0][tmpVal%10];
						tmpVal /= 10;
					} while(tmpVal != 0);
					FieldWidth = (FieldWidth>(unsigned)i)?(FieldWidth-(unsigned)i):0; //! ����ɲ���ո��λ��
                    FieldWidth = (FieldWidth>0&&v<0)?(FieldWidth-1):FieldWidth; //! �����ֵΪ��������ռλ��Ҫ��һ
					justify = ((FormatFlags&FORMAT_FLAG_LEFT_JUSTIFY) == FORMAT_FLAG_LEFT_JUSTIFY)?1:0; //! 1 - ����룻0 - �Ҷ���
					if(i > 0) i--; //! ���浹���ʽ��ʱ��i��λ��������1���ʴ˼�һ
                    else break; //! û���κ�������Ÿ�ʽ�����
                    bReady = TRUE;
					break;
				}
				case 'u':
				{
					v = va_arg(*pParamList, int);
					i = 0;
					uTmpVal = (unsigned)v;
					inserChar = ((FormatFlags&FORMAT_FLAG_PAD_ZERO) == FORMAT_FLAG_PAD_ZERO)?'0':' '; //! ռλ��ʹ�ÿո���0�ַ����
					do/** ע���ʽ���˳��Ϊ64���ַ� */
					{/** �����ʽ�� */
						tmpBuf[i++] = hexCharTbl[0][uTmpVal%10];
						uTmpVal /= 10;
					} while(uTmpVal != 0);
					FieldWidth = (FieldWidth>(unsigned)i)?(FieldWidth-(unsigned)i):0; //! ����ɲ���ո��λ��
					justify = ((FormatFlags&FORMAT_FLAG_LEFT_JUSTIFY) == FORMAT_FLAG_LEFT_JUSTIFY)?1:0; //! 1 - ����룻0 - �Ҷ���
					if(i > 0) i--; //! ���浹���ʽ��ʱ��i��λ��������1���ʴ˼�һ
                    else break; //! û���κ�������Ÿ�ʽ�����
                    bReady = TRUE;
					break;
				}
				case 'p': //! ָ���ʮ��������ֵ������ͬ����
				case 'P':
				case 'x':
				case 'X':
				{
					v = va_arg(*pParamList, int);
					i = 0;
					uTmpVal = (unsigned)v;
					inserChar = ((FormatFlags&FORMAT_FLAG_PAD_ZERO) == FORMAT_FLAG_PAD_ZERO)?'0':' '; //! ռλ��ʹ�ÿո���0�ַ����
					if((c == 'x') || (c == 'p')) letter = 1; //! ʮ��������ֵСд
					else if((c == 'X') || (c == 'P')) letter = 0; //! ʮ��������ֵ��д
					do/** ע���ʽ���˳��Ϊ64���ַ� */
					{/** �����ʽ�� */
						tmpBuf[i++] = (unsigned)(hexCharTbl[letter][uTmpVal&0x0000000F]);
						uTmpVal >>= 4;
					} while(uTmpVal != 0);
					FieldWidth = (FieldWidth>(unsigned)i)?(FieldWidth-(unsigned)i):0; //! ����ɲ���ո��λ��
					justify = ((FormatFlags&FORMAT_FLAG_LEFT_JUSTIFY) == FORMAT_FLAG_LEFT_JUSTIFY)?1:0; //! 1 - ����룻0 - �Ҷ���
					if(i > 0) i--; //! ���浹���ʽ��ʱ��i��λ��������1���ʴ˼�һ
                    else break; //! û���κ�������Ÿ�ʽ�����
                    bReady = TRUE;
					break;
				}
				case 's':
				{
					const char * s = va_arg(*pParamList, const char *);
					justify = 0;
                    inserChar = ' '; //! �ַ�����ռλ��ǿ��Ϊ�ո�
					i = 0; //! �ַ�������
                    if(size > 0) while((unsigned)i < size) {if(fmtPos >= M_PRINT_BUFF_SIZE) break; fmtBuf[fmtPos++] = s[i++];} //! ���ָ���˰����ȸ�ʽ�������ʽ��ָ�����ȵ��ַ�
                    else while(s[i] != '\0') {if(fmtPos >= M_PRINT_BUFF_SIZE) break; fmtBuf[fmtPos++] = s[i++];} //! ���ûָ����ʽ�����ȣ��������ʽ����ֱ������0ֵ�����
                    if(size > 0) size = 0; //! ��Ҫ���˵�ָ�����ȸ�ʽ�����֮��Ҫ�ѳ���ֵsize���㣬����������Զ������ȥ
					FieldWidth = (FieldWidth>(unsigned)i)?(FieldWidth-(unsigned)i):0; //! ����ɲ���ո��λ��
					justify = ((FormatFlags&FORMAT_FLAG_LEFT_JUSTIFY) == FORMAT_FLAG_LEFT_JUSTIFY)?1:0; //! 1 - ����룻0 - �Ҷ���
					if(i > 0) i = -1; //! �����ʽ�������ǲ��õ�����Ϊ-1�������
                    else break; //! û���κ�������Ÿ�ʽ�����
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
				if((justify == 0) || ((justify == 1) && (inserChar == '0'))) //! �Ҷ��룬����߲���ո񣨻�������벢��Ϊ0���ʱ����ǿ��Ϊ�Ҷ��룬��ֻ��������0���������ұ����0����ı���ʾ��ֵ��
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
				if((justify == 1) && (inserChar != '0')) //! ����룬����ռλ����������0ʱ�����ұ߲���ո�Ϊ0�������ұ߲���ռλ������ı���ʾ��ֵ��
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
            if(size > 0) size--; //! ����Ǵ����ȵĸ�ʽ���������ÿ���һ���ַ������ȼ�һ(��m_nprintf��δʹ�á�%s����ʽ��ʱ����ֱ������ַ����)
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
    r = va_arg(ParamList, int); //! ���Է��֣������б�ĵ�һ�����������룬�˴���Ҫ��һ�ζ������൱��ȥ����һ������������
    r = va_arg(ParamList, int); //! ���Է��֣������б�ĵ�һ�����������룬�˴���Ҫ��һ�ζ������൱��ȥ����һ������������
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

