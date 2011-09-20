void HPM_Init(void);           // initialize the UPC unit
void HPM_Start(char *label);   // start counting in a block marked by the label
void HPM_Stop(char *label);    // stop counting in a block marked by the label
void HPM_Print(void);          // print counters for all blocks
