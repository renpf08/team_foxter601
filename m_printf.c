/**
*  @file    m_printf.c
*  @author  maliwen
*  @date    2020/02/28
*  @history 1.0.0.0(版本号)
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
* @brief 当使用printf时，m_vprintf每格式化一个字符就输出一个字符（也可以全部格式化到一个buffer完之后整体输出，此处涉及到效率问题，后期可以根据实际情况再做优化）
* @param [in] c - 格式化后待输出的字符
* @param [out] none
* @return none
* @data 2020/03/03 17:07
* @author maliwen
* @note 此处使用该函数，是方便不同平台的移植
*/
void m_uart_out(char c)
{
    UartWriteBlocking((const char*)&c, 1);
}

/**
* @brief 数据格式化全处理
* @param [in] strBuf - 非空，则数据格式化后回存输出，否则no use；sFormat - 格式化字符串；... - 格式化数值列表
* @param [out] none
* @return dont care
* @data 2020/03/03 17:03
* @author maliwen
* @note none
*/
int m_vprintf(char *strBuf, const char * sFormat, va_list * pParamList) 
{
    //! 注：调试串口打印函数时，可能不能正常进入m_vprintf函数，此时可以尝试把hexCharTbl定义为全局变量
    char hexCharTbl[2][16] = {{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'}, 
                              {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' }};
	unsigned letter = 0; //! 十六进制大小写选择
	signed tmpVal = 0; //! 带符号数值临时变量
	unsigned uTmpVal = 0; //! 不带符号数值临时变量
	#ifdef USE_32BIT_MCU //! 浮点数此平台不支持，暂不处理（保留日后扩展，需要优化）
	double fVal = 1.0;
	double fTmpVal = 1.0;
	//  double fRound = 0.5; //! 四舍五入值
	#endif //! 保留该内容，等以后移植到兼容32为的平台上时，可使用上面浮点数（本平台不支持浮点）
	signed justify = 0; //! ==0:右对齐；>0:左对齐格数
	char inserChar = ' '; //! 占位符
	int i = 0; //! 必须为带符号类型
	char c; //! 用于逐个读取sFormat内容
	int v; //! 用于逐个读取pParamList参数列表里面的参数值
	unsigned NumDigits; //! 主要用于浮点数（该平台不支持浮点类型）
	unsigned FormatFlags; //! 用于格式化控制的字符
	unsigned FieldWidth = 0; //! 格式化后数据显示的占位数
	char tmpBuf[32]; //! 数值转字符串临时数组
	int len = 0; //! 当使用sprintf时，用于指示存放格式化字符存放的下标
	
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
					char fmtBuf[64] = {0}; //! 格式化数字为字符串
					unsigned fmtPos = 0; //! 格式化数字到字符串buf的位置
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
						    else m_uart_out('-');
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
					while(i >= 0)
					{
						fmtBuf[fmtPos++] = tmpBuf[i--];
					}
					if((justify == 0) || ((justify == 1) && (inserChar == '0'))) //! 右对齐，则左边插入空格（或者左对齐并且为0填充时，则强制为右对齐，即只在左边填充0，不能在右边填充0，会改变显示数值）
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
					if((justify == 1) && (inserChar != '0')) //! 左对齐，并且占位符不是数字0时，则右边插入空格（为0不能在右边插入占位符，会改变显示数值）
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
					char fmtBuf[64] = {0}; //! 格式化数字为字符串
					unsigned fmtPos = 0; //! 格式化数字到字符串buf的位置
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
					while(i >= 0)
					{
						fmtBuf[fmtPos++] = tmpBuf[i--];
					}
					if((justify == 0) || ((justify == 1) && (inserChar == '0'))) //! 右对齐，则左边插入空格（或者左对齐并且为0填充时，则强制为右对齐，即只在左边填充0，不能在右边填充0，会改变显示数值）
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
					if((justify == 1) && (inserChar != '0')) //! 左对齐，并且占位符不是数字0时，则右边插入空格（为0不能在右边插入占位符，会改变显示数值）
					{
						for(i = 0; i < (int)FieldWidth; i++)
						{
							if(strBuf != 0) strBuf[len++] = inserChar;
							else m_uart_out(inserChar);
						}
					}
					break;
				}
				case 'p': //! 指针跟十六进制数值可以相同处理
				case 'P':
				case 'x':
				case 'X':
				{
					char fmtBuf[64] = {0}; //! 格式化数字为字符串
					unsigned fmtPos = 0; //! 格式化数字到字符串buf的位置
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
					while(i >= 0)
					{
						fmtBuf[fmtPos++] = tmpBuf[i--];
					}
					if((justify == 0) || ((justify == 1) && (inserChar == '0'))) //! 右对齐，则左边插入空格（或者左对齐并且为0填充时，则强制为右对齐，即只在左边填充0，不能在右边填充0，会改变显示数值）
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
					if((justify == 1) && (inserChar != '0')) //! 左对齐，并且占位符不是数字0时，则右边插入空格（为0不能在右边插入占位符，会改变显示数值）
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
					i = 0; //! 字符串长度
					while(s[i++] != '\0');
					FieldWidth = (FieldWidth>(unsigned)i)?(FieldWidth-(unsigned)i):0; //! 计算可插入空格的位域
					FieldWidth = (FieldWidth!=0)?(FieldWidth+1):0; //! 测试发现，输出占位符比设置占位符少一个，故此加一
					justify = ((FormatFlags&FORMAT_FLAG_LEFT_JUSTIFY) == FORMAT_FLAG_LEFT_JUSTIFY)?1:0; //! 1 - 左对齐；0 - 右对齐
					if(justify == 0) //! 右对齐，则左边插入空格
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
					if(justify == 1) //! 左对齐，则右边插入空格
					{
						for(i = 0; i < (int)FieldWidth; i++)
						{
							if(strBuf != 0) strBuf[len++] = ' ';
							else m_uart_out(' ');
						}
					}
					break;
				}
				#ifdef USE_32BIT_MCU //! 浮点数此平台不支持，暂不处理（保留日后扩展，需要优化）
				case 'f': //! float实际精度只有3位小数，如果需要精确度更高，则建议传入double型，此处限定最大6位小数，另外整数部分绝对值也不适宜很大，否则小数部分会被挤掉；总之使用浮点打印需要十分注意！！！
				{
					fVal = va_arg(*pParamList, double);
					tmpVal = (signed)fVal; //! 整数部分
					fTmpVal = fVal-tmpVal; //! 小数部分
					//        for(i = 0; i < NumDigits; i++)
					//        {/** 根据小数位数，先求四舍五入 */
					//            fRound /= 10;
					//        }
					//        fTmpVal += fRound;
					/** 拼接整数部分 */
					i = 0;
					flag = '+';
					if(fVal < 0)
					{
						flag = '-';
						tmpVal = ~tmpVal+1;
					}
					do/** 注意格式化最长顺序为64个字符 */
					{/** 倒叙格式化 */
						tmpBuf[i++] = hexCharTbl[0][tmpVal%10];
						tmpVal /= 10;
					} while(tmpVal != 0);
					FieldWidth = (FieldWidth>((unsigned)i+NumDigits+1))?(FieldWidth-(NumDigits+1)):(unsigned)i; //! 取最长位域
					i--;
					//        if(flag == '-')
					//        {
					//            if(strBuf != 0) strBuf[len++] = flag;
					//			  else m_uart_out(flag);
					//        }
					for(width = 0; width < (int)FieldWidth; width++)
					{/** 顺序格式化 */
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
					/** 拼接小数部分 */
					tmpVal = fTmpVal*1000000000; //! 单精度float型，小数有效位为6位，此处不够完善，如果输入double型需要小数位数较多时，这也限死了6位小数
					if(tmpVal < 0)
					{
						tmpVal = ~tmpVal+1;
					}
					do
					{
						if((tmpVal%10) != 0) break;
						tmpVal /= 10; //! 去掉小数后面无效的0值位
					}while(tmpVal != 0);
					if(strBuf != 0) strBuf[len++] = '.';
					else m_uart_out('.');
					i = 0;
					do/** 注意格式化最长顺序为64个字符 */
					{/** 倒叙格式化 */
						tmpBuf[i++] = hexCharTbl[0][tmpVal%10];
						tmpVal /= 10;
					} while(tmpVal != 0);
					//        i = (i > NumDigits)?NumDigits:i;
					for(width = 0; width < i; width++)
					{/** 顺序格式化 */
						if((width >= (int)NumDigits) && (NumDigits != 0)) break; //! 取消数位数不超过指定位域
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
* @brief 格式化字符串，并输出到串口
* @param [in] sFormat - 格式化字符串；... - 格式化数值列表
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
    r = va_arg(ParamList, int); //! 测试发现，参数列表的第一个参数是乱码，此处需要做一次读操作相当于去掉第一个参数！！！
    r = m_vprintf(0, sFormat, &ParamList);
    va_end(ParamList);
    return r;
}

/**
* @brief 格式化字符串到指定buffer，并输出该buffer
* @param [in] buf - 格式化后的字符串存于该buffer，用于输出
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
    //r = va_arg(ParamList, int); //! 测试发现，参数列表的第一个参数是乱码，此处需要做一次读操作相当于去掉第一个参数！！！
    r = m_vprintf(buf, sFormat, &ParamList);
    va_end(ParamList);
    buf[r] = '\0';
    return r;
}

/**
* @brief 按日志的方式将调试信息格式化打印到串口，主要是添加了一些例如日志所在文件、函数、行数、调试级别等信息，方便快速定位以及解决问题
* @param [in] file - 所在文件名；func - 所在函数名；line - 所在行号； level - 调试等级
* @param [out] dont care
* @return dont care
* @data 2020/03/03 17:11
* @author maliwen
* @note 此函数调试输出的冗余信息较多，普通串口打印输出时不建议调用（建议使用m_printf()）
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
    r = va_arg(ParamList, int); //! 测试发现，参数列表的第一个参数是乱码，此处需要做一次读操作相当于去掉第一个参数！！！
    r = m_vprintf(0, str, &ParamList);
    va_end(ParamList);
    return r;
}

/**
* @brief 用来测试格式化输出是否是期望的结果
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
