#include <debug.h>          /* Simple host interface to the UART driver */
#include "../adapter.h"
#include <buf_utils.h>
#include <mem.h>
#include "serial_service.h"

#define USER_STORAGE_START_OFFSET               (0x3F)

#define USER_STORATE_INIT_FLAG_OFFSET           (USER_STORAGE_START_OFFSET) /* flag to indicate to initialize the bellow nvm storage */
#define USER_STORATE_INIT_FLAG_LENGTH           (0x01)

#if USE_PARAM_STORE
#define MOTORS_CURRENT_POSITION_OFFSET          (USER_STORATE_INIT_FLAG_OFFSET+USER_STORATE_INIT_FLAG_LENGTH)
#define MOTORS_CURRENT_POSITION_LENGTH          (0x06)

#define MOTORS_ZERO_POSITION_POLARITY_OFFSET    (MOTORS_CURRENT_POSITION_OFFSET+MOTORS_CURRENT_POSITION_LENGTH)
#define MOTORS_ZERO_POSITION_POLARITY_LENGTH    (0x06)

#define PAIRING_CODE_OFFSET                     (MOTORS_ZERO_POSITION_POLARITY_OFFSET+MOTORS_ZERO_POSITION_POLARITY_LENGTH)
#define PAIRING_CODE_LENGTH                     (0x02)

#define SYSTEM_DATE_TIME_OFFSET                 (PAIRING_CODE_OFFSET+PAIRING_CODE_LENGTH)
#define SYSTEM_DATE_TIME_LENGTH                 (0x07)/*save system date time every 15 ninutes*/

#define ALARM_CLOCK_OFFSET                      (SYSTEM_DATE_TIME_OFFSET+SYSTEM_DATE_TIME_LENGTH)
#define ALARM_CLOCK_LENGTH                      (20)/*tatal 5 alarm clocks*/
#define ALARM_CLOCK_SINGLE_LENGTH               (0x04)

#define DISPLAY_SETTING_OFFSET                  (ALARM_CLOCK_OFFSET+ALARM_CLOCK_LENGTH)
#define DISPLAY_SETTING_LENGTH                  (0x4)

#define PERSONAL_INFO_OFFSET                    (DISPLAY_SETTING_OFFSET+DISPLAY_SETTING_LENGTH)
#define PERSONAL_INFO_LENGTH                    (12)

#define SPORT_SETTING_OFFSET                    (PERSONAL_INFO_OFFSET+PERSONAL_INFO_LENGTH)
#define SPORT_SETTING_LENGTH                    (0x08)

#define HISTORY_CONTROL_OFFSET                  (SPORT_SETTING_OFFSET+SPORT_SETTING_LENGTH)
#define HISTORY_CONTROL_LENGTH                  (sizeof(his_ctrl_t))//(4)//(sizeof(his_ctrl_t))/* word width */
#else
#define HISTORY_CONTROL_OFFSET                  (USER_STORATE_INIT_FLAG_OFFSET+USER_STORATE_INIT_FLAG_LENGTH)
#define HISTORY_CONTROL_LENGTH                  (sizeof(his_ctrl_t))//(4)//(sizeof(his_ctrl_t))/* word width */
#endif

#define CONST_RING_BUFFER_LENGTH                (31+1)/* unit: day(1 byte more than acturlly need) */
#define CONST_DATA_ONEDAY_LENGTH                (sizeof(his_data_t))//(8)//(sizeof(his_data_t))
#define HISTORY_DATA_OFFSET                     (HISTORY_CONTROL_OFFSET+HISTORY_CONTROL_LENGTH)
#define HISTORY_DATA_LENGTH                     (CONST_DATA_ONEDAY_LENGTH*CONST_RING_BUFFER_LENGTH)/* store 20 days's history */

#define USER_STORAGE_TOTAL_LENGTH               (HISTORY_DATA_LENGTH+(HISTORY_DATA_OFFSET-USER_STORAGE_START_OFFSET))

/* sleep/sport history data storage model
 * |----------------------------------------|
 * |           ~~~~control block~~~~        |
 * |----------------------------------------|
 * |    history data store head pointer     | 1 byte
 * |----------------------------------------|
 * |    history data store tail pointer     | 1 byte
 * |----------------------------------------|
 * |        15-minute data index            | 1 byte
 * |----------------------------------------|
 * |             date index                 | 1 byte
 * |----------------------------------------|
 * |            ~~~~data block~~~~          |
 * |--------|----------------------|--------|
 * |        | day1  date & time    |4 bytes |
 * |        |----------------------|        |
 * |        |        sport         |2 byte  |
 * |        |----------------------|        |
 * |        |       reserve        |1 byte  |
 * |        |----------------------|        |
 * |        |        sleep         |1 byte  |
 * |        |----------------------|        |
 * |        |   ...     ...        |        |
 * |        |----------------------|        |
 * |        | day20  date & time   |        |
 * |        |----------------------|        |
 * |        |        sport         |        |
 * |        |----------------------|        |
 * |        |        sleep         |        |
 * |        |----------------------|        |
 * |        |       reserve        |        |
 * |--------|----------------------|--------|
 * |          ~~~~storage calc~~~~          |
 * |----------------------------------------|
 * | control-byte = 1+1+1+1 = 4 bytes       |
 * | day-byte = 4+2+1+1 = 8 bytes           |
 * | total = 4+8*20day = 164bytes = 82words |
 * | total = 4+8*31day = 252bytes = 126words|
 * |----------------------------------------|
 *
 *          EEPROM
 * |------------------------| 0x0000
 * |                        |
 * |      Bootloader        |
 * |                        |
 * |------------------------| 0x4100
 * |                        |
 * |       NVM Store        |
 * |                        |
 * |------------------------| Slot 1 Address(0x4500)
 * |                        |
 * |                        |
 * |     Application 1      |
 * |                        |
 * |                        |
 * |------------------------| Slot 2 Address(if present)
 * |                        |
 * |                        |
 * |     Application 2      |
 * |                        |
 * |                        |
 * |------------------------| Slot End Address(0x10000)
 *
 * 1. 内存空间分布
 *          EEPROM 
 *     �5536字节�4K�
 * |------------------------| 0x0000
 * |                        |
 * |      Bootloader        | 
 * |  16640字节�6.25K�      |
 * |------------------------| 0x4100
 * |                        |
 * |       NVM Store        | 
 * |      1024字节�K�       |
 * |------------------------| Slot 1 Address(0x4500)
 * |                        |
 * |                        |
 * |     Application        | 
 * |   47872字节�6.75K�     |
 * |      �xbb00 �        |
 * |------------------------| Slot End Address(0x10000)
 * 
 * 2. 编译输出信息
 *  Words: code=19357, data=3251, free=5792 decimal
 *  编译提示剩余5792words=11584字节=11.3125K
 *
 * 3. 实际内存占用
 *   占用：（19357+3251�2 = 45216字节�44.15625K�
 *   剩余�7872 - 45216 = 2656字节 = 2.59375K
 *
 *
 * |\---------------/|
 * | \   19 |  0   / |
 * |  \-----------/  |
 * |...|  ring   | 1 |
 * |---| buffer  |---|
 * | 5 |         | 2 |
 * |  /-----------\  |
 * | /   4  |  3   \ |
 * |/---------------\|
*/

void nvm_read(u16 *buffer, u16 length, u16 offset);
void nvm_write(u16 *buffer, u16 length, u16 offset);
s16 nvm_write_history_data(u16 *buffer, u8 index);

void nvm_read(u16 *buffer, u16 length, u16 offset)
{
    get_driver()->flash->flash_read(buffer, length, offset);
}
void nvm_write(u16 *buffer, u16 length, u16 offset)
{
    get_driver()->flash->flash_write(buffer, length, offset);
}

static u8 panic_check(u8 caller)
{
    u8 hexCharTbl[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    u8 rd_err[12] = {"read err:"};
    u8 wr_err[12] = {"writ err:"};
    u8 panic_code = panic_get();
    
    if(panic_code == 0x06) {
        rd_err[9] = hexCharTbl[(caller>>4)&0x0F];
        rd_err[10] = hexCharTbl[caller&0x0F];
        print(rd_err, 11);
        return 1;
    } else if(panic_code == 0x07) {
        wr_err[9] = hexCharTbl[(caller>>4)&0x0F];
        wr_err[10] = hexCharTbl[caller&0x0F];
        print(wr_err, 11);
        return 1;
    }
    return 0;
}
s16 nvm_check_storage_init(void)
{
    volatile u16 erase_offset = 0;
    u16 erase_value = 0;
    u16 init_flag = 0;

    nvm_read((u16*)&init_flag, USER_STORATE_INIT_FLAG_LENGTH, USER_STORATE_INIT_FLAG_OFFSET);
    if(panic_check(0xFF) != 0) return 1;

    if(init_flag == 0xA55A)
    {
        return 1; 
    }

    for(erase_offset = 0; erase_offset < USER_STORAGE_TOTAL_LENGTH; erase_offset++)
    {
        nvm_write(&erase_value, 1, USER_STORAGE_START_OFFSET+erase_offset);
        if(panic_check(erase_offset) != 0) return 1;
    }

    // print the initialized nvm contents
//    for(erase_offset = 0; erase_offset < USER_STORAGE_TOTAL_LENGTH; erase_offset++)
//    {
//        nvm_read(&erase_value, 1, USER_STORAGE_START_OFFSET+erase_offset);
//        if(panic_check(erase_offset) != 0) return 1;
//        get_driver()->uart->uart_write((u8*)&erase_value, 1);
//    }

    init_flag = 0xA55A;
    nvm_write((u16*)&init_flag, USER_STORATE_INIT_FLAG_LENGTH, USER_STORATE_INIT_FLAG_OFFSET);/* write user storage init flag */
    if(panic_check(0xFD) != 0) return 1;

    return 0;
}
#if USE_PARAM_STORE
s16 nvm_read_motor_current_position(u16 *buffer, u8 index)
{
    nvm_read(buffer, MOTORS_CURRENT_POSITION_LENGTH, MOTORS_CURRENT_POSITION_OFFSET);

    return 0;
}
s16 nvm_write_motor_current_position(u16 *buffer, u8 index)
{ 
    nvm_write(buffer, MOTORS_CURRENT_POSITION_LENGTH, MOTORS_CURRENT_POSITION_OFFSET);   

    return 0;
}
s16 nvm_read_zero_position_polarity(u16 *buffer, u8 index)
{
    nvm_read(buffer, MOTORS_ZERO_POSITION_POLARITY_LENGTH, MOTORS_ZERO_POSITION_POLARITY_OFFSET); 

    return 0;
}
s16 nvm_write_zero_position_polarity(u16 *buffer, u8 index)
{
    nvm_write(buffer, MOTORS_ZERO_POSITION_POLARITY_LENGTH, MOTORS_ZERO_POSITION_POLARITY_OFFSET); 

    return 0; 
}
s16 nvm_read_pairing_code(u16 *buffer, u8 index)
{
    nvm_read(buffer, PAIRING_CODE_LENGTH, PAIRING_CODE_OFFSET);

    return 0;
}
s16 nvm_write_pairing_code(u16 *buffer, u8 index)
{
    nvm_write(buffer, PAIRING_CODE_LENGTH, PAIRING_CODE_OFFSET);

    return 0;
}
s16 nvm_read_date_time(u16 *buffer, u8 index)
{
    nvm_read(buffer, SYSTEM_DATE_TIME_LENGTH, SYSTEM_DATE_TIME_OFFSET);

    return 0;
}
s16 nvm_write_date_time(u16 *buffer, u8 index)
{
    nvm_write(buffer, SYSTEM_DATE_TIME_LENGTH, SYSTEM_DATE_TIME_OFFSET);

    return 0;
}
s16 nvm_read_alarm_clock_single(u16 *buffer, u8 index)
{
    nvm_read(buffer, ALARM_CLOCK_SINGLE_LENGTH, ALARM_CLOCK_OFFSET+index*ALARM_CLOCK_SINGLE_LENGTH);

    return 0; 
}
s16 nvm_write_alarm_clock_single(u16 *buffer, u8 index)
{
    nvm_write(buffer, ALARM_CLOCK_SINGLE_LENGTH, ALARM_CLOCK_OFFSET+index*ALARM_CLOCK_SINGLE_LENGTH); 

    return 0;
}
s16 nvm_read_alarm_clock_total(u16 *buffer, u8 index)
{
    nvm_read(buffer, ALARM_CLOCK_LENGTH, ALARM_CLOCK_OFFSET);

    return 0;
}
s16 nvm_write_alarm_clock_total(u16 *buffer, u8 index)
{
    nvm_write(buffer, ALARM_CLOCK_LENGTH, ALARM_CLOCK_OFFSET); 

    return 0;
}
s16 nvm_read_display_setting(u16 *buffer, u8 index)
{
    nvm_read(buffer, DISPLAY_SETTING_LENGTH, DISPLAY_SETTING_OFFSET);

    return 0;
}
s16 nvm_write_display_setting(u16 *buffer, u8 index)
{
    nvm_write(buffer, DISPLAY_SETTING_LENGTH, DISPLAY_SETTING_OFFSET);  

    return 0;
}
s16 nvm_read_personal_info(u16 *buffer, u8 index)
{
    nvm_read(buffer, PERSONAL_INFO_LENGTH, PERSONAL_INFO_OFFSET);

    return 0;
}
s16 nvm_write_personal_info(u16 *buffer, u8 index)
{
    nvm_write(buffer, PERSONAL_INFO_LENGTH, PERSONAL_INFO_OFFSET);

    return 0;
}
s16 nvm_read_history_setting(u16 *buffer, u8 index)
{
    nvm_read(buffer, SPORT_SETTING_LENGTH, SPORT_SETTING_OFFSET);

    return 0;
}
s16 nvm_write_history_setting(u16 *buffer, u8 index)
{
    nvm_write(buffer, SPORT_SETTING_LENGTH, SPORT_SETTING_OFFSET);

    return 0;
}
#endif
s16 nvm_read_history_data(u16 *buffer, u8 index)
{
    his_ctrl_t ctrl;
    u16 ready = 0;

    nvm_check_storage_init();
    nvm_read((u16*)&ctrl, HISTORY_CONTROL_LENGTH, HISTORY_CONTROL_OFFSET);
    if(panic_check(0xFB) != 0) return 1;

    if(ctrl.read_tail == ctrl.ring_buf_head) { // ring buffer is empty, reset read pointer
        ctrl.read_tail = ctrl.ring_buf_tail;
        nvm_write((u16*)&ctrl, HISTORY_CONTROL_LENGTH, HISTORY_CONTROL_OFFSET);
        if(panic_check(0xFA) != 0) return 1;
        MemSet(buffer, 0, sizeof(his_data_t));
        return 1;
    }

    if(index == READ_HISDATA_LAST) {
        nvm_read((u16*)buffer, CONST_DATA_ONEDAY_LENGTH, HISTORY_DATA_OFFSET+ctrl.write_head*CONST_DATA_ONEDAY_LENGTH);
        if(panic_check(0xF9) != 0) return 1;
    } else {
        nvm_read((u16*)buffer, CONST_DATA_ONEDAY_LENGTH, HISTORY_DATA_OFFSET+ctrl.read_tail*CONST_DATA_ONEDAY_LENGTH);
        ctrl.read_tail = (ctrl.read_tail+1)%CONST_RING_BUFFER_LENGTH;
        nvm_write((u16*)&ctrl, HISTORY_CONTROL_LENGTH, HISTORY_CONTROL_OFFSET);
        if(panic_check(0xF8) != 0) return 1;
    }

    return ready;
}
s16 nvm_write_history_data(u16 *buffer, u8 index)
{
    his_ctrl_t ctrl;
    his_data_t *data = (his_data_t*)buffer;

    nvm_check_storage_init();
    MemSet(&ctrl, 0, sizeof(his_ctrl_t));
    nvm_read((u16*)&ctrl, HISTORY_CONTROL_LENGTH, HISTORY_CONTROL_OFFSET);
    if(panic_check(0xF7) != 0) return 1;
    
    /** user storage fulled, discard the oldest one */
    if(ctrl.ring_buf_tail == (ctrl.ring_buf_head+1)%CONST_RING_BUFFER_LENGTH)
    {
        ctrl.ring_buf_tail = (ctrl.ring_buf_tail+1)%CONST_RING_BUFFER_LENGTH;
        ctrl.read_tail = ctrl.ring_buf_tail;
    }
    ctrl.write_head = ctrl.ring_buf_head;
    ctrl.ring_buf_head = (ctrl.ring_buf_head+1)%CONST_RING_BUFFER_LENGTH;
       
//    ctrl.length = (ctrl.ring_buf_head>ctrl.ring_buf_tail)?
//                            (ctrl.ring_buf_head-ctrl.ring_buf_tail):
//                            (CONST_RING_BUFFER_LENGTH-ctrl.ring_buf_tail+ctrl.ring_buf_head);
    nvm_write((u16*)&ctrl, HISTORY_CONTROL_LENGTH, HISTORY_CONTROL_OFFSET);
    if(panic_check(0xF6) != 0) return 1;
    nvm_write((u16*)data, CONST_DATA_ONEDAY_LENGTH, (HISTORY_DATA_OFFSET+ctrl.write_head*CONST_DATA_ONEDAY_LENGTH));
    if(panic_check(0xF5) != 0) return 1;

    return 0;
}
s16 nvm_write_sport_data(u16 *buffer, u8 index)
{
    his_data_t data;
    MemSet(&data, 0, sizeof(his_data_t));

    data.step_counts = (s16)buffer[0];
    nvm_write_history_data((u16*)&data, 0);

    return 0;
}
s16 nvm_erase_history_data(void)
{
    his_ctrl_t ctrl;
    MemSet(&ctrl, 0, sizeof(his_ctrl_t));
    
    nvm_write((u16*)&ctrl, HISTORY_CONTROL_LENGTH, HISTORY_CONTROL_OFFSET);

    return 0;
}
#if USE_NVM_TEST
s16 nvm_read_test(void)
{
    his_data_t data;
    u16 buf[64] = {0};
    volatile u16 ready = 0;
    u16* ptr = NULL;
    volatile u8 len = 0;

    while(1)
    {
        ptr = buf;
        ready = nvm_read_history_data((u16*)&data, 0);
        if(ready != 0) break;
        BufWriteUint16((uint8 **)&ptr, data.year);
        BufWriteUint8((uint8 **)&ptr, data.month);
        BufWriteUint8((uint8 **)&ptr, data.day);
        BufWriteUint32((uint8 **)&ptr, &data.step_counts);
        len = ptr-buf;
        get_driver()->uart->uart_write((u8*)buf, len);
    }
    

    return 0;
}
s16 nvm_write_test(void)
{
    static u8 flag = 0;
    his_data_t data;
    his_ctrl_t ctrl = {0,0,0,0};
    #if 0
    volatile u16 use_size = USER_STORAGE_TOTAL_LENGTH;
    volatile u16 use_start = USER_STORAGE_START_OFFSET;
    volatile u16 use_end = HISTORY_DATA_OFFSET;
    volatile u16 use_last_len = HISTORY_DATA_LENGTH;
    volatile u16 his_ctrl = HISTORY_CONTROL_LENGTH;
    volatile u16 his_data = CONST_DATA_ONEDAY_LENGTH;
    volatile u16 buf_len = CONST_RING_BUFFER_LENGTH;
    volatile u16 size_u8 = sizeof(u8);
    volatile u16 size_u16 = sizeof(u16);
    volatile u16 size_u32 = sizeof(u32);

    if((use_size == 0) && (use_start == 0) && (use_end == 0) && (use_last_len == 0) &&
       (his_ctrl == 0) && (his_data == 0) && (buf_len == 0) &&
       (size_u8 == 0) && (size_u16 == 0) && (size_u32 == 0)) {
        flag = 1;
    }
    #endif
    if(flag == 0) {
        flag = 1;
        data.year = 2018;
        data.month = 6;
        data.day = 1;
        data.step_counts = 0;
        nvm_check_storage_init();
    } else {
        nvm_read((u16*)&ctrl, HISTORY_CONTROL_LENGTH, HISTORY_CONTROL_OFFSET);
        nvm_read((u16*)&data, CONST_DATA_ONEDAY_LENGTH, (HISTORY_DATA_OFFSET+ctrl.write_head*CONST_DATA_ONEDAY_LENGTH));
    }
    data.day++;
    data.step_counts++;
    nvm_write_history_data((u16*)&data, 0);
    
    return 0;
}
#endif


