/**
*  @file    m_printf.c
*  @author  maliwen
*  @date    2020/02/28
*  @history 1.0.0.0(�汾��)
*           1.0.0.0: Creat              2020/02/28
*/
#include <uart.h>
#include "m_printf.h"

#define FORMAT_FLAG_LEFT_JUSTIFY   (1u << 0)
#define FORMAT_FLAG_PAD_ZERO       (1u << 1)
#define FORMAT_FLAG_PRINT_SIGN     (1u << 2)
#define FORMAT_FLAG_ALTERNATE      (1u << 3)

void m_uart_out(char c);
int m_vprintf(char *strBuf, const char * sFormat, va_list * pParamList);

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
* @note none
*/
int m_vprintf(char *strBuf, const char * sFormat, va_list * pParamList) 
{
    //! ע�����Դ��ڴ�ӡ����ʱ�����ܲ�����������m_vprintf��������ʱ���Գ��԰�hexCharTbl����Ϊȫ�ֱ���
    char hexCharTbl[2][16] = {{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'}, 
                              {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' }};
	unsigned letter = 0; //! ʮ�����ƴ�Сдѡ��
	signed tmpVal = 0; //! ��������ֵ��ʱ����
	unsigned uTmpVal = 0; //! ����������ֵ��ʱ����
	#ifdef USE_32BIT_MCU //! ��������ƽ̨��֧�֣��ݲ����������պ���չ����Ҫ�Ż���
	double fVal = 1.0;
	double fTmpVal = 1.0;
	//  double fRound = 0.5; //! ��������ֵ
	#endif //! ���������ݣ����Ժ���ֲ������32Ϊ��ƽ̨��ʱ����ʹ�����渡��������ƽ̨��֧�ָ��㣩
	signed justify = 0; //! ==0:�Ҷ��룻>0:��������
	char inserChar = ' '; //! ռλ��
	int i = 0; //! ����Ϊ����������
	char c; //! ���������ȡsFormat����
	int v; //! ���������ȡpParamList�����б�����Ĳ���ֵ
	unsigned NumDigits; //! ��Ҫ���ڸ���������ƽ̨��֧�ָ������ͣ�
	unsigned FormatFlags; //! ���ڸ�ʽ�����Ƶ��ַ�
	unsigned FieldWidth = 0; //! ��ʽ����������ʾ��ռλ��
	char tmpBuf[32]; //! ��ֵת�ַ�����ʱ����
	int len = 0; //! ��ʹ��sprintfʱ������ָʾ��Ÿ�ʽ���ַ���ŵ��±�
	
	do
	{
		c = *sFormat;
		sFormat++;
		if (c == 0u)
		{
			break;
		}
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
					char c0;
					v = va_arg(*pParamList, int);
					c0 = (char)v;
					if(strBuf != 0) strBuf[len++] = c0;
					else m_uart_out(c0);
					break;
				}
				case 'd':
				{
					char fmtBuf[64] = {0}; //! ��ʽ������Ϊ�ַ���
					unsigned fmtPos = 0; //! ��ʽ�����ֵ��ַ���buf��λ��
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
					do
					{
						if(strBuf != 0) strBuf[len++] = fmtBuf[i++];
						else m_uart_out(fmtBuf[i++]);
					} while(fmtBuf[i] != '\0');
					if((justify == 1) && (inserChar != '0')) //! ����룬����ռλ����������0ʱ�����ұ߲���ո�Ϊ0�������ұ߲���ռλ������ı���ʾ��ֵ��
					{
						for(i = 0; i < (int)FieldWidth; i++)
						{
							if(strBuf != 0) strBuf[len++] = inserChar;
							else m_uart_out(inserChar);
						}
					}
					break;
				}
				case 'u':
				{
					char fmtBuf[64] = {0}; //! ��ʽ������Ϊ�ַ���
					unsigned fmtPos = 0; //! ��ʽ�����ֵ��ַ���buf��λ��
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
					do
					{
						if(strBuf != 0) strBuf[len++] = fmtBuf[i++];
						else m_uart_out(fmtBuf[i++]);
					} while(fmtBuf[i] != '\0');
					if((justify == 1) && (inserChar != '0')) //! ����룬����ռλ����������0ʱ�����ұ߲���ո�Ϊ0�������ұ߲���ռλ������ı���ʾ��ֵ��
					{
						for(i = 0; i < (int)FieldWidth; i++)
						{
							if(strBuf != 0) strBuf[len++] = inserChar;
							else m_uart_out(inserChar);
						}
					}
					break;
				}
				case 'p': //! ָ���ʮ��������ֵ������ͬ����
				case 'P':
				case 'x':
				case 'X':
				{
					char fmtBuf[64] = {0}; //! ��ʽ������Ϊ�ַ���
					unsigned fmtPos = 0; //! ��ʽ�����ֵ��ַ���buf��λ��
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
					do
					{
						if(strBuf != 0) strBuf[len++] = fmtBuf[i++];
						else m_uart_out(fmtBuf[i++]);
					} while(fmtBuf[i] != '\0');
					if((justify == 1) && (inserChar != '0')) //! ����룬����ռλ����������0ʱ�����ұ߲���ո�Ϊ0�������ұ߲���ռλ������ı���ʾ��ֵ��
					{
						for(i = 0; i < (int)FieldWidth; i++)
						{
							if(strBuf != 0) strBuf[len++] = inserChar;
							else m_uart_out(inserChar);
						}
					}
					break;
				}
				case 's':
				{
					const char * s = va_arg(*pParamList, const char *);
					justify = 0;
					i = 0; //! �ַ�������
					while(s[i++] != '\0');
					FieldWidth = (FieldWidth>(unsigned)i)?(FieldWidth-(unsigned)i):0; //! ����ɲ���ո��λ��
					FieldWidth = (FieldWidth!=0)?(FieldWidth+1):0; //! ���Է��֣����ռλ��������ռλ����һ�����ʴ˼�һ
					justify = ((FormatFlags&FORMAT_FLAG_LEFT_JUSTIFY) == FORMAT_FLAG_LEFT_JUSTIFY)?1:0; //! 1 - ����룻0 - �Ҷ���
					if(justify == 0) //! �Ҷ��룬����߲���ո�
					{
						for(i = 0; i < (int)FieldWidth; i++)
						{
							if(strBuf != 0) strBuf[len++] = ' ';
							else m_uart_out(' ');
						}
					}
					do
					{
						c = *s;
						s++;
						if (c == '\0') break;
						if(strBuf != 0) strBuf[len++] = c;
						else m_uart_out(c);
					} while(1);
					if(justify == 1) //! ����룬���ұ߲���ո�
					{
						for(i = 0; i < (int)FieldWidth; i++)
						{
							if(strBuf != 0) strBuf[len++] = ' ';
							else m_uart_out(' ');
						}
					}
					break;
				}
				#ifdef USE_32BIT_MCU //! ��������ƽ̨��֧�֣��ݲ����������պ���չ����Ҫ�Ż���
				case 'f': //! floatʵ�ʾ���ֻ��3λС���������Ҫ��ȷ�ȸ��ߣ����鴫��double�ͣ��˴��޶����6λС���������������־���ֵҲ�����˺ܴ󣬷���С�����ֻᱻ��������֮ʹ�ø����ӡ��Ҫʮ��ע�⣡����
				{
					fVal = va_arg(*pParamList, double);
					tmpVal = (signed)fVal; //! ��������
					fTmpVal = fVal-tmpVal; //! С������
					//        for(i = 0; i < NumDigits; i++)
					//        {/** ����С��λ���������������� */
					//            fRound /= 10;
					//        }
					//        fTmpVal += fRound;
					/** ƴ���������� */
					i = 0;
					flag = '+';
					if(fVal < 0)
					{
						flag = '-';
						tmpVal = ~tmpVal+1;
					}
					do/** ע���ʽ���˳��Ϊ64���ַ� */
					{/** �����ʽ�� */
						tmpBuf[i++] = hexCharTbl[0][tmpVal%10];
						tmpVal /= 10;
					} while(tmpVal != 0);
					FieldWidth = (FieldWidth>((unsigned)i+NumDigits+1))?(FieldWidth-(NumDigits+1)):(unsigned)i; //! ȡ�λ��
					i--;
					//        if(flag == '-')
					//        {
					//            if(strBuf != 0) strBuf[len++] = flag;
					//			  else m_uart_out(flag);
					//        }
					for(width = 0; width < (int)FieldWidth; width++)
					{/** ˳���ʽ�� */
						shift = (unsigned)(FieldWidth-width-1);
						if(shift > (unsigned)i)
						{
							if(strBuf != 0) strBuf[len++] = ' ';
							else m_uart_out(' ');
						}
						else
						{
							if(flag == '-')
							{
								if(strBuf != 0) strBuf[len++] = flag;
								else m_uart_out(flag);
								flag = '+';
							}
							if(strBuf != 0) strBuf[len++] = tmpBuf[shift];
							else m_uart_out(tmpBuf[shift]);
						}
					}
					/** ƴ��С������ */
					tmpVal = fTmpVal*1000000000; //! ������float�ͣ�С����ЧλΪ6λ���˴��������ƣ��������double����ҪС��λ���϶�ʱ����Ҳ������6λС��
					if(tmpVal < 0)
					{
						tmpVal = ~tmpVal+1;
					}
					do
					{
						if((tmpVal%10) != 0) break;
						tmpVal /= 10; //! ȥ��С��������Ч��0ֵλ
					}while(tmpVal != 0);
					if(strBuf != 0) strBuf[len++] = '.';
					else m_uart_out('.');
					i = 0;
					do/** ע���ʽ���˳��Ϊ64���ַ� */
					{/** �����ʽ�� */
						tmpBuf[i++] = hexCharTbl[0][tmpVal%10];
						tmpVal /= 10;
					} while(tmpVal != 0);
					//        i = (i > NumDigits)?NumDigits:i;
					for(width = 0; width < i; width++)
					{/** ˳���ʽ�� */
						if((width >= (int)NumDigits) && (NumDigits != 0)) break; //! ȡ����λ��������ָ��λ��
						shift = (unsigned)(i-width-1);
						if(shift > (unsigned)i)
						{
							if(strBuf != 0) strBuf[len++] = '0';
							else m_uart_out('0');
						}
						else
						{
							if(strBuf != 0) strBuf[len++] = tmpBuf[shift];
							else m_uart_out(tmpBuf[shift]);
						}
					}
					if(width < (int)NumDigits)
					{
						width = NumDigits-width;
						for(i = 0; i < width; i++)
						{
							if(strBuf != 0) strBuf[len++] = '0';
							else m_uart_out('0');
						}
					}
					break;
				}
				#endif
				case '%':
				{
					if(strBuf != 0) strBuf[len++] = '%';
					else m_uart_out('%');
					break;
					default:
					break;
				}
			}
			sFormat++;
		}
		else
		{
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
    r = m_vprintf(0, sFormat, &ParamList);
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
    r = m_vprintf(buf, sFormat, &ParamList);
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
    char preStr[32] = {0};
    char str[128] = {0};
    unsigned len = 0;
    
    
    do
    {
        preStr[len++] = *file;
        if(*file == '\\')
        {
            file += len;
            len = 0;
        }
    } while(*++file != '\0');
    preStr[len] = '\0';
    
    m_sprintf(str, "%-25s%-25s%6d%25s ", preStr, func, line, level);
    
    len = 0;
    while(str[len] != 0u)
    {
        len++;
    }
    while(*sFormat != 0u)
    {
        str[len++] = *sFormat++;
    }
    str[len] = '\0';

    va_start(ParamList, sFormat);
    r = va_arg(ParamList, int); //! ���Է��֣������б�ĵ�һ�����������룬�˴���Ҫ��һ�ζ������൱��ȥ����һ������������
    r = m_vprintf(0, str, &ParamList);
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
    char buf[16] = {"12345"};
    m_printf("%05d|%6d|%7d|\r\n", 123, -456, 789);
    m_printf("%05d|%6d|%7d|\r\n", -123, 456, -789);
    m_printf("%-05d|%-6d|%-7d|\r\n", 123, -456, 789);
    m_printf("%-05d|%-6d|%-7d|\r\n", -123, 456, -789);
    //-----------------------------------------------
    m_printf("%05u|%6u|%7u|\r\n", 123, -456, 789);
    m_printf("%05u|%6u|%7u|\r\n", -123, 456, -789);
    m_printf("%-05u|%-6u|%-7u|\r\n", 123, -456, 789);
    m_printf("%-05u|%-6u|%-7u|\r\n", -123, 456, -789);
    //-----------------------------------------------
    m_printf("%05x|%6x|%7x|\r\n", 123, -456, 789);
    m_printf("%05x|%6x|%7x|\r\n", -123, 456, -789);
    m_printf("%-05X|%-6X|%-7X|\r\n", 123, -456, 789);
    m_printf("%-05X|%-6X|%-7X|\r\n", -123, 456, -789);
    m_printf("addr:%p\r\n", buf);
    m_printf("addr:%P\r\n", buf);
    m_printf("addr:%08p\r\n", buf);
    //-----------------------------------------------
    M_LOG_ERROR("error test\r\n");
    M_LOG_WARNING("warning test\r\n");
    M_LOG_INFO("info test\r\n");
    M_LOG_DEBUG("debug test\r\n");
}
