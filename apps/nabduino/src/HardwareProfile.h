#ifndef _HARDWAREPROFILE_H_
#define _HARDWAREPROFILE_H_

#define GetSystemClock()                                                                (41666667ul)
#define GetInstructionClock()                                                           (GetSystemClock() / 4ul)
#define GetPeripheralClock()                                                            GetInstructionClock()

#endif
