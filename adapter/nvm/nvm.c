#include <debug.h>          /* Simple host interface to the UART driver */
#include "../adapter.h"

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

#define SPORT_DATA_CONTROL_OFFSET               (PERSONAL_INFO_OFFSET+PERSONAL_INFO_LENGTH)
#define SPORT_DATA_CONTROL_LENGTH               (4)/* word width */

#define CONST_RING_BUFFER_LENGTH                (20)/* unit: day */
#define CONST_DATE_DATA_LENGTH                  (2)/* yyyymmdd */
#define CONST_SPORT_DATA_LENGTH                 (1)/* word width, max 65535 */
#define CONST_SLEEP_DATA_LENGTH                 (1)/* word width, max 65535 */
#define CONST_DATA_ONEDAY_LENGTH                (CONST_DATE_DATA_LENGTH+CONST_SPORT_DATA_LENGTH+CONST_SLEEP_DATA_LENGTH)/* 96 pieces every day */
#define SPORT_DATA_OFFSET                       (SPORT_DATA_CONTROL_OFFSET+SPORT_DATA_CONTROL_LENGTH)
#define SPORT_DATA_LENGTH                       (CONST_DATA_ONEDAY_LENGTH*CONST_RING_BUFFER_LENGTH)/* store 20 days's history */

#define USER_STORAGE_TOTAL_LENGTH               (SPORT_DATA_LENGTH+(SPORT_DATA_OFFSET-USER_STORAGE_START_OFFSET))

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
*/

typedef struct {
    union {
        u16 ring_buf_ptr;
        struct {
            u8 ring_buf_head; /* sport data ring buffer head */
            u8 ring_buf_tail; /* sport data ring buffer tail */
        }ctrl;
    }ptr;
    union {
        u16 store_index;
        struct {
            u8 cell_data_index; /* fifteen-minute data index */
            u8 new_day_index; /* position to store date every day */
        }ctrl;
    }index;
    u16 sport_acc;
    u16 sleep_acc;
}storage_ctrl_t; /* for nvm to store */

void nvm_read(u16 *buffer, u16 length, u16 offset);
void nvm_write(u16 *buffer, u16 length, u16 offset);
s16 nvm_read_control_data(u16 *buffer, u8 index);
s16 nvm_write_control_data(u16 *buffer, u8 index);

void nvm_read(u16 *buffer, u16 length, u16 offset)
{
    get_driver()->flash->flash_read(buffer, length, offset);
}
void nvm_write(u16 *buffer, u16 length, u16 offset)
{
    get_driver()->flash->flash_write(buffer, length, offset);
}

void nvm_check_storage_init(void)
{
    u16 erase_offset = 0;
    u8 erase_value = 0;
    u16 init_flag = 0;

    nvm_read((u16*)&init_flag, USER_STORATE_INIT_FLAG_LENGTH, USER_STORATE_INIT_FLAG_OFFSET);

    if(init_flag == 0xA55A)
    {
        return; 
    }

    for(erase_offset = 0; erase_offset < USER_STORAGE_TOTAL_LENGTH; erase_offset++)
    {
        nvm_write((u16*)&erase_value, 1, USER_STORAGE_START_OFFSET+erase_offset);
    }

    init_flag = 0xA55A;
    nvm_write((u16*)&init_flag, USER_STORATE_INIT_FLAG_LENGTH, USER_STORATE_INIT_FLAG_OFFSET);/* write user storage init flag */
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
s16 nvm_read_sport_setting(u16 *buffer, u8 index)
{
    nvm_read(buffer, SPORT_SETTING_LENGTH, SPORT_SETTING_OFFSET);

    return 0;
}
s16 nvm_write_sport_setting(u16 *buffer, u8 index)
{
    nvm_write(buffer, SPORT_SETTING_LENGTH, SPORT_SETTING_OFFSET);

    return 0;
}
s16 nvm_read_control_data(u16 *buffer, u8 index)
{
    nvm_read(buffer, SPORT_DATA_CONTROL_LENGTH, SPORT_DATA_CONTROL_OFFSET);

    return 0;
}
s16 nvm_write_control_data(u16 *buffer, u8 index)
{
    nvm_write(buffer, SPORT_DATA_CONTROL_LENGTH, SPORT_DATA_CONTROL_OFFSET);

    return 0;
}
s16 nvm_read_sport_data(u16 *buffer, u8 index)
{
    storage_ctrl_t store;
    u8 store_head = 0;
    u16 read_offset = 0;
    u8 read_buffer[CONST_DATA_ONEDAY_LENGTH] = {0};


    nvm_read_control_data((u16*)&store, 0); /* read control data */

    store_head = store.ptr.ctrl.ring_buf_head;
    while(store_head != store.ptr.ctrl.ring_buf_tail)
    {
        read_offset = store_head*CONST_DATA_ONEDAY_LENGTH;
        nvm_read((u16*)read_buffer, CONST_DATA_ONEDAY_LENGTH, SPORT_DATA_OFFSET+read_offset);
        store_head = (store_head+1)%CONST_RING_BUFFER_LENGTH;
    }

    return 0;
}
u16 max_len = 0;
s16 nvm_write_sport_data(u16 *buffer, u8 index)
{
    storage_ctrl_t store;
    u8 new_day_test[4] = {0x00, 0x20, 0x04, 0x16};
    u16 date_data_offset = 0;
    u16 sport_data_offset = 0;
    u16 sport_data_length = 0;
    typedef struct { u16 sport; u16 sleep; }sport_data_t;
    sport_data_t sport_data = {.sport = 0, .sleep = 0};

    max_len = USER_STORAGE_TOTAL_LENGTH;
    if(max_len == 0)
    {
        store.index.ctrl.cell_data_index = 0;
    }

    nvm_check_storage_init();
    nvm_read_control_data((u16*)&store, 0); /* read control data */
    
    date_data_offset = (SPORT_DATA_OFFSET+store.ptr.ctrl.ring_buf_tail*CONST_DATA_ONEDAY_LENGTH);
    sport_data_offset = (date_data_offset+CONST_DATE_DATA_LENGTH);
    sport_data_length = (CONST_SPORT_DATA_LENGTH+CONST_SLEEP_DATA_LENGTH);
    nvm_read((u16*)&sport_data, sport_data_length, sport_data_offset);

    /** user storage fulled, discard the oldest one */
    if(store.ptr.ctrl.ring_buf_head == (store.ptr.ctrl.ring_buf_tail+1)%CONST_RING_BUFFER_LENGTH)
    {
        store.ptr.ctrl.ring_buf_head = (store.ptr.ctrl.ring_buf_head+1)%CONST_RING_BUFFER_LENGTH;
    }
    
    if((store.index.ctrl.cell_data_index%96) == 0) /* new day */
    {
        nvm_write((u16*)new_day_test, CONST_DATE_DATA_LENGTH, date_data_offset);
        
        store.ptr.ctrl.ring_buf_tail = (store.ptr.ctrl.ring_buf_tail+1)%CONST_RING_BUFFER_LENGTH;
        store.index.ctrl.cell_data_index = 0;
        sport_data.sport = 0;
    }

    sport_data.sport += *buffer;
    nvm_write((u16*)&sport_data, sport_data_length, sport_data_offset);
    
    store.index.ctrl.cell_data_index++;
    nvm_write_control_data((u16*)&store, 0);

    return 0;
}


