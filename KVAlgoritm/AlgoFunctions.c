// Source file for Algoritm functions
// Shared with ESP32 code
#include "C:\Users\klaus\Dropbox\CreativeElectronics\Grundfos\Optimizer2\KVAlgoritm\AlgoFunctions.h"


// Variables

KVStruct KValgo[KV_ARRAY_SIZE];			// array with 400k points, all KV data

double KVarrayData[KV_ARRAY_SIZE];		// Simulates raw data from the KV detection function

double HouseArray[HOUSE_ARRAY_SIZE];	// Data for House Curve
double HouseArrayx[HOUSE_ARRAY_SIZE];	// x values, used for regression analysis


HouseCurveStruct HouseModel;

int SystemMode;
// Functions


int test() {
	return 5;
}

void KVtargetAlgoritm(double KVval, long timestamp) {
	// KV algoritm
	// Takes in current measured KV point and timestamp for that point
	// Stores data in KV struct array, and calculates LOW, HIGH and TARGET
	// First sample [0] is newest sample. [400.000] is oldest  

	int i;
	double average;


	// Save KV value
	KValgo[timestamp].kv_UFH = KVval;

	// Calculate KV average
	average = 0;
	for (i = 0; i < KV_MEAS_AVGSAMPLES; i++) {
		average += KValgo[timestamp].kv_UFH;
	}
	average = average / (double)KV_MEAS_AVGSAMPLES;
	KValgo[timestamp].kv_mean_UFH = average;

	// Calculate envelope
	// Check if envelope is broken, then update envelope
	if (KValgo[timestamp].kv_UFH > KValgo[timestamp].kv_high_UFH) {
		// Update high value
		KValgo[timestamp].kv_high_UFH = KValgo[timestamp].kv_UFH;
	}
	if (KValgo[timestamp].kv_UFH < KValgo[timestamp].kv_low_UFH) {
		// Update low value
		KValgo[timestamp].kv_low_UFH = KValgo[timestamp].kv_UFH;
	}
	// Decay envelope
	// Linear regression over 480 samples, from high to mean value
	// high regression
	KValgo[timestamp].kv_high_UFH -= ((KValgo[timestamp].kv_high_UFH - KValgo[timestamp].kv_mean_UFH) / (double)KV_ENVELOPE_DECAY);
	// low regression
	KValgo[timestamp].kv_low_UFH += ((KValgo[timestamp].kv_mean_UFH - KValgo[timestamp].kv_low_UFH) / (double)KV_ENVELOPE_DECAY);

	// Set target (percentage between KV_high and KV_mean)
	KValgo[timestamp].kv_target_UFH = KValgo[timestamp].kv_mean_UFH + ((KValgo[timestamp].kv_high_UFH - KValgo[timestamp].kv_mean_UFH) * (double)KV_AMBITION_UFH);



}

void LoadDefaultHeatCurve(void) {
	// Deault heat curve is loaded into HouseCurveArray
	// Curve extends from -20 to 30 degrees ambient
	// For each point 20 samples are summed. Adding new value then has an effect of 5% on the stored value
#define W_PER_M2 0.063
#define M2 150
#define DELTA -20

//#define HouseAmbMaxDelta 30 - -20;
#define HOUSEAMBMIN -20
#define HOUSEAMBMAX 30
#define HOUSEAMBRANGEDELTA HOUSEAMBMAX - HOUSEAMBMIN


	double MaxPoweratminus15;	// Max power for house at -15 ambient
	// At - 15, delta is 35. Poff = a * Tout P = 0. At 0 degrees (-20 delta), DeltaTemp is 20
	MaxPoweratminus15 = M2 * W_PER_M2;
	HouseModel.AlphaDefault = MaxPoweratminus15 / 35.0;
	HouseModel.BetaDefault = (double)DELTA * HouseModel.AlphaDefault;


	int i;
	double AmbTemp; //	Variable for the x axis, ambient temperature
	double Phouse;	// House estimated power

	for (i = 0; i < HOUSE_ARRAY_SIZE; i++) {
		AmbTemp = ((double)i / (double)HOUSE_ARRAY_SIZE) * (double)HOUSEAMBRANGEDELTA;
		Phouse = HouseModel.AlphaDefault * AmbTemp + HouseModel.BetaDefault;
		HouseArray[i] = Phouse;
		HouseArrayx[i] = (double)i; // Just for regression

	}

	// Set Modifier shift
	HouseModel.BetaMod = 0;
}

#define SIMPLE_LINEAR_REGRESSION_ERROR_INPUT_VALUE -2
#define SIMPLE_LINEAR_REGRESSION_ERROR_NUMERIC     -3

int simple_linear_regression(const double* x, const double* y, const int n, double* slope_out, double* intercept_out, double* r2_out) {
	double sum_x = 0.0;
	double sum_xx = 0.0;
	double sum_xy = 0.0;
	double sum_y = 0.0;
	double sum_yy = 0.0;
	double n_real = (double)(n);
	int i = 0;
	double slope = 0.0;
	double denominator = 0.0;

	if (x == NULL || y == NULL || n < 2) {
		return SIMPLE_LINEAR_REGRESSION_ERROR_INPUT_VALUE;
	}

	for (i = 0; i < n; ++i) {
		sum_x += x[i];
		sum_xx += x[i] * x[i];
		sum_xy += x[i] * y[i];
		sum_y += y[i];
		sum_yy += y[i] * y[i];
	}

	denominator = n_real * sum_xx - sum_x * sum_x;
	if (denominator == 0.0) {
		return SIMPLE_LINEAR_REGRESSION_ERROR_NUMERIC;
	}
	slope = (n_real * sum_xy - sum_x * sum_y) / denominator;

	if (slope_out != NULL) {
		*slope_out = slope;
	}

	if (intercept_out != NULL) {
		*intercept_out = (sum_y - slope * sum_x) / n_real;
	}

	if (r2_out != NULL) {
		denominator = ((n_real * sum_xx) - (sum_x * sum_x)) * ((n_real * sum_yy) - (sum_y * sum_y));
		if (denominator == 0.0) {
			return SIMPLE_LINEAR_REGRESSION_ERROR_NUMERIC;
		}
		*r2_out = ((n_real * sum_xy) - (sum_x * sum_y)) * ((n_real * sum_xy) - (sum_x * sum_y)) / denominator;
	}

	return 0;
}



void UpdateHeatCurve(double AmbTemp, double HPPower) {
	// Function takes in current KVData and ambient temperature, updates house model
	// Then updates fitted house curve
	// Find closest match to index
#define HOUSESAMPLESPERPOINT 20 // 20 samples per point. Averaged when adding new

	int index;

	index = round((double)HOUSE_ARRAY_SIZE * ((AmbTemp - (double)HOUSEAMBMIN) / (double)HOUSEAMBRANGEDELTA));
	// New addition only has 5% impact (1/20)
	HouseArray[index] = HouseArray[index] * (1.0 - (1.0 / (double)HOUSESAMPLESPERPOINT)) + HPPower / (double)HOUSESAMPLESPERPOINT;

	// Fit house curve
	// Use center mass function to create new Ax+B (https://www.bragitoff.com/2018/05/linear-fitting-c-program/)
	// https://stackoverflow.com/questions/5083465/fast-efficient-least-squares-fit-algorithm-in-c
	HouseModel.Alpha = HouseModel.AlphaDefault; // for now, use default
	HouseModel.Beta = HouseModel.BetaDefault;

	double slope = 0.0;
	double intercept = 0.0;
	double r2 = 0.0;
	int res = 0;

	res = simple_linear_regression(HouseArrayx, HouseArray, sizeof(HouseArrayx) / sizeof(HouseArrayx[0]), &slope, &intercept, &r2);
	HouseModel.Alpha = slope;
	HouseModel.Beta = intercept;

}

double HPPowerSetpoint(double AmbTemp) {
	// Function that calculates the temperature setpoint for the heatpump
	// Based on HouseCurve, added BetaMod to shift curve vertically
	double Setpoint;

	Setpoint = HouseModel.Alpha * AmbTemp + HouseModel.Beta + HouseModel.BetaMod; // TBD temperature + power??
	return Setpoint;
}

void BetaAdjust(void) {
	// Funcion determines of the house curve needs to be shifted to regulate the heat pump temperature
#define BetaStep 0.5

	if (KValgo[0].kv_UFH > KValgo[0].kv_target_UFH) {
		// Increase by a defined step
		HouseModel.Beta += BetaStep;
	}
	else {
		// Decrease by a defined step
		HouseModel.Beta -= BetaStep;
	}

}

