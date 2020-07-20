#ifndef STATE_H
#define STATE_H

s16 state_clock(REPORT_E cb, void *args);
s16 state_low_voltage(REPORT_E cb, void *args);

s16 state_zero_adjust(REPORT_E cb, void *args);
void zero_adjust_test(u16 id);

s16 state_ble_switch(REPORT_E cb, void *args);
//void ble_switch_test(u16 id);

s16 state_notify(REPORT_E cb, void *args);
void notify_test(void);

s16 state_battery_week_switch(REPORT_E cb, void *args);
s16 state_battery_week_status_get(void);
//void battery_week_test(adapter_callback cb);

s16 state_run_test(REPORT_E cb, void *args);
//void test_run_test(u16 id);

s16 state_time_adjust(REPORT_E cb, void *args);
void time_adjust_test(u16 id);

s16 state_set_date_time(REPORT_E cb, void *args);
s16 state_nvm_access(REPORT_E cb, void *args);
s16 state_reboot(REPORT_E cb, void *args);
s16 state_step_get(REPORT_E cb, void *args);

#endif
