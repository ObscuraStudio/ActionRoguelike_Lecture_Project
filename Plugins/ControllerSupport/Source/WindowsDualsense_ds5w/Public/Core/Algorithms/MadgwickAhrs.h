#pragma once

#include "CoreMinimal.h"
#include <cmath>

class FMadgwickAhrs
{
public:
	FMadgwickAhrs( const float SampleFreq = 200.0f, const float Beta = 0.08f);

    // gx,gy,gz in rad/s, ax,ay,az in m/s^2, dt in seconds
    void UpdateImu(float gx, float gy, float gz, float ax, float ay, float az, float dt);
    void GetEuler(float &Roll, float &Yaw, float &Pitch);
    void SetBeta( const float BetaValue);
	void SetSampleFreq( const float Freq);
	void GetQuaternion(float &Nq0, float &Nq1, float &Nq2, float &Nq3) const;

	void Reset();

private:
    float Beta;
    float SampleFreq;
    float q0, q1, q2, q3;
};
