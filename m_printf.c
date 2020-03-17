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
//int m_printf(const char * sFormat, ...);

void m_uart_out(char c)
{
    UartWriteBlocking((const char*)&c, 1);
    //_StoreChar(p, c);
}

char hexCharTbl[2][16] = {{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'}, {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' }};
int m_vprintf(char *strBuf, const char * sFormat, va_list * pParamList) {
  unsigned letter = 0; //! 十六进制大小写选择
  unsigned shift = 0;
  signed tmpVal = 0;
  unsigned uTmpVal = 0;
  #ifdef USE_32BIT_MCU
  double fVal = 1.0;
  double fTmpVal = 1.0;
//  double fRound = 0.5; //! 四舍五入值
  #endif //! 保留该内容，等以后移植到兼容32为的平台上时，可使用上面浮点数（本平台不支持浮点）
  char flag = '+';
  signed justify = 0; //! ==0:右对齐；>0:左对齐格数
  char inserChar = ' ';
  int i = 0;
  char c;
  int v;
  unsigned NumDigits;
  unsigned FormatFlags;
  static unsigned FieldWidth; //! static for testing...
  FieldWidth = 0;
  //char strBuf[256]; //! used for sprintf
  char tmpBuf[32];
  int len = 0;
  int width = 0;
  unsigned pos = 0;
  do {
    c = *sFormat;
    sFormat++;
    if (c == 0u) {
      break;
    }
    pos++;
    if (c == '%') {
      //
      // Filter out flags
      //
      FormatFlags = 0u;
      v = 1;
      do {
        c = *sFormat;
          switch (c) {/** note: 'FormatFlags' did not achive in uart pirnt mode. add by mlw at 20190804 02:32*/
        case '-': FormatFlags |= FORMAT_FLAG_LEFT_JUSTIFY; sFormat++; break;
        case '0': FormatFlags |= FORMAT_FLAG_PAD_ZERO;     sFormat++; break;
        case '+': FormatFlags |= FORMAT_FLAG_PRINT_SIGN;   sFormat++; break;
        case '#': FormatFlags |= FORMAT_FLAG_ALTERNATE;    sFormat++; break;
        default:  v = 0; break;
        }
      } while (v);
      //
      // filter out field with
      //
      FieldWidth = 0u;
      do {
        c = *sFormat;
        if ((c < '0') || (c > '9')) {
          break;
        }
        sFormat++;
        FieldWidth = (FieldWidth * 10u) + ((unsigned)c - '0');
      } while (1);

      //
      // Filter out precision (number of digits to display)
      //
      NumDigits = 0u;
      c = *sFormat;
      if (c == '.') {
        sFormat++;
        do {
          c = *sFormat;
          if ((c < '0') || (c > '9')) {
            break;
          }
          sFormat++;
          NumDigits = NumDigits * 10u + ((unsigned)c - '0');
        } while (1);
      }
      //
      // Filter out length modifier
      //
      c = *sFormat;
      do {
        if ((c == 'l') || (c == 'h')) {
          sFormat++;
          c = *sFormat;
        } else {
          break;
        }
      } while (1);
      //
      // Handle specifiers
      //
      switch (c) {
      case 'c': {
        char c0;
        v = va_arg(*pParamList, int);
        c0 = (char)v;
        if(strBuf != 0) strBuf[len++] = c0;
		else m_uart_out(c0);
        break;
      }
      case 'd':
        v = va_arg(*pParamList, int);
        i = 0;
        inserChar = ' ';
        justify = 0;
        flag = '+';
        tmpVal = (unsigned)v;
        if(tmpVal < 0)
        {
            flag = '-';
            tmpVal = ~tmpVal+1;
        }
        do/** 注意格式化最长顺序为64个字符 */
        {/** 倒叙格式化 */
            tmpBuf[i++] = hexCharTbl[0][tmpVal%10];
            tmpVal /= 10;
        } while(tmpVal != 0);
        FieldWidth = (FieldWidth>(unsigned)i)?FieldWidth:(unsigned)i; //! 取最长位域
        i--;
//        if(flag == '-')
//        {
//            if(strBuf != 0) strBuf[len++] = flag;
//			  else m_uart_out(flag);
//        }
        if((FormatFlags&FORMAT_FLAG_PAD_ZERO) == FORMAT_FLAG_PAD_ZERO) //! 默认空格填充或者0字符填充
        {
            inserChar = '0';
        }
        if((FormatFlags&FORMAT_FLAG_PRINT_SIGN) != FORMAT_FLAG_PRINT_SIGN) //! 非右对齐即左对齐
        {
            justify = 1;
        }
        for(width = 0; width < (int)FieldWidth; width++)
        {/** 顺序格式化 */
            shift = (unsigned)(FieldWidth-width-1);
            if(shift > (unsigned)i)
            {
                if(((FormatFlags&FORMAT_FLAG_PRINT_SIGN) == FORMAT_FLAG_PRINT_SIGN) ||  //! 右对齐
                    (((FormatFlags&FORMAT_FLAG_PRINT_SIGN) != FORMAT_FLAG_PRINT_SIGN) && (inserChar == '0'))) //! 左对齐补0处理
                {
                    if((flag == '-') && (inserChar == '0'))
                    {/** 防止‘-’在0中间出现 */
                        if(strBuf != 0) strBuf[len++] = flag;
                        else m_uart_out(flag);
                        flag = '+'; //! 对齐后再添加‘-’号
                    }
                    if(strBuf != 0) strBuf[len++] = inserChar;
                    else m_uart_out(inserChar);
                }
                else if(((FormatFlags&FORMAT_FLAG_PRINT_SIGN) != FORMAT_FLAG_PRINT_SIGN) && (inserChar != '0'))
                {
                    justify++;
                }
            }
            else
            {
                if(flag == '-')
                {
                    if(strBuf != 0) strBuf[len++] = flag;
                    else m_uart_out(flag);
                    flag = '+'; //! 对齐后再添加‘-’号
                }
                if(strBuf != 0) strBuf[len++] = tmpBuf[shift];
                else m_uart_out(tmpBuf[shift]);
            }
        }
        for(i = 0; i < (justify-1); i++)
        {
            if(strBuf != 0) strBuf[len++] = ' '; //! 左对齐，非0补齐时，则用空格在末尾填充
            else m_uart_out(' ');
        }
        break;
      case 'u':
        v = va_arg(*pParamList, int);
        i = 0;
        inserChar = ' ';
        justify = 0;
        uTmpVal = (unsigned)v;
        do/** 注意格式化最长顺序为64个字符 */
        {/** 倒叙格式化 */
            tmpBuf[i++] = hexCharTbl[0][uTmpVal%10];
            uTmpVal /= 10;
        } while(uTmpVal != 0);
        FieldWidth = (FieldWidth>(unsigned)i)?FieldWidth:(unsigned)i; //! 取最长位域
        i--;
        if((FormatFlags&FORMAT_FLAG_PAD_ZERO) == FORMAT_FLAG_PAD_ZERO) //! 默认空格填充或者0字符填充
        {
            inserChar = '0';
        }
        if((FormatFlags&FORMAT_FLAG_PRINT_SIGN) != FORMAT_FLAG_PRINT_SIGN) //! 非右对齐即左对齐
        {
            justify = 1;
        }
        for(width = 0; width < (int)FieldWidth; width++)
        {/** 顺序格式化 */
            shift = (unsigned)(FieldWidth-width-1);
            if(shift > (unsigned)i)
            {
                if(((FormatFlags&FORMAT_FLAG_PRINT_SIGN) == FORMAT_FLAG_PRINT_SIGN) ||  //! 右对齐
                    (((FormatFlags&FORMAT_FLAG_PRINT_SIGN) != FORMAT_FLAG_PRINT_SIGN) && (inserChar == '0'))) //! 左对齐补0处理
                {
                    if(strBuf != 0) strBuf[len++] = inserChar;
                    else m_uart_out(inserChar);
                }
                else if(((FormatFlags&FORMAT_FLAG_PRINT_SIGN) != FORMAT_FLAG_PRINT_SIGN) && (inserChar != '0'))
                {
                    justify++;
                }
            }
            else
            {
                if(strBuf != 0) strBuf[len++] = tmpBuf[shift];
                else m_uart_out(tmpBuf[shift]);
            }
        }
        for(i = 0; i < (justify-1); i++)
        {
            if(strBuf != 0) strBuf[len++] = ' '; //! 左对齐，右边不用0补齐
            else m_uart_out(' ');
        }
        break;
      case 'x':
      case 'X':
        v = va_arg(*pParamList, int);
        i = 0;
        inserChar = ' ';
        justify = 0;
        letter = 0;
        uTmpVal = (unsigned)v;
        if(c == 'x')
        {
            letter = 1;
        }
        do
        {
            i++;
            uTmpVal >>= 4;
        }while(uTmpVal != 0);
        FieldWidth = (FieldWidth>(unsigned)i)?FieldWidth:(unsigned)i; //! 取最长位域
        i--;
        if((FormatFlags&FORMAT_FLAG_PAD_ZERO) == FORMAT_FLAG_PAD_ZERO) //! 默认空格填充或者0字符填充
        {
            inserChar = '0';
        }
        if((FormatFlags&FORMAT_FLAG_PRINT_SIGN) != FORMAT_FLAG_PRINT_SIGN) //! 非右对齐即左对齐
        {
            justify = 1;
        }
        for(width = 0; width < (int)FieldWidth; width++)
        {
            shift = (unsigned)(FieldWidth-width-1)*4;
            if(shift > (unsigned)i*4)
            {
                if(((FormatFlags&FORMAT_FLAG_PRINT_SIGN) == FORMAT_FLAG_PRINT_SIGN) ||  //! 右对齐
                    (((FormatFlags&FORMAT_FLAG_PRINT_SIGN) != FORMAT_FLAG_PRINT_SIGN) && (inserChar == '0'))) //! 左对齐补0处理
                {
                    if(strBuf != 0) strBuf[len++] = inserChar;
                    else m_uart_out(inserChar);
                }
                else if(((FormatFlags&FORMAT_FLAG_PRINT_SIGN) != FORMAT_FLAG_PRINT_SIGN) && (inserChar != '0'))
                {
                    justify++;
                }
            }
            else
            {
                if(strBuf != 0) strBuf[len++] = (unsigned)(hexCharTbl[letter][(v>>shift)&0x0000000F]);
                else m_uart_out(hexCharTbl[letter][(v>>shift)&0x0000000F]);
            }
        }
        for(i = 0; i < (justify-1); i++)
        {
            if(strBuf != 0) strBuf[len++] = ' '; //! 左对齐，右边不用0补齐
            else m_uart_out(' ');
        }
//        for(width = 0; width < FieldWidth; width++)
//        {
//            shift = (unsigned)(FieldWidth-width-1)*4;
//            if(strBuf != 0) strBuf[len++] = (unsigned)(hexCharTbl[0][(v>>shift)&0x0000000F]);
//			  else m_uart_out(hexCharTbl[0][(v>>shift)&0x0000000F]);
//        }
        break;
      case 's':
        {
          const char * s = va_arg(*pParamList, const char *);
          do {
            c = *s;
            s++;
            if (c == '\0') {
              break;
            }
            if(strBuf != 0) strBuf[len++] = c;
			else m_uart_out(c);
          } while(1);
        }
        break;
      case 'p':
      case 'P':
        v = va_arg(*pParamList, int);
        i = 0;
        letter = 0;
        uTmpVal = (unsigned)v;
        if(c == 'p')
        {
            letter = 1;
        }
        do
        {
            i++;
            uTmpVal >>= 4;
        }while(uTmpVal != 0);
        FieldWidth = (FieldWidth>(unsigned)i)?FieldWidth:(unsigned)i; //! 取最长位域
        for(width = 0; width < (int)FieldWidth; width++)
        {
            shift = (unsigned)(FieldWidth-width-1)*4;
            if(strBuf != 0) strBuf[len++] = (unsigned)(hexCharTbl[letter][(v>>shift)&0x0000000F]);
			else m_uart_out(hexCharTbl[letter][(v>>shift)&0x0000000F]);
        }
        break;
      #ifdef USE_32BIT_MCU
      case 'f': //! float实际精度只有3位小数，如果需要精确度更高，则建议传入double型，此处限定最大6位小数，另外整数部分绝对值也不适宜很大，否则小数部分会被挤掉；总之使用浮点打印需要十分注意！！！
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
      #endif
      case '%':
        if(strBuf != 0) strBuf[len++] = '%';
		else m_uart_out('%');
        break;
      default:
        break;
      }
      sFormat++;
    } else {
        if(strBuf != 0) strBuf[len++] = c;
        else m_uart_out(c);
    }
  } while(1);

  return len;
}

int m_printf(const char * sFormat, ...)
{
	return 0;
    int r = 0;
    va_list ParamList;

    va_start(ParamList, sFormat);
    r = va_arg(ParamList, int); //! 测试发现，参数列表的第一个参数是乱码，此处需要做一次读操作相当于去掉第一个参数！！！
    r = m_vprintf(0, sFormat, &ParamList);
    va_end(ParamList);
    return r;
}

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

#if 0 //! 本平台及支持16位及以下数据类型
void m_printf_test(void) {   
  unsigned char cnt = 0;
  static volatile int _Cnt;
  //signed char val1 = -125;
//  unsigned char val2 = 125;
//  unsigned char buf[] = {"maliwen"};
//  unsigned short hexV = 0x56e;
  //float vPi = -123.141592465;
 
  #if 0
  m_printf("%02d test: pi = %f, val1 = %d, buf(%p) = %s, hexV = 0x%05X\r\n", cnt++, vPi, val1, buf, buf,hexV);
  m_printf("%02d test: pi = %3.12f, val1 = %d, buf(%P) = %s, hexV = 0x%05X\r\n", cnt++, vPi, val1, buf, buf,hexV);
  m_printf("%02d test: pi = %3.12f, val1 = %d, buf(%p) = %s, hexV = 0x%05X\r\n", cnt++, vPi, val1, buf, buf,hexV);
  m_printf("%02d test: pi = %3.12f, pi = %3.12f, pi = %3.12f, val1 = %d, buf(%P) = %s, hexV = 0x%05X\r\n", cnt++, vPi, vPi, vPi, val1, buf, buf,hexV);
  #endif
  m_printf("%s\r\n", "maliwen test at 20190804 1725");
  m_printf("%s\r\n", "maliwen");
  m_printf("%-10s:%s\r\n", "name", "maliwen");
  m_printf("%+10s:%s\r\n", "name", "maliwen");
  
  m_printf("%02d SEGGER Real-Time-Terminal Sample\r\n\r\n", cnt++);
  m_printf("%02d ###### Testing SEGGER_printf() ######\r\n", cnt++);
  
  m_printf("%02d printf Test: %%x,      0x1ABC : %x.\r\n", cnt++, 0x1ABC);
  m_printf("%02d printf Test: %%+x,     0x1ABC : %+x.\r\n", cnt++, 0x1ABC);
  m_printf("%02d printf Test: %%.3x,    0x1ABC : %.3x.\r\n", cnt++, 0x1ABC);
  m_printf("%02d printf Test: %%.6x,    0x1ABC : %.6x.\r\n", cnt++, 0x1ABC);

  m_printf("%x.\r\n", 0x23aB);
  m_printf("%X.\r\n", 0x23aB);
  m_printf("%x.\r\n", 0x123aB);
  m_printf("%X.\r\n", 0x123aB);
  m_printf("%x.\r\n", 0x123aBc);
  m_printf("%X.\r\n", 0x123aBc);
  m_printf("%+02d printf Test: %%-.3x,    0x123aB : %-.3x.\r\n", cnt++, 0x123aB);
  m_printf("%+02d printf Test: %%-0.3X,   0x123aB : %-0.3X.\r\n", cnt++, 0x123aB);
  m_printf("%+02d printf Test: %%+.3x,    0x123aB : %+.3x.\r\n", cnt++, 0x123aB);
  m_printf("%+02d printf Test: %%+0.3X,   0x123aB : %+0.3X.\r\n", cnt++, 0x123aB);
  
  #if 0
  m_printf("%+02d printf Test: %%4.5f,     123.45 : %4.5f.\r\n", cnt++, 123.45);
  m_printf("%+02d printf Test: %%04.5f,    123.45 : %04.5f.\r\n", cnt++, 123.45);
  m_printf("%+02d printf Test: %%-4.5f,    123.45 : %-4.5f.\r\n", cnt++, 123.45);
  m_printf("%+02d printf Test: %%-04.5f,   123.45 : %-04.5f.\r\n", cnt++, 123.45);
  m_printf("%+02d printf Test: %%+8.5f,    123.45 : %+8.5f.\r\n", cnt++, 123.45);
  m_printf("%+02d printf Test: %%+08.5f,   123.45 : %+08.5f.\r\n", cnt++, 123.45);
  m_printf("%+02d printf Test: %%8.3f,     123.45 : %8.3f.\r\n", cnt++, 123.45);
  m_printf("%+02d printf Test: %%08.3f,    123.45 : %08.3f.\r\n", cnt++, 123.45);
  m_printf("%+02d printf Test: %%-8.3f,    123.45 : %-8.3f.\r\n", cnt++, 123.45);
  m_printf("%+02d printf Test: %%-08.3f,   123.45 : %-08.3f.\r\n", cnt++, 123.45);
  m_printf("%+02d printf Test: %%+8.3f,    123.45 : %+8.3f.\r\n", cnt++, 123.45);
  m_printf("%+02d printf Test: %%+08.3f,   123.45 : %+08.3f.\r\n", cnt++, 123.45);
  m_printf("%+02d printf Test: %%8.3f,     -123.45 : %8.3f.\r\n", cnt++, -123.45);
  m_printf("%+02d printf Test: %%08.3f,    -123.45 : %08.3f.\r\n", cnt++, -123.45);
  m_printf("%+02d printf Test: %%-8.3f,    -123.45 : %-8.3f.\r\n", cnt++, -123.45);
  m_printf("%+02d printf Test: %%-08.3f,   -123.45 : %-08.3f.\r\n", cnt++, -123.45);
  m_printf("%+02d printf Test: %%+8.3f,    -123.45 : %+8.3f.\r\n", cnt++, -123.45);
  m_printf("%+02d printf Test: %%+08.3f,   -123.45 : %+08.3f.\r\n", cnt++, -123.45);
  #endif

  m_printf("%02d printf Test: %%c,         'S' : %c.\r\n", cnt++, 'S');
  m_printf("%02d printf Test: %%5c,        'E' : %5c.\r\n", cnt++, 'E');
  m_printf("%02d printf Test: %%-5c,       'G' : %-5c.\r\n", cnt++, 'G');
  m_printf("%02d printf Test: %%5.3c,      'G' : %-5c.\r\n", cnt++, 'G');
  m_printf("%02d printf Test: %%.3c,       'E' : %-5c.\r\n", cnt++, 'E');
  m_printf("%02d printf Test: %%c,         'R' : %c.\r\n", cnt++, 'R');

  m_printf("%02d printf Test: %%s,      \"RTT\" : %s.\r\n", cnt++, "RTT");
  m_printf("%02d printf Test: %%s, \"RTT\\r\\nRocks.\" : %s.\r\n", cnt++, "RTT\r\nRocks.");

  m_printf("%+02d %%d %d, %8.3d, %2.4d, %d, %10.6d.\r\n", cnt++, 12345, 12345, 12345, 12345, 12345);
  m_printf("%+02d %%d %d, %8.3d, %2.4d, %d, %10.6d.\r\n", cnt++, -12345, -12345, -12345, -12345, -12345);
  m_printf("%+02d %%u %u, %8.3u, %2.4u, %u, %10.6u.\r\n", cnt++, 12345, 12345, 12345, 12345, 12345);
  m_printf("%+02d %%u %u, %8.3u, %2.4u, %u, %10.6u.\r\n", cnt++, -12345, -12345, -12345, -12345, -12345);
  
  m_printf("%+02d printf Test: %%8.3d,    12345 : %8.3d.\r\n", cnt++, 12345);
  m_printf("%+02d printf Test: %%08.3d,   12345 : %08.3d.\r\n", cnt++, 12345);
  m_printf("%+02d printf Test: %%-8.3d,   12345 : %-8.3d.\r\n", cnt++, 12345);
  m_printf("%+02d printf Test: %%-08.3d,  12345 : %-08.3d.\r\n", cnt++, 12345);
  m_printf("%+02d printf Test: %%+8.3d,   12345 : %+8.3d.\r\n", cnt++, 12345);
  m_printf("%+02d printf Test: %%+08.3d,  12345 : %+08.3d.\r\n", cnt++, 12345);
  m_printf("%+02d printf Test: %%8.3d,    -12345 : %8.3d.\r\n", cnt++, -12345);
  m_printf("%+02d printf Test: %%08.3d,   -12345 : %08.3d.\r\n", cnt++, -12345);
  m_printf("%+02d printf Test: %%-8.3d,   -12345 : %-8.3d.\r\n", cnt++, -12345);
  m_printf("%+02d printf Test: %%-08.3d,  -12345 : %-08.3d.\r\n", cnt++, -12345);
  m_printf("%+02d printf Test: %%+8.3d,   -12345 : %+8.3d.\r\n", cnt++, -12345);
  m_printf("%+02d printf Test: %%+08.3d,  -12345 : %+08.3d.\r\n", cnt++, -12345);
  
  m_printf("%+02d printf Test: %%8.3u,    12345 : %8.3u.\r\n", cnt++, 12345);
  m_printf("%+02d printf Test: %%08.3u,   12345 : %08.3u.\r\n", cnt++, 12345);
  m_printf("%+02d printf Test: %%-8.3u,   12345 : %-8.3u.\r\n", cnt++, 12345);
  m_printf("%+02d printf Test: %%-08.3u,  12345 : %-08.3u.\r\n", cnt++, 12345);
  m_printf("%+02d printf Test: %%+8.3u,   12345 : %+8.3u.\r\n", cnt++, 12345);
  m_printf("%+02d printf Test: %%+08.3u,  12345 : %+08.3u.\r\n", cnt++, 12345);
  
  m_printf("%+02d printf Test: %%8.3x,     0x123aB : %8.3x.\r\n", cnt++, 0x123aB);
  m_printf("%+02d printf Test: %%08.3X,    0x123aB : %08.3X.\r\n", cnt++, 0x123aB);
  m_printf("%+02d printf Test: %%-8.3x,    0x123aB : %-8.3x.\r\n", cnt++, 0x123aB);
  m_printf("%+02d printf Test: %%-08.3X,   0x123aB : %-08.3X.\r\n", cnt++, 0x123aB);
  m_printf("%+02d printf Test: %%+8.3x,    0x123aB : %+8.3x.\r\n", cnt++, 0x123aB);
  m_printf("%+02d printf Test: %%+08.3X,   0x123aB : %+08.3X.\r\n", cnt++, 0x123aB);
  
  #if 0
  m_printf("%02d printf Test: %%f,      -1234.5 : %f.\r\n", cnt++, -1234.5);
  m_printf("%02d printf Test: %%+f,     -123.45 : %+f.\r\n", cnt++, -123.45);
  m_printf("%02d printf Test: %%.3f,    -12.345 : %.3f.\r\n", cnt++, -12.345);
  m_printf("%02d printf Test: %%.6f,    -1.2345 : %.6f.\r\n", cnt++, -1.2345);
  m_printf("%02d printf Test: %%6.3f,   -1234.5 : %6.3f.\r\n", cnt++, -1234.5);
  m_printf("%02d printf Test: %%8.6f,   -123.45 : %8.6f.\r\n", cnt++, -123.45);
  m_printf("%02d printf Test: %%08f,    -12.345 : %08f.\r\n", cnt++, -12.345);
  m_printf("%02d printf Test: %%08.6f,  -1.2345 : %08.6f.\r\n", cnt++, -1.2345);
  m_printf("%02d printf Test: %%0f,     -1234.5 : %0f.\r\n", cnt++, -1234.5);
  m_printf("%02d printf Test: %%-.6f,   -123.45 : %-.6f.\r\n", cnt++, -123.45);
  m_printf("%02d printf Test: %%-6.3f,  -12.345 : %-6.3f.\r\n", cnt++, -12.345);
  m_printf("%02d printf Test: %%-8.6f,  -1.2345 : %-8.6f.\r\n", cnt++, -1.2345);
  m_printf("%02d printf Test: %%-08f,   -1234.5 : %-08f.\r\n", cnt++, -1234.5);
  m_printf("%02d printf Test: %%-08.6f, -123.45 : %-08.6f.\r\n", cnt++, -123.45);
  m_printf("%02d printf Test: %%-0f,    -12.345 : %-0f.\r\n", cnt++, -12.345);

  m_printf("%02d printf Test: %%f,      1234.5 : %f.\r\n", cnt++, 1234.5);
  m_printf("%02d printf Test: %%+f,     123.45 : %+f.\r\n", cnt++, 123.45);
  m_printf("%02d printf Test: %%.3f,    12.345 : %.3f.\r\n", cnt++, 12.345);
  m_printf("%02d printf Test: %%.6f,    1.2345 : %.6f.\r\n", cnt++, 1.2345);
  m_printf("%02d printf Test: %%6.3f,   1234.5 : %6.3f.\r\n", cnt++, 1234.5);
  m_printf("%02d printf Test: %%8.6f,   123.45 : %8.6f.\r\n", cnt++, 123.45);
  m_printf("%02d printf Test: %%08f,    12.345 : %08f.\r\n", cnt++, 12.345);
  m_printf("%02d printf Test: %%08.6f,  1.2345 : %08.6f.\r\n", cnt++, 1.2345);
  m_printf("%02d printf Test: %%0f,     1234.5 : %0f.\r\n", cnt++, 1234.5);
  m_printf("%02d printf Test: %%-.6f,   123.45 : %-.6f.\r\n", cnt++, 123.45);
  m_printf("%02d printf Test: %%-6.3f,  12.345 : %-6.3f.\r\n", cnt++, 12.345);
  m_printf("%02d printf Test: %%-8.6f,  1.2345 : %-8.6f.\r\n", cnt++, 1.2345);
  m_printf("%02d printf Test: %%-08f,   1234.5 : %-08f.\r\n", cnt++, 1234.5);
  m_printf("%02d printf Test: %%-08.6f, 123.45 : %-08.6f.\r\n", cnt++, 123.45);
  m_printf("%02d printf Test: %%-0f,    12.345 : %-0f.\r\n", cnt++, 12.345);
  #endif

  m_printf("%02d printf Test: %%u,       12345 : %u.\r\n", cnt++, 12345);
  m_printf("%02d printf Test: %%+u,      12345 : %+u.\r\n", cnt++, 12345);
  m_printf("%02d printf Test: %%.3u,     12345 : %.3u.\r\n", cnt++, 12345);
  m_printf("%02d printf Test: %%.6u,     12345 : %.6u.\r\n", cnt++, 12345);
  m_printf("%02d printf Test: %%6.3u,    12345 : %6.3u.\r\n", cnt++, 12345);
  m_printf("%02d printf Test: %%8.6u,    12345 : %8.6u.\r\n", cnt++, 12345);
  m_printf("%02d printf Test: %%08u,     12345 : %08u.\r\n", cnt++, 12345);
  m_printf("%02d printf Test: %%08.6u,   12345 : %08.6u.\r\n", cnt++, 12345);
  m_printf("%02d printf Test: %%0u,      12345 : %0u.\r\n", cnt++, 12345);
  m_printf("%02d printf Test: %%-.6u,    12345 : %-.6u.\r\n", cnt++, 12345);
  m_printf("%02d printf Test: %%-6.3u,   12345 : %-6.3u.\r\n", cnt++, 12345);
  m_printf("%02d printf Test: %%-8.6u,   12345 : %-8.6u.\r\n", cnt++, 12345);
  m_printf("%02d printf Test: %%-08u,    12345 : %-08u.\r\n", cnt++, 12345);
  m_printf("%02d printf Test: %%-08.6u,  12345 : %-08.6u.\r\n", cnt++, 12345);
  m_printf("%02d printf Test: %%-0u,     12345 : %-0u.\r\n", cnt++, 12345);

  m_printf("%02d printf Test: %%u,      -12345 : %u.\r\n", cnt++, -12345);
  m_printf("%02d printf Test: %%+u,     -12345 : %+u.\r\n", cnt++, -12345);
  m_printf("%02d printf Test: %%.3u,    -12345 : %.3u.\r\n", cnt++, -12345);
  m_printf("%02d printf Test: %%.6u,    -12345 : %.6u.\r\n", cnt++, -12345);
  m_printf("%02d printf Test: %%6.3u,   -12345 : %6.3u.\r\n", cnt++, -12345);
  m_printf("%02d printf Test: %%8.6u,   -12345 : %8.6u.\r\n", cnt++, -12345);
  m_printf("%02d printf Test: %%08u,    -12345 : %08u.\r\n", cnt++, -12345);
  m_printf("%02d printf Test: %%08.6u,  -12345 : %08.6u.\r\n", cnt++, -12345);
  m_printf("%02d printf Test: %%0u,     -12345 : %0u.\r\n", cnt++, -12345);
  m_printf("%02d printf Test: %%-.6u,   -12345 : %-.6u.\r\n", cnt++, -12345);
  m_printf("%02d printf Test: %%-6.3u,  -12345 : %-6.3u.\r\n", cnt++, -12345);
  m_printf("%02d printf Test: %%-8.6u,  -12345 : %-8.6u.\r\n", cnt++, -12345);
  m_printf("%02d printf Test: %%-08u,   -12345 : %-08u.\r\n", cnt++, -12345);
  m_printf("%02d printf Test: %%-08.6u, -12345 : %-08.6u.\r\n", cnt++, -12345);
  m_printf("%02d printf Test: %%-0u,    -12345 : %-0u.\r\n", cnt++, -12345);

  m_printf("%02d printf Test: %%d,      -12345 : %d.\r\n", cnt++, -12345);
  m_printf("%02d printf Test: %%+d,     -12345 : %+d.\r\n", cnt++, -12345);
  m_printf("%02d printf Test: %%.3d,    -12345 : %.3d.\r\n", cnt++, -12345);
  m_printf("%02d printf Test: %%.6d,    -12345 : %.6d.\r\n", cnt++, -12345);
  m_printf("%02d printf Test: %%6.3d,   -12345 : %6.3d.\r\n", cnt++, -12345);
  m_printf("%02d printf Test: %%8.6d,   -12345 : %8.6d.\r\n", cnt++, -12345);
  m_printf("%02d printf Test: %%08d,    -12345 : %08d.\r\n", cnt++, -12345);
  m_printf("%02d printf Test: %%08.6d,  -12345 : %08.6d.\r\n", cnt++, -12345);
  m_printf("%02d printf Test: %%0d,     -12345 : %0d.\r\n", cnt++, -12345);
  m_printf("%02d printf Test: %%-.6d,   -12345 : %-.6d.\r\n", cnt++, -12345);
  m_printf("%02d printf Test: %%-6.3d,  -12345 : %-6.3d.\r\n", cnt++, -12345);
  m_printf("%02d printf Test: %%-8.6d,  -12345 : %-8.6d.\r\n", cnt++, -12345);
  m_printf("%02d printf Test: %%-08d,   -12345 : %-08d.\r\n", cnt++, -12345);
  m_printf("%02d printf Test: %%-08.6d, -12345 : %-08.6d.\r\n", cnt++, -12345);
  m_printf("%02d printf Test: %%-0d,    -12345 : %-0d.\r\n", cnt++, -12345);

  m_printf("%02d printf Test: %%x,      0x1234ABC : %x.\r\n", cnt++, 0x1234ABC);
  m_printf("%02d printf Test: %%+x,     0x1234ABC : %+x.\r\n", cnt++, 0x1234ABC);
  m_printf("%02d printf Test: %%.3x,    0x1234ABC : %.3x.\r\n", cnt++, 0x1234ABC);
  m_printf("%02d printf Test: %%.6x,    0x1234ABC : %.6x.\r\n", cnt++, 0x1234ABC);
  m_printf("%02d printf Test: %%6.3x,   0x1234ABC : %6.3x.\r\n", cnt++, 0x1234ABC);
  m_printf("%02d printf Test: %%8.6x,   0x1234ABC : %8.6x.\r\n", cnt++, 0x1234ABC);
  m_printf("%02d printf Test: %%08x,    0x1234ABC : %08x.\r\n", cnt++, 0x1234ABC);
  m_printf("%02d printf Test: %%08.6x,  0x1234ABC : %08.6x.\r\n", cnt++, 0x1234ABC);
  m_printf("%02d printf Test: %%0x,     0x1234ABC : %0x.\r\n", cnt++, 0x1234ABC);
  m_printf("%02d printf Test: %%-.6x,   0x1234ABC : %-.6x.\r\n", cnt++, 0x1234ABC);
  m_printf("%02d printf Test: %%-6.3x,  0x1234ABC : %-6.3X.\r\n", cnt++, 0x1234ABC);
  m_printf("%02d printf Test: %%-8.6x,  0x1234ABC : %-8.6X.\r\n", cnt++, 0x1234ABC);
  m_printf("%02d printf Test: %%-08x,   0x1234ABC : %-08X.\r\n", cnt++, 0x1234ABC);
  m_printf("%02d printf Test: %%-08.6x, 0x1234ABC : %-08.6X.\r\n", cnt++, 0x1234ABC);
  m_printf("%02d printf Test: %%-0x,    0x1234ABC : %-0x.\r\n", cnt++, 0x1234ABC);

  m_printf("%02d printf Test: %%p,      &_Cnt      : %p.\r\n", cnt++, &_Cnt);

  m_printf("%02d ###### SEGGER_printf() Tests done. ######\r\n", cnt++);
}
#endif
