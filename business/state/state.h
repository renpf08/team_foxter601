#ifndef STATE_H
#define STATE_H

s16 state_clock(REPORT_E cb, void *args);
s16 state_low_voltage(REPORT_E cb, void *args);
s16 state_zero_adjust(REPORT_E cb, void *args);
s16 state_ble_switch(REPORT_E cb, void *args);
s16 state_notify(REPORT_E cb, void *args);
s16 state_battery_week_switch(REPORT_E cb, void *args);
s16 state_run_test(REPORT_E cb, void *args);
s16 state_time_adjust(REPORT_E cb, void *args);
s16 state_set_date_time(REPORT_E cb, void *args);

#endif
