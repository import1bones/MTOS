# MOS
Mute OS
## Memory Structure
know *memory structure* is important for system,which memory can be allocated,which can't.this is a job of system.

	+----------+ <-0X FFFF FFFC
	|up to 4GB |
	|	   |
	\/\/\/\/\/\/

	/\/\/\/\/\/\
	|          |
	|extend RAM|
	+----------+ <-0X 0010 0000
	+----------+ <-0X 000F FFFC
	|BIOS   ROM|
	+----------+ <-0X 000F 0000
	+----------+ <-0X 000E FFFC
	|16 bits   |
	|devices,  |
	|expansion |
	|ROMs      |
	+----------+ <-0X 000C 0000
	+----------+ <-0X 000B FFFC
	|   VGA    |
	|  Display |
	+----------+ <-0X 000A 0000
	+----------+ <-0X 0009 FFFC
	|    LOW   |
	|  Memory  |
	+----------+ <-0X 0000 0000
	|<-4bytes->| HEAD Address of 4 bytes(32 bits)

physical address = 16 * segment + offset
