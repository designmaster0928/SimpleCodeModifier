//KVtargetAlgoritm(CurrentKVData, 0); // every 30 minutes

// sensor readings every second (except when changing zone)
// pump head, flow, temperatures
// No power from HP

//#include "debugapi.h"
#include <iostream>

#include <fstream>
#include <string.h>
#include "string"

#include "C:\Users\klaus\Dropbox\CreativeElectronics\Grundfos\Optimizer2\KVAlgoritm\AlgoFunctions.h"


int counter;
int i;

float value1;

using namespace std;

extern int SystemMode;


extern KVStruct KValgo[KV_ARRAY_SIZE];			// array with 400k points, all KV data

extern double KVarrayData[KV_ARRAY_SIZE];		// Simulates raw data from the KV detection function

extern double HouseArray[HOUSE_ARRAY_SIZE];	// Data for House Curve
extern double HouseArrayx[HOUSE_ARRAY_SIZE];	// x values, used for regression analysis

extern HouseCurveStruct HouseModel;


double KVmeas(long index) {
	// Function emulates the KV measurement
	// gets the KV value at a certain index
	return (KVarrayData[index]);
}

void LoadKVdatafromFile(void) {
	// Get KV data from file and load into array
	ifstream fin("KV_EmulationNom_Mod_DK.csv");
	string strline, strremain, strCol1, strout;

	string delimeter = ";";

	double KVtmpvar1, KVtmpvar2;

	char tmpstr[100];
	char* strptr;

	long LineNo;
	int d1;

	LineNo = 0;
	while (!fin.eof()) {
		getline(fin, strline, '\n'); // get line
		d1 = strline.find(';'); // find delimiter

		strCol1 = strline.substr(0, d1); // parse first Column
		d1++;
		strremain = strline.substr(d1); // remaining line

		//std::cout << "LineNo:" << LineNo << "\n";
		//std::cout << strCol1 << "\n";
		//std::cout << strremain << "\n";

		//std::string num = "0.6";

		// Store in array
		KVtmpvar1 = ::atof(strCol1.c_str());
		KVtmpvar2 = ::atof(strremain.c_str());

		KVarrayData[LineNo] = KVtmpvar2;

		LineNo++;

		//if (LineNo >= 1000) break;
		if (LineNo > KV_ARRAY_SIZE-1) break;
		
	}

	fin.close();


}

void stdoutcheck(void) {

	counter = 1;

	std::cout << "Optimizer\n";

	ifstream fin("Infile.csv");
	ofstream fout("OutFile.csv");
	string strline, strremain, strCol1, strout;

	string delimeter = ";";

	char tmpstr[100];
	char* strptr;

	int LineNo;
	int d1;
	double Outdoor, EnergyCost;

	typedef struct {
		double Outdoor;
		double EnergyCost;
	} ForecastValuesStruct;

	ForecastValuesStruct ForecastValues[100]; // forecast values for 24 hours
	strptr = tmpstr;

	LineNo = 0;
	while (!fin.eof()) {
		getline(fin, strline, '\n'); // get line
		d1 = strline.find(';'); // find dilimiter

		strCol1 = strline.substr(0, d1); // parse first Column
		d1++;
		strremain = strline.substr(d1); // remaining line

		std::cout << "LineNo:" << LineNo << "\n";
		std::cout << strCol1 << "\n";
		std::cout << strremain << "\n";

		std::string num = "0.6";

		Outdoor = ::atof(strCol1.c_str());
		EnergyCost = ::atof(strremain.c_str());

		ForecastValues[LineNo].Outdoor = Outdoor;
		ForecastValues[LineNo].EnergyCost = EnergyCost;


		//Outdoor = strtof(strCol1, NULL);



		//strcpy(strptr, strCol1);

		//value1 = strtof(strptr, NULL);

		strout.append(strCol1);
		strout.append(delimeter);

		fout << strout << endl; //out file line
		LineNo++;
	}

	fin.close();
	fout.close();


	for (i = 0; i < 10; i++) {
		counter = counter * 2;
		counter = counter * 2;
		std::cout << "counter" << counter << "\n";
	}

}

void MoveKVarray(void) {
	// Function moves the entire array one tick (so [0] is free for new data)
	// Bad efficiency, but easier for debugging right now
	long i;

	for (i = KV_ARRAY_SIZE-1; i > 0; i--) {
		// Move one pos
		KValgo[i].kv_UFH = KValgo[i-1].kv_UFH;
		KValgo[i].kv_mean_UFH = KValgo[i-1].kv_mean_UFH;
		KValgo[i].kv_high_UFH = KValgo[i-1].kv_high_UFH;
		KValgo[i].kv_low_UFH = KValgo[i-1].kv_low_UFH;
		KValgo[i].kv_target_UFH = KValgo[i-1].kv_target_UFH;

	}

}



void PrintKVArray(void) {
	// Function prints KV array
	long i;
	double test1;

	ofstream fout("KVOutFile.csv");
	string strline, strremain, strCol1, strout;

	string delimeter = ",";

	char tmpstr[100];
	char* strptr;
	strptr = tmpstr;
	test1 = 11.1;

	//fout << "ArrayIndex; KV_UFH; KV_MEAN_UFH; KV_HIGH_UFH; KV_LOW_UFH; KV_TARGET_UFH" << "\n";


	for (i = 0; i < KV_ARRAY_SIZE; i++) {
		//strout.append(strCol1);
		//strout.append(delimeter);
		//KValgo[i].kv_mean_UFH = KValgo[i - 1].kv_mean_UFH;
		//KValgo[i].kv_high_UFH = KValgo[i - 1].kv_high_UFH;
		//KValgo[i].kv_low_UFH = KValgo[i - 1].kv_low_UFH;
		//KValgo[i].kv_target_UFH = KValgo[i - 1].kv_target_UFH;

		fout << i << delimeter << KValgo[i].kv_UFH << delimeter << KValgo[i].kv_mean_UFH << delimeter << KValgo[i].kv_high_UFH << delimeter << KValgo[i].kv_low_UFH << delimeter << KValgo[i].kv_target_UFH << delimeter << "\n";


	}
	
	//fout << strout << endl; //out file line
	fout << endl; //out file line

	fout.close();
		
}

void PrintHeatCurve(void) {
	// Function prints Heat Curve array
	long i;
	double test1;

	ofstream fout("HeatOutFile.csv");
	string strline, strremain, strCol1, strout;

	string delimeter = ",";

	char tmpstr[100];
	char* strptr;
	strptr = tmpstr;
	test1 = 11.1;

	fout << "ArrayIndex" << delimeter << "Index" << delimeter << "Xvalues" << delimeter << "Power" << "\n";


	for (i = 0; i < HOUSE_ARRAY_SIZE; i++) {
		fout << i << delimeter << HouseArrayx[i] << delimeter << HouseArray[i] << delimeter << "\n";
	}
	fout << endl; //out file line

	fout.close();

}


int main(void) {
	
	int i;
	long timestamptick;
	double CurrentKVData;
	double AmbientTemp;
	//stdoutcheck();

	test();

	// Load KV data for test period into array
	LoadKVdatafromFile();
	AmbientTemp = 20;
	// Emulate that time is ticking by
	// We are running in House mode only
	SystemMode = ONLYHEATING;
		
	// run the KValgorimn from the start to end of the KVarray data
	timestamptick = 0;

	while (timestamptick < 10000) {
		// Get data from KV measurement
		CurrentKVData = KVmeas(timestamptick);
		// Move data one tick (data free at [0])
		MoveKVarray();
		// run KV algoritm
		// Copy current KV data into [0] position
		KVtargetAlgoritm(CurrentKVData, 0); // every 30 minutes
	
		// Update HouseModel
		if (SystemMode == ONLYHEATING) {
			// Allowed to update HouseModel since only HP is heating house (TBD what about delays?)
			UpdateHeatCurve(AmbientTemp, CurrentKVData);
			// Adjust temperature
			BetaAdjust();
			// Run update of heat pump
			HPPowerSetpoint(AmbientTemp);
		}
		timestamptick++;
	}

	// Print array
	PrintKVArray();
	timestamptick = 0;

	// Load default heat curve
	LoadDefaultHeatCurve();

	// Print heat curve
	PrintHeatCurve();

	// endless loop
	while (1) {
		// Run temperature setpoint
	}

	i = 0;
}