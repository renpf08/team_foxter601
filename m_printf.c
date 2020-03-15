/**
*  @file    m_printf.c
*  @author  maliwen
*  @date    2020/02/28
*  @history 1.0.0.0(�汾��)
*           1.0.0.0: Creat              2020/02/28
*/
#include <uart.h>
#include "m_printf.h"
#include "m_timer.h"

#define FORMAT_FLAG_LEFT_JUSTIFY   (1u << 0)
#define FORMAT_FLAG_PAD_ZERO       (1u << 1)
#define FORMAT_FLAG_PRINT_SIGN     (1u << 2)
#define FORMAT_FLAG_ALTERNATE      (1u << 3)

void m_uart_out(char c);
int m_vprintf(char *strBuf, unsigned size, const char * sFormat, va_list * pParamList);

/**
* @brief ��ʹ��printfʱ��m_vprintfÿ��ʽ��һ���ַ������һ���ַ���Ҳ����ȫ����ʽ����һ��buffer��֮������������˴��漰��Ч�����⣬���ڿ��Ը���ʵ����������Ż���
* @param [in] c - ��ʽ�����������ַ�
* @param [out] none
* @return none
* @data 2020/03/03 17:07
* @author maliwen
* @note �˴�ʹ�øú������Ƿ��㲻ͬƽ̨����ֲ
*/
void m_uart_out(char c)
{
    UartWriteBlocking((const char*)&c, 1);
}

/**
* @brief ���ݸ�ʽ��ȫ����
* @param [in] strBuf - �ǿգ������ݸ�ʽ����ش����������no use��sFormat - ��ʽ���ַ�����... - ��ʽ����ֵ�б�
* @param [out] none
* @return dont care
* @data 2020/03/03 17:03
* @author maliwen
* @note ��bug������ӡ�ַ���Ϊ��ֵ�ַ��������ַ����м�����ֵ0x25��ʮ����37��ʱ���ᱻʶ��ɸ�ʽ��
*       �Ʒ���%�����������������Ŀǰ��δ�Դ�bug�����޸��������ⷢ����20200314 23:47��
*/
//! ע�����Դ��ڴ�ӡ����ʱ�����ܲ�����������m_vprintf��������ʱ���Գ��԰�hexCharTbl����Ϊȫ�ֱ���
int m_vprintf(char *strBuf, unsigned size, const char * sFormat, va_list * pParamList) 
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
						    else m_uart_out('-');
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
					else m_uart_out('%');
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
						else m_uart_out(inserChar);
					}
				}
				i = 0;
				while(i < (int)fmtPos)
				{
					if(strBuf != 0) strBuf[len++] = fmtBuf[i++];
					else m_uart_out(fmtBuf[i++]);
				}
				if((justify == 1) && (inserChar != '0')) //! ����룬����ռλ����������0ʱ�����ұ߲���ո�Ϊ0�������ұ߲���ռλ������ı���ʾ��ֵ��
				{
					for(i = 0; i < (int)FieldWidth; i++)
					{
						if(strBuf != 0) strBuf[len++] = inserChar;
						else m_uart_out(inserChar);
					}
				}
                
                for(i = 0; i < (int)sizeof(fmtBuf); i++) fmtBuf[i] = '\0';
                for(i = 0; i < (int)sizeof(tmpBuf); i++) tmpBuf[i] = '\0';
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
			else m_uart_out(c);
		}
	} while(1);

	return len;
}

/**
* @brief ��ʽ���ַ����������������
* @param [in] sFormat - ��ʽ���ַ�����... - ��ʽ����ֵ�б�
* @param [out] none
* @return dont care
* @data 2020/03/03 17:01
* @author maliwen
* @note none
*/
int m_printf(const char * sFormat, ...)
{
    int r = 0;
    va_list ParamList;

    va_start(ParamList, sFormat);
    r = va_arg(ParamList, int); //! ���Է��֣������б�ĵ�һ�����������룬�˴���Ҫ��һ�ζ������൱��ȥ����һ������������
    r = m_vprintf(0, 0, sFormat, &ParamList);
    va_end(ParamList);
    return r;
}

/**
* @brief ��ָ�����ȵĸ�ʽ���ַ����������������
* @param [in] size - ָ����ʽ���Ĳ������ȣ�sFormat - ��ʽ���ַ�����... - ��ʽ����ֵ�б�
* @param [out] none
* @return dont care
* @data 2020/03/05 09:19
* @author maliwen
* @note ����size����ָ�����ȵĸ�ʽ���ַ�����������Ա�֤����ַ����м���0ֵʱ���ᱻ�ض�
*       �������ʽ��������ַ����п��ܰ���0ֵʱ����Ҫʹ�ô˺���(ָ����ʽ���ĳ���)
*/
int m_nprintf(unsigned size, const char * sFormat, ...)
{
    int r = 0;
    va_list ParamList;

    va_start(ParamList, sFormat);
    //r = va_arg(ParamList, int); //! ���Է��֣������б�ĵ�һ�����������룬�˴���Ҫ��һ�ζ������൱��ȥ����һ������������
    r = m_vprintf(0, size, sFormat, &ParamList);
    va_end(ParamList);
    return r;
}

/**
* @brief ��ʽ���ַ�����ָ��buffer���������buffer
* @param [in] buf - ��ʽ������ַ������ڸ�buffer���������
* @param [out] buf
* @return dont care
* @data 2020/03/03 16:59
* @author maliwen
* @note none
*/
int m_sprintf(char *buf, const char * sFormat, ...)
{
    int r = 0;
    va_list ParamList;

    va_start(ParamList, sFormat);
    //r = va_arg(ParamList, int); //! ���Է��֣������б�ĵ�һ�����������룬�˴���Ҫ��һ�ζ������൱��ȥ����һ������������
    r = m_vprintf(buf, 0, sFormat, &ParamList);
    va_end(ParamList);
    buf[r] = '\0';
    return r;
}

/**
* @brief ��ָ�����ȵĸ�ʽ���ַ�����ָ��buffer���������buffer
* @param [in] size - ָ����ʽ���Ĳ������ȣ�buf - ��ʽ������ַ������ڸ�buffer���������
* @param [out] buf
* @return dont care
* @data 2020/03/05 10:57
* @author maliwen
* @note ����size����ָ�����ȵĸ�ʽ���ַ�����������Ա�֤����ַ����м���0ֵʱ���ᱻ�ض�
*       �������ʽ��������ַ����п��ܰ���0ֵʱ����Ҫʹ�ô˺���(ָ����ʽ���ĳ���)
*/
int m_snprintf(char *buf, unsigned size, const char * sFormat, ...)
{
    int r = 0;
    va_list ParamList;

    va_start(ParamList, sFormat);
    //r = va_arg(ParamList, int); //! ���Է��֣������б�ĵ�һ�����������룬�˴���Ҫ��һ�ζ������൱��ȥ����һ������������
    r = m_vprintf(buf, size, sFormat, &ParamList);
    va_end(ParamList);
    buf[r] = '\0';
    return r;
}

/**
* @brief ����־�ķ�ʽ��������Ϣ��ʽ����ӡ�����ڣ���Ҫ�������һЩ������־�����ļ������������������Լ������Ϣ��������ٶ�λ�Լ��������
* @param [in] file - �����ļ�����func - ���ں�������line - �����кţ� level - ���Եȼ�
* @param [out] dont care
* @return dont care
* @data 2020/03/03 17:11
* @author maliwen
* @note �˺������������������Ϣ�϶࣬��ͨ���ڴ�ӡ���ʱ��������ã�����ʹ��m_printf()��
*/
int m_log(const char* file, const char* func, unsigned line, const char* level, const char * sFormat, ...)
{
    int r = 0;
    va_list ParamList;
    char preStr[64] = {0};
    unsigned len = 0;
    time_t* time = m_get_time();
    
    do
    {
        if(len >= sizeof(preStr)) break;
        preStr[len++] = *file;
        if(*file == '\\')
        {
            file += len;
            len = 0;
        }
    } while(*++file != '\0');
    preStr[len] = '\0';
    
    m_printf("<%02d:%02d:%02d> %-30s%-40s%6d%10s:", time->hour, time->minute, time->second, preStr, func, line, level);
    va_start(ParamList, sFormat);
    //r = va_arg(ParamList, int); //! ���Է��֣������б�ĵ�һ�����������룬�˴���Ҫ��һ�ζ������൱��ȥ����һ������������
    r = m_vprintf(0, 0, sFormat, &ParamList);
    va_end(ParamList);
    return r;
}

/**
* @brief �������Ը�ʽ������Ƿ��������Ľ��
* @param [in] none
* @param [out] none
* @return none
* @data 2020/03/03 16:57
* @author maliwen
* @note none
*/
void m_printf_test(void)
{
    #if 0
    //--------------------------------------------------------------------------
    //������ַ����м�����ֵ0x25ʱ���ᱻ���ɸ�ʽ���Ʒ�%�������ᵼ�����������bug�ݲ��޸�
    //mlw 20200315 00:11
    m_printf("\x12\x25\x35\x56\x78\x0d\x0a");
    //--------------------------------------------------------------------------
    //ʮ�����ƴ����ȿ��Ƶĸ�ʽ����������ڲ����ַ����м���0ֵ��������
    char buf[16] = {"\x12\x34\x00\x56\x00\x78\x00\x00\x91\x00\x00\x00"};
    m_nprintf(12, buf); //! ָ�����Ȳ��Ҳ�����ʽ���Ƶĸ�ʽ�����
    m_nprintf(12, "%s", buf); //! ָ�������Ҵ���ʽ���Ƶĸ�ʽ�����
    m_snprintf(buf, 12, "%s", buf); //! ָ�����ȣ���ȥ����ʽ���ƣ��ĸ�ʽ����ָ��buffer
    m_nprintf(12, buf); //! ��ʽ����ָ����buffer���ٸ�ʽ�����������
    //--------------------------------------------------------------------------
    //�ַ�����������ڲ��Դ���ַ�������Լ������ַ��ӻس�����������
    m_printf("a");
    m_printf("a\r\n");
    m_printf("abc\r\n");
    m_printf("%s\r\n","abc");
    //--------------------------------------------------------------------------
    //ʮ���ƴ���������ʽ���������������롢�Ҷ��롢ռλ��0�����������
    m_printf("%05d|%6d|%7d|\r\n", -123, 456, -789);
    m_printf("%-05d|%-6d|%-7d|\r\n", 123, -456, 789);
    m_printf("%-05d|%-6d|%-7d|\r\n", -123, 456, -789);
    //--------------------------------------------------------------------------
    //ʮ�����޷�������ʽ���������������롢�Ҷ��롢ռλ��0�����������
    m_printf("%05u|%6u|%7u|\r\n", 123, -456, 789);
    m_printf("%05u|%6u|%7u|\r\n", -123, 456, -789);
    m_printf("%-05u|%-6u|%-7u|\r\n", 123, -456, 789);
    m_printf("%-05u|%-6u|%-7u|\r\n", -123, 456, -789);
    //--------------------------------------------------------------------------
    //ʮ����������ʽ���������������롢�Ҷ��롢ռλ��0�����������
    m_printf("%05x|%6x|%7x|\r\n", 123, -456, 789);
    m_printf("%05x|%6x|%7x|\r\n", -123, 456, -789);
    m_printf("%-05X|%-6X|%-7X|\r\n", 123, -456, 789);
    m_printf("%-05X|%-6X|%-7X|\r\n", -123, 456, -789);
    //--------------------------------------------------------------------------
    //�ַ�����ʽ���������������롢�Ҷ��롢ռλ��0����������ԣ��ַ�����ռλ��ǿ��Ϊ�ո�
    m_printf("%5s|%6s|%7s|\r\n", "test", "test", "test");
    m_printf("%05s|%06s|%07s|\r\n", "test", "test", "test");
    m_printf("%-5s|%-6s|%-7s|\r\n", "test", "test", "test");
    m_printf("%-05s|%-06s|%-07s|\r\n", "test", "test", "test");
    //--------------------------------------------------------------------------
    //ָ���ַ��ʽ�������������Сдʮ��������ʾ���Լ��ո����0�ַ�ռλ��
    m_printf("addr:%p\r\n", buf);
    m_printf("addr:%P\r\n", buf);
    m_printf("addr:%8P\r\n", buf);
    m_printf("addr:%08p\r\n", buf);
    //--------------------------------------------------------------------------
    //��ʽ����־���������·���������������кš����Եȼ��ȣ������ɿ������ӣ����߳�����ʱ��ȣ�
    M_LOG_ERROR("error test\r\n");
    M_LOG_WARNING("warning test\r\n");
    M_LOG_INFO("info test\r\n");
    M_LOG_DEBUG("debug test\r\n");
    #endif
    #if 0
    //--------------------------------------------------------------------------
    //ʮ�����ƴ����ȿ��Ƶĸ�ʽ����������ڲ����ַ����м���0ֵ��������
    char buf[16] = {"\x12\x34\x00\x56\x00\x78\x00\x00\x91\x00\x00\x00"};
    m_nprintf(12, buf); //! ָ�����Ȳ��Ҳ�����ʽ���Ƶĸ�ʽ�����
    m_nprintf(12, "%s", buf); //! ָ�������Ҵ���ʽ���Ƶĸ�ʽ�����
    m_snprintf(buf, 12, "%s", buf); //! ָ�����ȣ���ȥ����ʽ���ƣ��ĸ�ʽ����ָ��buffer
    m_nprintf(12, buf); //! ��ʽ����ָ����buffer���ٸ�ʽ�����������
    //--------------------------------------------------------------------------
    //�ַ�����������ڲ��Դ���ַ�������Լ������ַ��ӻس�����������
    M_LOG_INFO("a");
    M_LOG_INFO("a\r\n");
    M_LOG_INFO("abc\r\n");
    M_LOG_INFO("%s\r\n","abc");
    //--------------------------------------------------------------------------
    //ʮ���ƴ���������ʽ���������������롢�Ҷ��롢ռλ��0�����������
    M_LOG_INFO("%05d|%6d|%7d|\r\n", -123, 456, -789);
    M_LOG_INFO("%-05d|%-6d|%-7d|\r\n", 123, -456, 789);
    M_LOG_INFO("%-05d|%-6d|%-7d|\r\n", -123, 456, -789);
    //--------------------------------------------------------------------------
    //ʮ�����޷�������ʽ���������������롢�Ҷ��롢ռλ��0�����������
    M_LOG_INFO("%05u|%6u|%7u|\r\n", 123, -456, 789);
    M_LOG_INFO("%05u|%6u|%7u|\r\n", -123, 456, -789);
    M_LOG_INFO("%-05u|%-6u|%-7u|\r\n", 123, -456, 789);
    M_LOG_INFO("%-05u|%-6u|%-7u|\r\n", -123, 456, -789);
    //--------------------------------------------------------------------------
    //ʮ����������ʽ���������������롢�Ҷ��롢ռλ��0�����������
    M_LOG_INFO("%05x|%6x|%7x|\r\n", 123, -456, 789);
    M_LOG_INFO("%05x|%6x|%7x|\r\n", -123, 456, -789);
    M_LOG_INFO("%-05X|%-6X|%-7X|\r\n", 123, -456, 789);
    M_LOG_INFO("%-05X|%-6X|%-7X|\r\n", -123, 456, -789);
    //--------------------------------------------------------------------------
    //�ַ�����ʽ���������������롢�Ҷ��롢ռλ��0����������ԣ��ַ�����ռλ��ǿ��Ϊ�ո�
    M_LOG_INFO("%5s|%6s|%7s|\r\n", "test", "test", "test");
    M_LOG_INFO("%05s|%06s|%07s|\r\n", "test", "test", "test");
    M_LOG_INFO("%-5s|%-6s|%-7s|\r\n", "test", "test", "test");
    M_LOG_INFO("%-05s|%-06s|%-07s|\r\n", "test", "test", "test");
    //--------------------------------------------------------------------------
    //ָ���ַ��ʽ�������������Сдʮ��������ʾ���Լ��ո����0�ַ�ռλ��
    M_LOG_INFO("addr:%p\r\n", buf);
    M_LOG_INFO("addr:%P\r\n", buf);
    M_LOG_INFO("addr:%8P\r\n", buf);
    M_LOG_INFO("addr:%08p\r\n", buf);
    //--------------------------------------------------------------------------
    //��ʽ����־���������·���������������кš����Եȼ��ȣ������ɿ������ӣ����߳�����ʱ��ȣ�
    M_LOG_ERROR("error test\r\n");
    M_LOG_WARNING("warning test\r\n");
    M_LOG_INFO("info test\r\n");
    M_LOG_DEBUG("debug test\r\n");
    #endif
}
