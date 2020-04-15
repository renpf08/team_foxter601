#include <debug.h>          /* Simple host interface to the UART driver */
#include "../adapter.h"

#define POWER_ON_COUNTER_OFFSET                   (0x3F)

#define MOTORS_CURRENT_POSITION_OFFSET          (0x40)
#define MOTORS_CURRENT_POSITION_LENGTH          (0x06)

#define MOTORS_ZERO_POSITION_POLARITY_OFFSET    (MOTORS_CURRENT_POSITION_OFFSET+MOTORS_CURRENT_POSITION_LENGTH)
#define MOTORS_ZERO_POSITION_POLARITY_LENGTH    (0x06)

#define PAIRING_CODE_OFFSET                     (MOTORS_ZERO_POSITION_POLARITY_OFFSET+MOTORS_ZERO_POSITION_POLARITY_LENGTH)
#define PAIRING_CODE_LENGTH                     (0x01)

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

#define SPORT_DATA_CONTROL_OFFSET               (PERSONAL_INFO_OFFSET+PERSONAL_INFO_LENGTH)
#define SPORT_DATA_CONTROL_LENGTH               (2)/* word width */

#define SPORT_DATA_OFFSET                       (SPORT_DATA_CONTROL_LENGTH+SPORT_DATA_CONTROL_LENGTH)
#define SPORT_DATA_LENGTH                       (528)/* word width */
#define SPORT_DATE_UNIT_LENGTH                  (2)/* word width */

#define SPORT_DATA_ONEDAY_SIZE                  (SPORT_DATA_CONTROL_LENGTH + SPORT_DATE_UNIT_LENGTH*15)

#if 0
#define SP_CURRENT_BLOCK_INDEX_OFFSET           (SPORT_SETTING_OFFSET+SPORT_SETTING_LENGTH)
#define SP_CURRENT_BLOCK_INDEX_LENGTH           (0x01)

#define SP_TOTAL_BLOCK_INDEX_OFFSET             (SP_CURRENT_BLOCK_INDEX_OFFSET+SP_CURRENT_BLOCK_INDEX_LENGTH)/*保存当前存储空间的块 总数量*/  
#define SP_TOTAL_BLOCK_INDEX_LENGTH             (0x01)

#define SPORT_DATE_OFFSET                       (SP_TOTAL_BLOCK_INDEX_OFFSET+SP_TOTAL_BLOCK_INDEX_LENGTH)
#define SPORT_DATE_LENGTH                       (0x03)  /*日期 3个words*/
 
#define SP_ONE_DAY_TOTAL_DATA_ADDRESS_OFFSET_0  (SPORT_DATE_OFFSET+SPORT_DATE_LENGTH)
#define SP_DATA_LENGTH                          (0x08)  /*一组数据 8个words*/

/*新修改的数据存储格式，只有总量和睡眠信息*/
#define SP_ASLEEP_INFO_ONEDAY_ADDRESS_OFFSET_0  (SP_ONE_DAY_TOTAL_DATA_ADDRESS_OFFSET_0+SP_DATA_LENGTH)
#define SP_ASLEEP_INFO_ONEDAY_LENGTH            (12)    /*一组数据 12个words*/

/*新修改的数据存储格式结束*/
#define SP_BLOCK_LENGTH                         (23) /*一个BLOCK的数据长度23个words*/
/*780个words,1560 Bytes, EEPROM空间32KB 用来存数据 ，最大可以存20天的数据量*/
#define SP_MAX_BLOCKS                           (30) /*(10) (16)*/
/*运动数据存储区结束*/

#define SP_BLOCK_HEADER_INDEX                    (0x80)
#define SP_BLOCK_DATE                            (0x81)
#define SP_BLOCK_TOTAL_DATA                      (0x00)
#define SP_BLOCK_ASLEEP_INFO_NUMBER              (97)
#define SP_BLOCK_ASLEEP_INFO_ONEDAYS			 (101)	
#endif

/* sleep/sport history data storage model
 * |----------------------------------------|
 * |           ~~~~control block~~~~        |
 * |----------------------------------------|
 * | history data store head pointer        | 1 byte
 * |----------------------------------------|
 * | history data store tail pointer        | 1 byte
 * |----------------------------------------|
 * | current day store index(15 per day)    | 1 byte
 * |----------------------------------------|
 * | position to store date every day       | 1 byte
 * |----------------------------------------|
 * |            ~~~~data block~~~~          |
 * |--------|----------------------|--------|
 * |        |  date & time 00      |4 bytes|
 * |        |----------------------|    +   |
 * |        | index:00 sport/sleep |(2 byte |
 * |        |----------------------|        |
 * |        | index:01 sport/sleep |   *15s)| 34 byte
 * |        |----------------------|        |
 * |        |   ...     ...        |=34bytes|   *20
 * |        |----------------------|        |   *31
 * |   20   | index:14 sport/sleep |        |
 * |  days  |----------------------|--------|=680 bytes
 * |        |       ...            |        |=1054 bytes
 * |        |----------------------|        |
 * |        |  date & time 19      |        |
 * |        |----------------------|        |
 * |        | index:00 sport/sleep |        |
 * |        |----------------------|        |
 * |        | index:01 sport/sleep |        |
 * |        |----------------------|        |
 * |        |   ...     ...        |        |
 * |        |----------------------|        |
 * |        | index:14 sport/sleep |        |
 * |--------|----------------------|-----   |
 * | total=680+4 bytes  = 342 words(20 days)|
 * |      =1054+4 bytes = 529 words(20 days)|
 * |----------------------------------------|
*/

typedef struct {
    union {
        u16 store_buf;
        struct {
            u8 ring_buf_head; /* sport data ring buffer head */
            u8 ring_buf_tail; /* sport data ring buffer tail */
        }msg;
    }ptr;
    union {
        u16 buf_index;
        struct {
            u8 cell_data_index; /* fifteen-minute data index */
            u8 new_day_offset; /* position to store date every day */
        }msg;
    }index;
}store_ctrl_params_t; /* for nvm to store */

typedef struct {
    u8 head; /* sport data ring buffer head */
    u8 tail; /* sport data ring buffer tail */
    u8 index; /* fifteen-minute data index */
    u8 start; /* position to store date every day */
}ctrl_params_t; /* get value form store_ctrl_params_t */

void _read(u16 *buffer, u16 length, u16 offset);
void _write(u16 *buffer, u16 length, u16 offset);
s16 nvm_read_control_data(u16 *buffer, u8 index);
s16 nvm_write_control_data(u16 *buffer, u8 index);

void _read(u16 *buffer, u16 length, u16 offset)
{
    get_driver()->flash->flash_read(buffer, length, offset);
}
void _write(u16 *buffer, u16 length, u16 offset)
{
    get_driver()->flash->flash_write(buffer, length, offset);
}

s16 nvm_read_motor_current_position(u16 *buffer, u8 index)
{
    _read(buffer, MOTORS_CURRENT_POSITION_LENGTH, MOTORS_CURRENT_POSITION_OFFSET);

    return 0;
}
s16 nvm_write_motor_current_position(u16 *buffer, u8 index)
{ 
    _write(buffer, MOTORS_CURRENT_POSITION_LENGTH, MOTORS_CURRENT_POSITION_OFFSET);   

    return 0;
}
s16 nvm_read_zero_position_polarity(u16 *buffer, u8 index)
{
    _read(buffer, MOTORS_ZERO_POSITION_POLARITY_LENGTH, MOTORS_ZERO_POSITION_POLARITY_OFFSET); 

    return 0;
}
s16 nvm_write_zero_position_polarity(u16 *buffer, u8 index)
{
    _write(buffer, MOTORS_ZERO_POSITION_POLARITY_LENGTH, MOTORS_ZERO_POSITION_POLARITY_OFFSET); 

    return 0; 
}
s16 nvm_read_pairing_code(u16 *buffer, u8 index)
{
    _read(buffer, PAIRING_CODE_LENGTH, PAIRING_CODE_OFFSET);

    return 0;
}
s16 nvm_write_pairing_code(u16 *buffer, u8 index)
{
    _write(buffer, PAIRING_CODE_LENGTH, PAIRING_CODE_OFFSET);

    return 0;
}

s16 nvm_read_date_time(u16 *buffer, u8 index)
{
    _read(buffer, SYSTEM_DATE_TIME_LENGTH, SYSTEM_DATE_TIME_OFFSET);

    return 0;
}
s16 nvm_write_date_time(u16 *buffer, u8 index)
{
    _write(buffer, SYSTEM_DATE_TIME_LENGTH, SYSTEM_DATE_TIME_OFFSET);

    return 0;
}
s16 nvm_read_alarm_clock_single(u16 *buffer, u8 index)
{
    _read(buffer, ALARM_CLOCK_SINGLE_LENGTH, ALARM_CLOCK_OFFSET+index*ALARM_CLOCK_SINGLE_LENGTH);

    return 0; 
}
s16 nvm_write_alarm_clock_single(u16 *buffer, u8 index)
{
    _write(buffer, ALARM_CLOCK_SINGLE_LENGTH, ALARM_CLOCK_OFFSET+index*ALARM_CLOCK_SINGLE_LENGTH); 

    return 0;
}
s16 nvm_read_alarm_clock_total(u16 *buffer, u8 index)
{
    _read(buffer, ALARM_CLOCK_LENGTH, ALARM_CLOCK_OFFSET);

    return 0;
}
s16 nvm_write_alarm_clock_total(u16 *buffer, u8 index)
{
    _write(buffer, ALARM_CLOCK_LENGTH, ALARM_CLOCK_OFFSET); 

    return 0;
}
s16 nvm_read_display_setting(u16 *buffer, u8 index)
{
    _read(buffer, DISPLAY_SETTING_LENGTH, DISPLAY_SETTING_OFFSET);

    return 0;
}
s16 nvm_write_display_setting(u16 *buffer, u8 index)
{
    _write(buffer, DISPLAY_SETTING_LENGTH, DISPLAY_SETTING_OFFSET);  

    return 0;
}
s16 nvm_read_personal_info(u16 *buffer, u8 index)
{
    _read(buffer, PERSONAL_INFO_LENGTH, PERSONAL_INFO_OFFSET);

    return 0;
}
s16 nvm_write_personal_info(u16 *buffer, u8 index)
{
    _write(buffer, PERSONAL_INFO_LENGTH, PERSONAL_INFO_OFFSET);

    return 0;
}
s16 nvm_read_sport_setting(u16 *buffer, u8 index)
{
    _read(buffer, SPORT_SETTING_LENGTH, SPORT_SETTING_OFFSET);

    return 0;
}
s16 nvm_write_sport_setting(u16 *buffer, u8 index)
{
    _write(buffer, SPORT_SETTING_LENGTH, SPORT_SETTING_OFFSET);

    return 0;
}
s16 nvm_read_control_data(u16 *buffer, u8 index)
{
    _read(buffer, SPORT_DATA_CONTROL_LENGTH, SPORT_DATA_CONTROL_OFFSET+index*0);

    return 0;
}
s16 nvm_write_control_data(u16 *buffer, u8 index)
{
    _write(buffer, SPORT_DATA_CONTROL_LENGTH, SPORT_DATA_CONTROL_OFFSET+index*0);

    return 0;
}
s16 nvm_read_sport_data(u16 *buffer, u8 index)
{
    _read(buffer, SPORT_DATA_LENGTH, SPORT_DATA_OFFSET+index*0);

    return 0;
}
s16 nvm_write_sport_data(u16 *buffer, u8 index)
{
    store_ctrl_params_t store_ctrl_param;
    u8 new_day_pos = 0;

    nvm_read_control_data((u16*)&store_ctrl_param, 0);
    index = store_ctrl_param.index.msg.cell_data_index;

    /* write date to nvm in the begin of a day */
    if(index == 0)
    {
        new_day_pos = SPORT_DATA_ONEDAY_SIZE*store_ctrl_param.index.msg.new_day_offset;
        //_write(buffer, SPORT_DATA_LENGTH, SPORT_DATA_OFFSET+new_day_pos);
    }
    
    //_write(buffer, SPORT_DATA_LENGTH, SPORT_DATA_OFFSET+index*0);

    return 0;
}


