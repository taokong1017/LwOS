#ifndef __TICK_H__
#define __TICK_H__

#define MS_PER_SECOND 1000UL
#define US_PER_SECOND 1000000UL
#define NS_PER_SECOND 1000000000UL
#define PER_MS 1000UL
#define NS_PER_MS 1000000UL
#define NS_PER_US 1000UL

void tick_announce();
uint64_t current_ticks_get();
uint64_t current_cycles_get();
uint32_t ms2tick(uint32_t ms);
uint32_t tick2ms(uint32_t tick);
uint32_t us2tick(uint32_t us);
uint32_t tick2us(uint32_t tick);
void udelay(uint32_t us);
void mdelay(uint32_t ms);

#endif
