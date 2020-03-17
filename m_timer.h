
#ifndef __M_TIMER_H__
#define __M_TIMER_H__

/**
*  @file    m_timer.h
*  @author  maliwen
*  @date    2020/02/20
*  @history 1.0.0.0(Version)
*           1.0.0.0: Creat              2020/02/20
*/

typedef struct
{
  volatile uint8 year;
  volatile uint8 month;
  volatile uint8 day;
  volatile uint8 hour;
  volatile uint8 minute;
  volatile uint8 second;
} time_t;

time_t* m_get_time(void);
void m_timer_init(void);
void m_timer_set(uint8* timeStr);

#endif /** end of __M_TIMER_H__ */
