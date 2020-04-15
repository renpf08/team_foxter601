#include <debug.h>          /* Simple host interface to the UART driver */
#include "../common/common.h"
#include "../driver/driver.h"

#define POWER_ON_COUNTER_OFFSET                   (0x3F)

#define MOTORS_CURRENT_POSITION_OFFSET          (0x40)
#define MOTORS_CURRENT_POSITION_LENGTH          (0x06)

#define MOTORS_ZERO_POSITION_POLARITY_OFFSET    (MOTORS_CURRENT_POSITION_OFFSET+MOTORS_CURRENT_POSITION_LENGTH)
#define MOTORS_ZERO_POSITION_POLARITY_LENGTH    (0x06)

#define SB100_BT_ADDRESS_OFFSET                 (MOTORS_ZERO_POSITION_POLARITY_OFFSET+MOTORS_ZERO_POSITION_POLARITY_LENGTH)
#define SB100_BT_ADDRESS_LENGTH                 (0x05)

#define BONDED_BT_ADDRESS_OFFSET                (SB100_BT_ADDRESS_OFFSET+SB100_BT_ADDRESS_LENGTH)
#define BONDED_BT_ADDRESS_LENGTH                (0x05)

#define BONDED_STORE_BLOCK_OFFSET               (BONDED_BT_ADDRESS_OFFSET+BONDED_BT_ADDRESS_LENGTH)
#define BONDED_STORE_BLOCK_LENGTH               (0x01)/*是否连接标志*/

#define DATE_TIME_STORE_BLOCK_OFFSET            (BONDED_STORE_BLOCK_OFFSET+BONDED_STORE_BLOCK_LENGTH)
#define DATE_TIME_STORE_BLOCK_LENGTH            (0x07)/*15分钟存储一次时间走动数据*/

#define ALARM_SETTING_0_BLOCK_OFFSET            (DATE_TIME_STORE_BLOCK_OFFSET+DATE_TIME_STORE_BLOCK_LENGTH)
#define ALARM_SETTING_LENGTH                    (0x04) /*单组值长度*/
#define ALARM_SETTING_BLOCK_LENGTH              (20)/*总共有5组闹钟设定值*/

#define DISPLAY_SETTING_BLOCK_OFFSET            (ALARM_SETTING_0_BLOCK_OFFSET+ALARM_SETTING_BLOCK_LENGTH)
#define DISPLAY_SETTING_BLOCK_LENGTH            (0x4)

#define BODY_INFO_BLOCK_OFFSET                  (DISPLAY_SETTING_BLOCK_OFFSET+DISPLAY_SETTING_BLOCK_LENGTH)
#define BODY_INFO_BLOCK_LENGTH                  (12)

#define SP_GOAL_DATA_OFFSET                     (BODY_INFO_BLOCK_OFFSET+BODY_INFO_BLOCK_LENGTH)
#define SP_GOAL_DATA_LENGTH                     (0x08)  /*一组数据 8个words*/

/****运动数据存储区*****/
#define SP_CURRENT_BLOCK_INDEX_OFFSET           (SP_GOAL_DATA_OFFSET+SP_GOAL_DATA_LENGTH)/*保存当前存储空间的块 索引号*/
#define SP_CURRENT_BLOCK_INDEX_LENGTH           (0x01)

#define SP_TOTAL_BLOCK_INDEX_OFFSET             (SP_CURRENT_BLOCK_INDEX_OFFSET+SP_CURRENT_BLOCK_INDEX_LENGTH)/*保存当前存储空间的块 总数量*/  
#define SP_TOTAL_BLOCK_INDEX_LENGTH             (0x01)
                                                    /*区块重复开始*/
#define SP_START_ADDRESS_OFFSET_0               (SP_TOTAL_BLOCK_INDEX_OFFSET+SP_TOTAL_BLOCK_INDEX_LENGTH)  


#define SP_DATE_ADDRESS_OFFSET_0                (SP_START_ADDRESS_OFFSET_0)
#define SP_DATE_LENGTH                          (0x03)  /*日期 3个words*/
 
#define SP_ONE_DAY_TOTAL_DATA_ADDRESS_OFFSET_0  (SP_DATE_ADDRESS_OFFSET_0+SP_DATE_LENGTH)
#define SP_DATA_LENGTH                          (0x08)  /*一组数据 8个words*/

/*新修改的数据存储格式，只有总量和睡眠信息*/
#define SP_ASLEEP_INFO_ONEDAY_ADDRESS_OFFSET_0  (SP_ONE_DAY_TOTAL_DATA_ADDRESS_OFFSET_0+SP_DATA_LENGTH)
#define SP_ASLEEP_INFO_ONEDAY_LENGTH            (12)    /*一组数据 12个words*/

/*新修改的数据存储格式结束*/
#define SP_BLOCK_LENGTH                         (23) /*一个BLOCK的数据长度23个words*/
/*780个words,1560 Bytes, EEPROM空间32KB 用来存数据 ，最大可以存20天的数据量*/
#define SP_MAX_BLOCKS                           (30) /*(10) (16)*/
/*运动数据存储区结束*/

typedef s16 (*nvm_write)(u16 *buffer, u8 index);
typedef s16 (*nvm_read)(u16 *buffer, u8 index);

typedef struct {
	nvm_read 	read_motor_current_position;
	nvm_write 	write_motor_current_position;
	nvm_read 	read_zero_position_polarity;
	nvm_write 	write_zero_position_polarity;
	nvm_read 	read_bonded_bt_address;
	nvm_write 	write_bonded_bt_address;
	nvm_read 	read_date_time;
	nvm_write 	write_date_time;
	nvm_read 	read_single_alarm_clock;
	nvm_write 	write_single_alarm_clock;
	nvm_read 	read_all_alarm_clock;
	nvm_write 	write_all_alarm_clock;
	nvm_read 	read_display_setting;
	nvm_write 	write_display_setting;
	nvm_read 	read_personal_info;
	nvm_write 	write_personal_info;
	nvm_read 	read_personal_sport_goal;
	nvm_write 	write_personal_sport_goal;
	nvm_read 	read_current_sport_data_index;
	nvm_write 	write_current_sport_data_index;
	nvm_read 	read_current_sport_data_total;
	nvm_write 	write_current_sport_data_total;
	nvm_read 	read_total_sport_data;
	nvm_write 	write_total_sport_data;
	nvm_read 	read_sleep_data;
	nvm_write 	write_sleep_data;
	nvm_read 	read_sport_date;
	nvm_write 	write_sport_date;
}nvm_t;

void _read(u16 *buffer, u16 length, u16 offset);
void _write(u16 *buffer, u16 length, u16 offset);
s16 nvm_read_motor_current_position(u16 *buffer, u8 index);
s16 nvm_write_motor_current_position(u16 *buffer, u8 index);
s16 nvm_read_zero_position_polarity(u16 *buffer, u8 index);
s16 nvm_write_zero_position_polarity(u16 *buffer, u8 index);
s16 nvm_read_bonded_bt_address(u16 *buffer, u8 index);
s16 nvm_write_bonded_bt_address(u16 *buffer, u8 index);
s16 nvm_read_date_time(u16 *buffer, u8 index);
s16 nvm_write_date_time(u16 *buffer, u8 index);
s16 nvm_read_single_alarm_clock(u16 *buffer, u8 index);
s16 nvm_write_single_alarm_clock(u16 *buffer, u8 index);
s16 nvm_read_all_alarm_clock(u16 *buffer, u8 index);
s16 nvm_write_all_alarm_clock(u16 *buffer, u8 index);
s16 nvm_read_display_setting(u16 *buffer, u8 index);
s16 nvm_write_display_setting(u16 *buffer, u8 index);
s16 nvm_read_personal_info(u16 *buffer, u8 index);
s16 nvm_write_personal_info(u16 *buffer, u8 index);
s16 nvm_read_personal_sport_goal(u16 *buffer, u8 index);
s16 nvm_write_personal_sport_goal(u16 *buffer, u8 index);
s16 nvm_read_current_sport_data_index(u16 *buffer, u8 index);
s16 nvm_write_current_sport_data_index(u16 *buffer, u8 index);
s16 nvm_read_current_sport_data_total(u16 *buffer, u8 index);
s16 nvm_write_current_sport_data_total(u16 *buffer, u8 index);
s16 nvm_read_total_sport_data(u16 *buffer, u8 index);
s16 nvm_write_total_sport_data(u16 *buffer, u8 index);
s16 nvm_read_sleep_data(u16 *buffer, u8 index);
s16 nvm_write_sleep_data(u16 *buffer, u8 index);
s16 nvm_read_sport_date(u16 *buffer, u8 index);
s16 nvm_write_sport_date(u16 *buffer, u8 index);
s16 nvm_init(void);
nvm_t *nvm_get(void);


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
s16 nvm_read_bonded_bt_address(u16 *buffer, u8 index)
{
    _read(buffer, BONDED_BT_ADDRESS_LENGTH,BONDED_BT_ADDRESS_OFFSET);

    return 0;
}
s16 nvm_write_bonded_bt_address(u16 *buffer, u8 index)
{
    _write(buffer, BONDED_BT_ADDRESS_LENGTH,BONDED_BT_ADDRESS_OFFSET);

    return 0;
}
s16 nvm_read_date_time(u16 *buffer, u8 index)
{
    _read(buffer, DATE_TIME_STORE_BLOCK_LENGTH,DATE_TIME_STORE_BLOCK_OFFSET);

    return 0;
}
s16 nvm_write_date_time(u16 *buffer, u8 index)
{
    _write(buffer, DATE_TIME_STORE_BLOCK_LENGTH,DATE_TIME_STORE_BLOCK_OFFSET);

    return 0;
}
s16 nvm_read_single_alarm_clock(u16 *buffer, u8 index)
{
    _read(buffer, ALARM_SETTING_LENGTH, ALARM_SETTING_0_BLOCK_OFFSET+index*ALARM_SETTING_LENGTH);

    return 0; 
}
s16 nvm_write_single_alarm_clock(u16 *buffer, u8 index)
{
    _write(buffer, ALARM_SETTING_LENGTH, ALARM_SETTING_0_BLOCK_OFFSET+index*ALARM_SETTING_LENGTH); 

    return 0;
}
s16 nvm_read_all_alarm_clock(u16 *buffer, u8 index)
{
    _read(buffer, ALARM_SETTING_BLOCK_LENGTH, ALARM_SETTING_0_BLOCK_OFFSET);

    return 0;
}
s16 nvm_write_all_alarm_clock(u16 *buffer, u8 index)
{
    _write(buffer, ALARM_SETTING_LENGTH, ALARM_SETTING_0_BLOCK_OFFSET+index*ALARM_SETTING_LENGTH); 

    return 0;
}
s16 nvm_read_display_setting(u16 *buffer, u8 index)
{
    _read(buffer, DISPLAY_SETTING_BLOCK_LENGTH, DISPLAY_SETTING_BLOCK_OFFSET);

    return 0;
}
s16 nvm_write_display_setting(u16 *buffer, u8 index)
{
    _write(buffer, DISPLAY_SETTING_BLOCK_LENGTH, DISPLAY_SETTING_BLOCK_OFFSET);  

    return 0;
}
s16 nvm_read_personal_info(u16 *buffer, u8 index)
{
    _read(buffer, BODY_INFO_BLOCK_LENGTH, BODY_INFO_BLOCK_OFFSET);

    return 0;
}
s16 nvm_write_personal_info(u16 *buffer, u8 index)
{
    _write(buffer, BODY_INFO_BLOCK_LENGTH, BODY_INFO_BLOCK_OFFSET);

    return 0;
}
s16 nvm_read_personal_sport_goal(u16 *buffer, u8 index)
{
    _read(buffer, SP_GOAL_DATA_LENGTH, SP_GOAL_DATA_OFFSET);

    return 0;
}
s16 nvm_write_personal_sport_goal(u16 *buffer, u8 index)
{
    _write(buffer, SP_GOAL_DATA_LENGTH, SP_GOAL_DATA_OFFSET);

    return 0;
}
s16 nvm_read_current_sport_data_index(u16 *buffer, u8 index)
{
    _read(buffer, SP_CURRENT_BLOCK_INDEX_LENGTH, SP_CURRENT_BLOCK_INDEX_OFFSET);

    return 0;
}
s16 nvm_write_current_sport_data_index(u16 *buffer, u8 index)
{
    _write(buffer, SP_CURRENT_BLOCK_INDEX_LENGTH, SP_CURRENT_BLOCK_INDEX_OFFSET);

    return 0;
}
s16 nvm_read_current_sport_data_total(u16 *buffer, u8 index)
{
    _read(buffer, SP_TOTAL_BLOCK_INDEX_LENGTH, SP_TOTAL_BLOCK_INDEX_OFFSET);

    return 0;
}
s16 nvm_write_current_sport_data_total(u16 *buffer, u8 index)
{
    _write(buffer, SP_TOTAL_BLOCK_INDEX_LENGTH, SP_TOTAL_BLOCK_INDEX_OFFSET);

    return 0;
}
s16 nvm_read_total_sport_data(u16 *buffer, u8 index)
{
    _read(buffer, SP_DATA_LENGTH, SP_ONE_DAY_TOTAL_DATA_ADDRESS_OFFSET_0+index*SP_BLOCK_LENGTH);

    return 0;
}
s16 nvm_write_total_sport_data(u16 *buffer, u8 index)
{
    _write(buffer, SP_DATA_LENGTH, SP_ONE_DAY_TOTAL_DATA_ADDRESS_OFFSET_0+index*SP_BLOCK_LENGTH);

    return 0;
}
s16 nvm_read_sleep_data(u16 *buffer, u8 index)
{
    _read(buffer, SP_ASLEEP_INFO_ONEDAY_LENGTH, SP_ASLEEP_INFO_ONEDAY_ADDRESS_OFFSET_0+index*SP_BLOCK_LENGTH);

    return 0;
}
s16 nvm_write_sleep_data(u16 *buffer, u8 index)
{
    _write(buffer, SP_ASLEEP_INFO_ONEDAY_LENGTH, SP_ASLEEP_INFO_ONEDAY_ADDRESS_OFFSET_0+index*SP_BLOCK_LENGTH);

    return 0;
}
s16 nvm_read_sport_date(u16 *buffer, u8 index)
{
    _read(buffer, SP_DATE_LENGTH, SP_DATE_ADDRESS_OFFSET_0+index*SP_BLOCK_LENGTH);

    return 0;
}
s16 nvm_write_sport_date(u16 *buffer, u8 index)
{
    _write(buffer, SP_DATE_LENGTH, SP_DATE_ADDRESS_OFFSET_0+index*SP_BLOCK_LENGTH);

    return 0;
}

nvm_t nvm_usr = {
	.read_motor_current_position = nvm_read_motor_current_position,
	.write_motor_current_position = nvm_write_motor_current_position,
	.read_zero_position_polarity = nvm_read_zero_position_polarity,
	.write_zero_position_polarity = nvm_write_zero_position_polarity,
	.read_bonded_bt_address = nvm_read_bonded_bt_address,
	.write_bonded_bt_address = nvm_write_bonded_bt_address,
	.read_date_time = nvm_read_date_time,
	.write_date_time = nvm_write_date_time,
	.read_single_alarm_clock = nvm_read_single_alarm_clock,
	.write_single_alarm_clock = nvm_write_single_alarm_clock,
	.read_all_alarm_clock = nvm_read_all_alarm_clock,
	.write_all_alarm_clock = nvm_write_all_alarm_clock,
	.read_display_setting = nvm_read_display_setting,
	.write_display_setting = nvm_write_display_setting,
	.read_personal_info = nvm_read_personal_info,
	.write_personal_info = nvm_write_personal_info,
	.read_personal_sport_goal = nvm_read_personal_sport_goal,
	.write_personal_sport_goal = nvm_write_personal_sport_goal,
	.read_current_sport_data_index = nvm_read_current_sport_data_index,
	.write_current_sport_data_index = nvm_write_current_sport_data_index,
	.read_current_sport_data_total = nvm_read_current_sport_data_total,
	.write_current_sport_data_total = nvm_write_current_sport_data_total,
	.read_total_sport_data = nvm_read_total_sport_data,
	.write_total_sport_data = nvm_write_total_sport_data,
	.read_sleep_data = nvm_read_sleep_data,
	.write_sleep_data = nvm_write_sleep_data,
	.read_sport_date = nvm_read_sport_date,
	.write_sport_date = nvm_write_sport_date,
};

nvm_t *nvm_get(void)
{
    return &nvm_usr;
}
