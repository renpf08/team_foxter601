#ifndef STATE_H
#define STATE_H

s16 state_clock(REPORT_E cb, void *args);
s16 state_low_voltage(REPORT_E cb, void *args);
s16 state_zero_adjust(REPORT_E cb, void *args);
s16 state_ble_switch(REPORT_E cb, void *args);

#endif
