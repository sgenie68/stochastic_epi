#include "utils.h"
#include <random>
#include <chrono>
#include <stdio.h>
#include <math.h>

using namespace std;

static std::mt19937 generator(std::chrono::system_clock::now().time_since_epoch().count());

int weighted_choice(double *weights,int num_of_weights)
{
 	std::uniform_real_distribution<double> distribution(0.0,1.0);
	std::vector<double> cumsum;
	double sum=0.0,x;

	if(num_of_weights<2)
		return 0;
	cumsum.reserve(num_of_weights);
	//normalize
	for(int i=0;i<num_of_weights;i++)
	{
		sum+=weights[i];
		cumsum[i]=0.0;
	}
	for(int i=0;i<num_of_weights;i++)
		weights[i]*=1.0/sum;

	cumsum[0]=weights[0];
	for(int i=1;i<num_of_weights;i++)
		cumsum[i]=weights[i]+cumsum[i-1];
	
	x=distribution(generator);
	for(int i=0;i<num_of_weights;i++)
		if(x<cumsum[i])
			return i;
	return -1;
}

bool trigger(double prob)
{
	double weights[2]={prob,1.0-prob};

	return (weighted_choice(weights,2)==0);
}


void uniform(double *val,int num,double a,double b)
{
 	std::uniform_real_distribution<double> distribution(a,b);
	
	for(int i=0;i<num;i++)
	{
		val[i]=distribution(generator);
	}
}

void normal(double *val,int num,double mean,double var)
{
 	std::normal_distribution<double> distribution(mean,var);
	
	for(int i=0;i<num;i++)
	{
		val[i]=distribution(generator);
	}
}


void poisson(int *val,int num,double lambda)
{
 	std::poisson_distribution<int> distribution(lambda);
	
	for(int i=0;i<num;i++)
	{
		val[i]=distribution(generator);
	}
}


#if 0
double distance(double lat1,double lon1,double lat2,double lon2)
{
	const double R=6371e3;
	double deltaphi=(lat2-lat1);
	double deltalam=(lon2-lon1);
	double a=sin(deltaphi/2.0) * sin(deltaphi/2.0) +
        		cos(lat1) * cos(lat2) * sin(deltalam/2.0) * sin(deltalam/2.0);
	double c=2.0*atan2(sqrt(a),sqrt(1.0-a));
	
	return R*c;

}
#else
double distance(double lat1,double lon1,double lat2,double lon2)
{
	const double earthRadius=6371000.0;

	double u=sin((lat2 - lat1)/2.0);
  	double v=sin((lon2 - lon1)/2.0);
  	return 2.0*earthRadius*asin(sqrt(u * u + cos(lat1) * cos(lat2) * v * v));	
}
#endif


#ifdef DEBUG

int main()
{
	double v;
	int vi;

	for(int i=0;i<10;i++)
		printf("Trigger: %d\n",trigger(0.8));
	normal(&v,1,5.0,7.0);	
	printf("Normal(5,2)=%lf\n",v);
	poisson(&vi,1,5.0);	
	printf("Poisson(5)=%d\n",vi);
	return 0;
}

#endif
