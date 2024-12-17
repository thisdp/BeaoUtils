#pragma once
#include <Arduino.h>

class DynamicAverageFilter{
private:
	uint8_t writeIndex;
	uint8_t dataRecorded;
	uint16_t dataLength;
	float *data;
public:
	float average;
	DynamicAverageFilter(uint16_t len){
		dataLength = len;
		data = new float[len];
		for(uint8_t i=0;i<dataLength;i++) data[i] = 0;
		writeIndex = 0;
		dataRecorded = 0;
	};
	void append(float d){
		data[writeIndex++] = d;
		writeIndex %= dataLength;
		if(dataRecorded<dataLength) dataRecorded++;
		float sum = 0;
		for(uint8_t i=0;i<dataRecorded;i++) sum += data[i];
		average = sum/dataRecorded;
	}
	void clear(){
		for(uint8_t i=0;i<dataLength;i++) data[i] = 0;
		writeIndex = 0;
		dataRecorded = 0;
		average = 0;
	}
};

template <uint16_t DataLen>
class AverageFilter {
private:
		uint8_t writeIndex;
		uint8_t dataRecorded;
		float data[DataLen]; // 静态数组
public:
		float average;
		AverageFilter() : writeIndex(0), dataRecorded(0), average(0.0f) {
				for (uint8_t i = 0; i < DataLen; ++i) {
						data[i] = 0;
				}
		}
		void append(float d) {
				data[writeIndex++] = d;
				writeIndex %= DataLen;
				if (dataRecorded < DataLen) dataRecorded++;
				float sum = 0;
				for (uint8_t i = 0; i < dataRecorded; ++i) sum += data[i];
				average = sum / dataRecorded;
		}
		void clear() {
				for (uint8_t i = 0; i < DataLen; ++i) data[i] = 0;
				writeIndex = 0;
				dataRecorded = 0;
				average = 0;
		}
};