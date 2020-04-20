#include <debug.h>          /* Simple host interface to the UART driver */
#include "../adapter.h"
  
typedef struct {
    union {
        u32 ctrl3;
        struct {
            u8 ring_buf_head; /* sport data ring buffer head */
            u8 ring_buf_tail; /* sport data ring buffer tail */
            u8 read_head;
            u8 write_tail;
        }ctrl2;
    }ctrl1;
    union {
        u16 index3;
        struct {
            u8 data_index; /* fifteen-minute data index */
            u8 resv;
        }index2;
    }index1;
}ctrl_t; /* for nvm to store */

typedef struct {
    union {
        u32 date3;
        struct {
            u8 days; /* how many days with history data  */
            u8 year;
            u8 month;
            u8 day;
        }date2;
    }date1;
    union {
        u8 sport3[4];
        struct {
            u16 step;
            u8 sleep;
            u8 count; /* how many times stored history data this day(96 in total)  */
        }sport2;
    }sport1;
}data_t; /* for nvm to store */

#define USER_STORAGE_START_OFFSET               (0x3F)

#define USER_STORATE_INIT_FLAG_OFFSET           (USER_STORAGE_START_OFFSET) /* flag to indicate to initialize the bellow nvm storage */
#define USER_STORATE_INIT_FLAG_LENGTH           (0x01)

#define MOTORS_CURRENT_POSITION_OFFSET          (USER_STORATE_INIT_FLAG_OFFSET+USER_STORATE_INIT_FLAG_LENGTH)
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

#define HISTORY_CONTROL_OFFSET                  (PERSONAL_INFO_OFFSET+PERSONAL_INFO_LENGTH)
#define HISTORY_CONTROL_LENGTH                  (sizeof(ctrl_t))/* word width */

#define CONST_TIME_GRANULARITY                  (96)
#define CONST_RING_BUFFER_LENGTH                (31+1)/* unit: day(1 byte more than acturlly need) */
#define CONST_DATA_ONEDAY_LENGTH                (sizeof(data_t))
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
 * |------------------------| Slot 1 Address
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

s16 nvm_check_storage_init(void)
{
    u16 erase_offset = 0;
    u8 erase_value = 0;
    u16 init_flag = 0;

    nvm_read((u16*)&init_flag, USER_STORATE_INIT_FLAG_LENGTH, USER_STORATE_INIT_FLAG_OFFSET);

    if(init_flag == 0xA55A)
    {
        return 1; 
    }

    for(erase_offset = 0; erase_offset < USER_STORAGE_TOTAL_LENGTH; erase_offset++)
    {
        nvm_write((u16*)&erase_value, 1, USER_STORAGE_START_OFFSET+erase_offset);
    }

    //for(erase_offset = 0; erase_offset < USER_STORAGE_TOTAL_LENGTH; erase_offset++)
    //{
    //    nvm_read((u16*)&erase_value, 1, USER_STORAGE_START_OFFSET+erase_offset);
    //    get_driver()->uart->uart_write((u8*)&erase_value, 1);
    //}

    init_flag = 0xA55A;
    nvm_write((u16*)&init_flag, USER_STORATE_INIT_FLAG_LENGTH, USER_STORATE_INIT_FLAG_OFFSET);/* write user storage init flag */

    return 0;
}
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
s16 nvm_read_history_data(u16 *buffer, u8 index)
{
    ctrl_t ctrl;
    u16 read_offset = 0;
    data_t *data = (data_t*)buffer;
    u16 ready = 0;

    nvm_check_storage_init();
    nvm_read((u16*)&ctrl, HISTORY_CONTROL_LENGTH, HISTORY_CONTROL_OFFSET);

    if(ctrl.ctrl1.ctrl2.read_head != ctrl.ctrl1.ctrl2.ring_buf_tail) {
        read_offset = ctrl.ctrl1.ctrl2.read_head*CONST_DATA_ONEDAY_LENGTH;
        nvm_read((u16*)data, CONST_DATA_ONEDAY_LENGTH, HISTORY_DATA_OFFSET+read_offset);
        buffer[7] = data->sport1.sport2.count;
        buffer[6] = data->sport1.sport2.sleep;
        buffer[5] = data->sport1.sport2.step&0x00FF;
        buffer[4] = (data->sport1.sport2.step>>8)&0x00FF;
        ctrl.ctrl1.ctrl2.read_head = (ctrl.ctrl1.ctrl2.read_head+1)%CONST_RING_BUFFER_LENGTH;
        ready = 1;
    }
    else /* buffer empty, reset read pointer */
    {
        ctrl.ctrl1.ctrl2.read_head = ctrl.ctrl1.ctrl2.ring_buf_head;
    }
    if(ctrl.ctrl1.ctrl2.ring_buf_head != ctrl.ctrl1.ctrl2.ring_buf_tail)
    {
        nvm_write((u16*)&ctrl, HISTORY_CONTROL_LENGTH, HISTORY_CONTROL_OFFSET);
    }

    return ready;
}
s16 nvm_write_history_data(u16 *buffer, u8 index)
{
    ctrl_t ctrl = {.ctrl1.ctrl3=0, .index1.index3=0};
    data_t data = {.date1.date3=0, .sport1.sport3={0,0,0,0}};
    static u8 test_date[3] = {0x20, 0x04, 0x17};
    u16 test_init = 0;
    static u16 test_step = 100;
    static u8 test_sleep = 4;

    test_init = nvm_check_storage_init();
    nvm_read((u16*)&ctrl, HISTORY_CONTROL_LENGTH, HISTORY_CONTROL_OFFSET);
    nvm_read((u16*)&data, CONST_DATA_ONEDAY_LENGTH, (HISTORY_DATA_OFFSET+ctrl.ctrl1.ctrl2.write_tail*CONST_DATA_ONEDAY_LENGTH));

    if(ctrl.index1.index2.data_index%CONST_TIME_GRANULARITY == 0)  /* new day */
    {
        /** user storage fulled, discard the oldest one */
        if(ctrl.ctrl1.ctrl2.ring_buf_head == (ctrl.ctrl1.ctrl2.ring_buf_tail+1)%CONST_RING_BUFFER_LENGTH)
        {
            ctrl.ctrl1.ctrl2.ring_buf_head = (ctrl.ctrl1.ctrl2.ring_buf_head+1)%CONST_RING_BUFFER_LENGTH;
            ctrl.ctrl1.ctrl2.read_head = ctrl.ctrl1.ctrl2.ring_buf_head;
        }
        
        data.date1.date2.year = test_date[0];
        data.date1.date2.month = test_date[1];
        data.date1.date2.day = test_date[2]++;
        
        ctrl.index1.index2.data_index = 0;
        data.sport1.sport2.count = 0;
        data.sport1.sport2.sleep = 0;
        data.sport1.sport2.step = 0;
        ctrl.ctrl1.ctrl2.write_tail = ctrl.ctrl1.ctrl2.ring_buf_tail;
        ctrl.ctrl1.ctrl2.ring_buf_tail = (ctrl.ctrl1.ctrl2.ring_buf_tail+1)%CONST_RING_BUFFER_LENGTH;
    }

    data.sport1.sport2.step += test_step;//((data_t*)buffer)->sport1.sport2.step;
    data.sport1.sport2.sleep += test_sleep;//((data_t*)buffer)->sport1.sport2.sleep;
    ctrl.index1.index2.data_index++;
    data.sport1.sport2.count++;
    data.date1.date2.days = (ctrl.ctrl1.ctrl2.ring_buf_tail>ctrl.ctrl1.ctrl2.ring_buf_head)?
                            (ctrl.ctrl1.ctrl2.ring_buf_tail-ctrl.ctrl1.ctrl2.ring_buf_head):
                            (CONST_RING_BUFFER_LENGTH-ctrl.ctrl1.ctrl2.ring_buf_head+ctrl.ctrl1.ctrl2.ring_buf_tail);
    nvm_write((u16*)&ctrl, HISTORY_CONTROL_LENGTH, HISTORY_CONTROL_OFFSET);
    nvm_write((u16*)&data, CONST_DATA_ONEDAY_LENGTH, (HISTORY_DATA_OFFSET+ctrl.ctrl1.ctrl2.write_tail*CONST_DATA_ONEDAY_LENGTH));

    return 0;
}
s16 nvm_write_step_data(u16 *buffer, u8 index)
{
    data_t data = {.date1.date3=0, .sport1.sport3={0,0,0,0}};

    data.sport1.sport2.step = (s16)buffer[0];
    nvm_write_history_data((u16*)&data, 0);

    return 0;
}
s16 nvm_write_sleep_data(u16 *buffer, u8 index)
{
    data_t data = {.date1.date3=0, .sport1.sport3={0,0,0,0}};

    data.sport1.sport2.sleep = (u8)buffer[0];
    nvm_write_history_data((u16*)&data, 0);

    return 0;
}
s16 nvm_erase_history_data(void)
{
    ctrl_t ctrl = {.ctrl1.ctrl3=0, .index1.index3=0};
    
    nvm_write((u16*)&ctrl, HISTORY_CONTROL_LENGTH, HISTORY_CONTROL_OFFSET);

    return 0;
}
