/*
Arduino Library for splitting a RTCM-Stream input byte by byte into seperated messages.

GNU GPL v3
by hagre 2019
*/


#include "Arduino.h"
#include "rtcmstreamsplitter.h"

RTCMStreamSplitter::RTCMStreamSplitter()
{
    _inLine = false;
    _receiveStreamLengthCtr = -1;
}

uint16_t RTCMStreamSplitter::inputByte(uint8_t input) //ToDo Protocoll check
{ 
    _receiveStreamLengthCtr ++;
    _receiveStream[_receiveStreamLengthCtr] = input;
    if (_receiveStreamLengthCtr < 2){
        _inLine = false;
        return 0;
    }
    else {
        if (_receiveStream[_receiveStreamLengthCtr - 2] == 0xD3 && _inLine == false){ // 211 Präamble
            if ((_receiveStream[_receiveStreamLengthCtr - 1] & B11111100) == 0){ //have to be always == 0
                _inLine = true;
                for (uint16_t i = 0; i < MAX_RTCM_BUFFFERSIZE ; i++){ //reset/clear outputstream
                    outputStream [i] = 0;
                }
                outputStreamLength = 0; //reset/clear outputstream
                _receiveStreamLength = ((_receiveStream[_receiveStreamLengthCtr -1] & B00000011) << 8 ) + (_receiveStream[_receiveStreamLengthCtr]);
                _receiveStream[0] = _receiveStream[_receiveStreamLengthCtr - 2];
                _receiveStream[1] = _receiveStream[_receiveStreamLengthCtr - 1];
                _receiveStream[2] = _receiveStream[_receiveStreamLengthCtr];
                _receiveStreamLengthCtr = 2;
            }
        }
		else if (_receiveStreamLengthCtr > MAX_RTCM_BUFFFERSIZE - 5 && _inLine == false){ // short bevor buffer overrun and still nothing found yet
            if (_receiveStream[_receiveStreamLengthCtr - 1] == 0xD3){ // 211 Präamble just started
                if ((_receiveStream[_receiveStreamLengthCtr] & B11111100) == 0){ //have to be always == 0 and next byte is fitting for "inline"
                    _receiveStream[0] = _receiveStream[_receiveStreamLengthCtr - 1];
                    _receiveStream[1] = _receiveStream[_receiveStreamLengthCtr];
                    _receiveStreamLengthCtr = 1; //seting everything up to "discover" a new inline in the next loop
                }
                else {
                    _receiveStreamLengthCtr = -1;
                }
            }
            else {
                _receiveStreamLengthCtr = -1;
            }    
        }
		
        if (_inLine == true && _receiveStreamLengthCtr >= _receiveStreamLength + 5){
            hashData (0, _receiveStreamLength+3);
            if (_inputStreamHash[0] == _receiveStream[_receiveStreamLengthCtr - 2]){
                if (_inputStreamHash[1] == _receiveStream[_receiveStreamLengthCtr - 1]){ 
                    if (_inputStreamHash[2] == _receiveStream[_receiveStreamLengthCtr]){
                        _inLine = false;
                        uint16_t receiveStreamType = (uint16_t)(_receiveStream[3] << 4 ) + (uint16_t)((uint16_t)(_receiveStream[4] & B11110000) >> 4);
                        outputStreamLength = _receiveStreamLength + 6;
                        for (uint16_t i = 0; i < outputStreamLength; i++){
                            outputStream [i] = _receiveStream[i];
                        }
                        _receiveStreamLengthCtr = -1;
                        return receiveStreamType;
                    }
                }
            }
            _receiveStreamLengthCtr = -1;
        }
    }
    return 0;
}

void RTCMStreamSplitter::hashData(uint16_t startctr,uint16_t lengthctr)
{
    uint32_t crc = 0;
    for (int16_t i = 0; i < lengthctr; i++) {
	    crc = (crc<<8) ^ crc24q[(_receiveStream[i + startctr]) ^ (uint8_t)(crc >> 16)]; 
    }
    _inputStreamHash[0] = (crc >> 16) & 0xFF;
    _inputStreamHash[1] = (crc >> 8) & 0xFF;
    _inputStreamHash[2] = crc & 0xFF;
}
