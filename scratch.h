

void scratchpad_erase();
void scratchpad_write(char xdata*src, char xdata*dest, unsigned char data quantity);
void scratchpad_read(char code*src, char xdata*dest, unsigned char data quantity);

void flash_erase();
void flash_write(char xdata*src, char xdata*dest, unsigned int data quantity);
void flash_read(char code*src, char xdata*dest, unsigned int data quantity);
void flash_lock();