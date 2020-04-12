#ifndef UTILS_H
#define UTILS_H
#include <stdlib.h>

int weighted_choice(double *weights,int num_of_weights);
bool trigger(double prob);


//distributions
void normal(double *val,int num,double mean,double var);
void poisson(int *val,int num,double lambda);
void uniform(double *val,int num,double a,double b);

double distance(double lat,double lon,double centreLat,double centreLong);
inline double deg2rad(double deg) { return deg*3.14159/180.0; }
inline double rad2deg(double rad) { return rad*180.0/3.14159; }
inline double meters2rad(double meters) { return meters/400075000.0*3.14159*2.0; }



#endif

