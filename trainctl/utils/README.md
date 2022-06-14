#  Utils


- lf_mqueue: lock-free circular buffer, use by *msg* subsystem

  Important : there shall be a single writer thread  and a single reader thread in these circular buffer
  
- itm_debug : provides basic trace macro using ITM (SWO) stm32 trace facilities

