#ifndef _HIGHLEVEL_DRIVERS_FPGA_H_
#define _HIGHLEVEL_DRIVERS_FPGA_H_

#define DATA_SOP 0xBABA1A9A

void ResetFpga();
void Init(uint32_t * regsContentIn, uint32_t opLen);
void SetDelay(uint32_t delayIn);
void SetMode(uint32_t modeIn);
void GetEventNumber(uint32_t * extTrigCount, uint32_t * intTrigCount);
void EventReset();
void Calibrate(uint32_t calibIn);
void intTriggerPeriod(uint32_t periodIn);
void selectTrigger(uint32_t intTrigIn);
void configureTestUnit(uint32_t tuCfg);
int getEvent(uint32_t *evt, int evtLen);

#endif //_HIGHLEVEL_DRIVERS_FPGA_H_
