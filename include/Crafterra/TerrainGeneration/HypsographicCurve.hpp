/*##########################################################################################

	Crafterra Library 🌏

	[Planning and Production]
	2017-2022 Kasugaccho
	2018-2022 As Project

	[Contact Us]
	wanotaitei@gmail.com
	https://github.com/AsPJT/Crafterra

	[License]
	Distributed under the CC0 1.0.
	https://creativecommons.org/publicdomain/zero/1.0/

##########################################################################################*/

#ifndef INCLUDED_CRAFTERRA_LIBRARY_CRAFTERRA_TERRAIN_GENERATION_HYPSOGRAPHIC_CURVE
#define INCLUDED_CRAFTERRA_LIBRARY_CRAFTERRA_TERRAIN_GENERATION_HYPSOGRAPHIC_CURVE

#include <AsLib2/DataType/PrimitiveDataType.hpp> // int
#include <cmath>

namespace Crafterra {

    constexpr double math_pi = 3.14159265358979323846;

    double funcG(double x_, double mountainousness_){
        return atan((2.0*x_-1.0)*tan(mountainousness_* math_pi *0.5))/(mountainousness_* math_pi *0.5)*0.5+0.5;
    }

    double funcA(double x_, double mountainousness_){
        return (funcG(x_, mountainousness_)-0.5)*(mountainousness_*0.5+0.5)+0.5;
    }

    double funcB(double x_){
        double a = (2.0*x_-2.0);
        return a*a*(1.0 + a)*0.25+1.0;
    }

    double funcC(double x_){
        return (tan((2.0*x_-1.0)*0.25* math_pi)*pow(abs(2.0*x_-1.0), 2.0*x_)+1.0)*0.5;
    }

    // 地形の標高分布を表す曲線
    double processNoiseUsingHypsographicCurve(double noise_, double min_height_, double max_height_, double mountainousness_, double water_height){

        double noise_adj;

        double water_prop = (water_height-min_height_)/(max_height_-min_height_); // 海面率

        if(noise_ > water_prop){
            noise_adj = (noise_-water_prop)/(1.0-water_prop)*0.5+0.5;
        } else {
            noise_adj = (noise_/water_prop)*0.5;
        }

        double fa = funcA(noise_adj, mountainousness_*0.5+0.5);
        double fr = funcB(fa)*funcC(fa);

        if(fr > water_prop){
            fr = (2.0*fr-1.0)*(1.0-water_prop)+water_prop;
        } else {
            fr = (2.0*fr)*water_prop;
        }
        
        if(fr < 0.0) fr = 0.0; // never
        if(fr > 1.0) fr = 1.0; // never

        return fr;

    }
}

#endif //Included Crafterra Library