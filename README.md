# RTCM_Stream_Splitter
Arduino Library for splitting a RTCM-Stream input byte by byte into seperated messages
(GNU GPL V3)

uint16_t inputByte(uint8_t input); - input byte by byte - when msg completed returning message type as integer - else 0

uint8_t outputStream[MAX_RTCM_BUFFFERSIZE]; public array - as output of msg

uint16_t outputStreamLength; public int - as output of msg

#define MAX_RTCM_BUFFFERSIZE could be defined outside of this library
