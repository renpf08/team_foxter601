/**
*  @file    m_printf.c
*  @author  maliwen
*  @date    2020/02/28
*  @history 1.0.0.0(版本号)
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
* @note 有bug：当打印字符串为数值字符串，而字符串中间有数值0x25（十进制37）时，会被识别成格式控
*       制符：%，进而导致输出出错，目前尚未对此bug进行修复（该问题发现与20200314 23:47）
*/
//! 注：调试串口打印函数时，可能不能正常进入m_vprintf函数，此时可以尝试把hexCharTbl定义为全局变量
int m_vprintf(char *strBuf, unsigned size, const char * sFormat, va_list * pParamList) 
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
				if((justify == 0) || ((justify == 1) && (inserChar == '0'))) //! 右对齐，则左边插入空格（或者左对齐并且为0填充时，则强制为右对齐，即只在左边填充0，不能在右边填充0，会改变显示数值）
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
				if((justify == 1) && (inserChar != '0')) //! 左对齐，并且占位符不是数字0时，则右边插入空格（为0不能在右边插入占位符，会改变显示数值）
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
            if(size > 0) size--; //! 如果是带长度的格式化输出，则每输出一个字符，长度减一(当m_nprintf中未使用“%s”格式化时，会直接逐个字符输出)
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
    r = m_vprintf(0, 0, sFormat, &ParamList);
    va_end(ParamList);
    return r;
}

/**
* @brief 带指定长度的格式化字符串，并输出到串口
* @param [in] size - 指定格式化的参数长度；sFormat - 格式化字符串；... - 格式化数值列表
* @param [out] none
* @return dont care
* @data 2020/03/05 09:19
* @author maliwen
* @note 参数size用于指定长度的格式化字符串的输出，以保证输出字符串中间有0值时不会被截断
*       如果待格式化输出的字符串中可能包含0值时，需要使用此函数(指定格式化的长度)
*/
int m_nprintf(unsigned size, const char * sFormat, ...)
{
    int r = 0;
    va_list ParamList;

    va_start(ParamList, sFormat);
    //r = va_arg(ParamList, int); //! 测试发现，参数列表的第一个参数是乱码，此处需要做一次读操作相当于去掉第一个参数！！！
    r = m_vprintf(0, size, sFormat, &ParamList);
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
    r = m_vprintf(buf, 0, sFormat, &ParamList);
    va_end(ParamList);
    buf[r] = '\0';
    return r;
}

/**
* @brief 带指定长度的格式化字符串到指定buffer，并输出该buffer
* @param [in] size - 指定格式化的参数长度；buf - 格式化后的字符串存于该buffer，用于输出
* @param [out] buf
* @return dont care
* @data 2020/03/05 10:57
* @author maliwen
* @note 参数size用于指定长度的格式化字符串的输出，以保证输出字符串中间有0值时不会被截断
*       如果待格式化输出的字符串中可能包含0值时，需要使用此函数(指定格式化的长度)
*/
int m_snprintf(char *buf, unsigned size, const char * sFormat, ...)
{
    int r = 0;
    va_list ParamList;

    va_start(ParamList, sFormat);
    //r = va_arg(ParamList, int); //! 测试发现，参数列表的第一个参数是乱码，此处需要做一次读操作相当于去掉第一个参数！！！
    r = m_vprintf(buf, size, sFormat, &ParamList);
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
    //r = va_arg(ParamList, int); //! 测试发现，参数列表的第一个参数是乱码，此处需要做一次读操作相当于去掉第一个参数！！！
    r = m_vprintf(0, 0, sFormat, &ParamList);
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
    #if 0
    //--------------------------------------------------------------------------
    //当输出字符串中间有数值0x25时，会被当成格式控制符%，进而会导致输出出错，此bug暂不修复
    //mlw 20200315 00:11
    m_printf("\x12\x25\x35\x56\x78\x0d\x0a");
    //--------------------------------------------------------------------------
    //十六进制带长度控制的格式化输出，用于测试字符串中间有0值的输出情况
    char buf[16] = {"\x12\x34\x00\x56\x00\x78\x00\x00\x91\x00\x00\x00"};
    m_nprintf(12, buf); //! 指定长度并且不带格式控制的格式化输出
    m_nprintf(12, "%s", buf); //! 指定长度且带格式控制的格式化输出
    m_snprintf(buf, 12, "%s", buf); //! 指定长度（必去带格式控制）的格式化到指定buffer
    m_nprintf(12, buf); //! 格式化到指定的buffer后，再格式化输出到串口
    //--------------------------------------------------------------------------
    //字符串输出，用于测试打个字符输出，以及单个字符加回车换行数处等
    m_printf("a");
    m_printf("a\r\n");
    m_printf("abc\r\n");
    m_printf("%s\r\n","abc");
    //--------------------------------------------------------------------------
    //十进制带符号数格式化输出，包括左对齐、右对齐、占位符0填充等输出测试
    m_printf("%05d|%6d|%7d|\r\n", -123, 456, -789);
    m_printf("%-05d|%-6d|%-7d|\r\n", 123, -456, 789);
    m_printf("%-05d|%-6d|%-7d|\r\n", -123, 456, -789);
    //--------------------------------------------------------------------------
    //十进制无符号数格式化输出，包括左对齐、右对齐、占位符0填充等输出测试
    m_printf("%05u|%6u|%7u|\r\n", 123, -456, 789);
    m_printf("%05u|%6u|%7u|\r\n", -123, 456, -789);
    m_printf("%-05u|%-6u|%-7u|\r\n", 123, -456, 789);
    m_printf("%-05u|%-6u|%-7u|\r\n", -123, 456, -789);
    //--------------------------------------------------------------------------
    //十六进制数格式化输出，包括左对齐、右对齐、占位符0填充等输出测试
    m_printf("%05x|%6x|%7x|\r\n", 123, -456, 789);
    m_printf("%05x|%6x|%7x|\r\n", -123, 456, -789);
    m_printf("%-05X|%-6X|%-7X|\r\n", 123, -456, 789);
    m_printf("%-05X|%-6X|%-7X|\r\n", -123, 456, -789);
    //--------------------------------------------------------------------------
    //字符串格式化输出，包括左对齐、右对齐、占位符0填充等输出测试（字符串的占位符强制为空格）
    m_printf("%5s|%6s|%7s|\r\n", "test", "test", "test");
    m_printf("%05s|%06s|%07s|\r\n", "test", "test", "test");
    m_printf("%-5s|%-6s|%-7s|\r\n", "test", "test", "test");
    m_printf("%-05s|%-06s|%-07s|\r\n", "test", "test", "test");
    //--------------------------------------------------------------------------
    //指针地址格式化输出，包括大小写十六进制显示，以及空格符、0字符占位符
    m_printf("addr:%p\r\n", buf);
    m_printf("addr:%P\r\n", buf);
    m_printf("addr:%8P\r\n", buf);
    m_printf("addr:%08p\r\n", buf);
    //--------------------------------------------------------------------------
    //格式化日志输出，包括路径名、函数名、行号、调试等级等（后续可看情况添加，如线程名，时间等）
    M_LOG_ERROR("error test\r\n");
    M_LOG_WARNING("warning test\r\n");
    M_LOG_INFO("info test\r\n");
    M_LOG_DEBUG("debug test\r\n");
    #endif
    #if 0
    //--------------------------------------------------------------------------
    //十六进制带长度控制的格式化输出，用于测试字符串中间有0值的输出情况
    char buf[16] = {"\x12\x34\x00\x56\x00\x78\x00\x00\x91\x00\x00\x00"};
    m_nprintf(12, buf); //! 指定长度并且不带格式控制的格式化输出
    m_nprintf(12, "%s", buf); //! 指定长度且带格式控制的格式化输出
    m_snprintf(buf, 12, "%s", buf); //! 指定长度（必去带格式控制）的格式化到指定buffer
    m_nprintf(12, buf); //! 格式化到指定的buffer后，再格式化输出到串口
    //--------------------------------------------------------------------------
    //字符串输出，用于测试打个字符输出，以及单个字符加回车换行数处等
    M_LOG_INFO("a");
    M_LOG_INFO("a\r\n");
    M_LOG_INFO("abc\r\n");
    M_LOG_INFO("%s\r\n","abc");
    //--------------------------------------------------------------------------
    //十进制带符号数格式化输出，包括左对齐、右对齐、占位符0填充等输出测试
    M_LOG_INFO("%05d|%6d|%7d|\r\n", -123, 456, -789);
    M_LOG_INFO("%-05d|%-6d|%-7d|\r\n", 123, -456, 789);
    M_LOG_INFO("%-05d|%-6d|%-7d|\r\n", -123, 456, -789);
    //--------------------------------------------------------------------------
    //十进制无符号数格式化输出，包括左对齐、右对齐、占位符0填充等输出测试
    M_LOG_INFO("%05u|%6u|%7u|\r\n", 123, -456, 789);
    M_LOG_INFO("%05u|%6u|%7u|\r\n", -123, 456, -789);
    M_LOG_INFO("%-05u|%-6u|%-7u|\r\n", 123, -456, 789);
    M_LOG_INFO("%-05u|%-6u|%-7u|\r\n", -123, 456, -789);
    //--------------------------------------------------------------------------
    //十六进制数格式化输出，包括左对齐、右对齐、占位符0填充等输出测试
    M_LOG_INFO("%05x|%6x|%7x|\r\n", 123, -456, 789);
    M_LOG_INFO("%05x|%6x|%7x|\r\n", -123, 456, -789);
    M_LOG_INFO("%-05X|%-6X|%-7X|\r\n", 123, -456, 789);
    M_LOG_INFO("%-05X|%-6X|%-7X|\r\n", -123, 456, -789);
    //--------------------------------------------------------------------------
    //字符串格式化输出，包括左对齐、右对齐、占位符0填充等输出测试（字符串的占位符强制为空格）
    M_LOG_INFO("%5s|%6s|%7s|\r\n", "test", "test", "test");
    M_LOG_INFO("%05s|%06s|%07s|\r\n", "test", "test", "test");
    M_LOG_INFO("%-5s|%-6s|%-7s|\r\n", "test", "test", "test");
    M_LOG_INFO("%-05s|%-06s|%-07s|\r\n", "test", "test", "test");
    //--------------------------------------------------------------------------
    //指针地址格式化输出，包括大小写十六进制显示，以及空格符、0字符占位符
    M_LOG_INFO("addr:%p\r\n", buf);
    M_LOG_INFO("addr:%P\r\n", buf);
    M_LOG_INFO("addr:%8P\r\n", buf);
    M_LOG_INFO("addr:%08p\r\n", buf);
    //--------------------------------------------------------------------------
    //格式化日志输出，包括路径名、函数名、行号、调试等级等（后续可看情况添加，如线程名，时间等）
    M_LOG_ERROR("error test\r\n");
    M_LOG_WARNING("warning test\r\n");
    M_LOG_INFO("info test\r\n");
    M_LOG_DEBUG("debug test\r\n");
    #endif
}
