#include <string.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include "area.h"
#include "utils.h"
#include "const.h"


using namespace std;


Person::Person():m_age(0),m_duration(0),m_quarantine(false)
{
	initialise();
}

Person::~Person()
{
}

bool Person::next_epoch()
{
	m_age++;
	return !trigger(death());
}

void Person::initialise()
{
	double val=0.0;

	while(val<=0.0)
		normal(&val,1,DURATION_MEAN,DURATION_STDEV);
	m_duration=floor(val);
}


double Person::death()
{
	//Need to make probabolity of death age-related
	return 0.001;	
}

int Person::contacts()
{
	int val;

	poisson(&val,1,2);
	//Make number of contacts related to the age
	return val;
}

int Person::contaminate(int contacts)
{
	double rate=0.7;

	if(age()<3)
		rate=0.15;
	else if(age()>15)
		rate=0.1;
	if(trigger(rate))
		rate=1.0;
	else
		rate=0.0;
	return floor((quarantine()?0:contacts)*rate); 
}


AreaBase::AreaBase():m_epoch(0)
{
}


AreaBase::~AreaBase()
{
}

void AreaBase::next_epoch()
{
	m_epoch++;
}

bool AreaBase::initialise(const std::string& name)
{
	return true;
}

void AreaBase::run()
{
	for(PERSONS::iterator it=m_sick.begin();it!=m_sick.end();)
	{
		SPERSON p=*it;
		p->touched()=0;
		if(!p->next_epoch())
		{
			m_dead.push_back(p);
		}
		else if(p->duration()<=p->age())
		{
			m_recovered.push_back(p);
		}
		else
		{
			//Contaminate neighbours
	 		p->touched()+=p->contaminate(p->contacts());
//			if(p->age()>2)
//				p->quarantine(true);
			++it;
			continue;
		}
		m_sick.erase(it++);
	}
}
 

AreaRank::AreaRank():AreaBase(),distr_vector(NULL)
{
	MPI_Comm_size(MPI_COMM_WORLD, &m_nodesnum);
	MPI_Comm_rank(MPI_COMM_WORLD, &m_rank);
}

AreaRank::~AreaRank()
{
}

bool AreaRank::initialise(const std::string& name)
{
	AreaBase::initialise(name);
	m_population.Initialise(name);
	m_population.LoadConstants();
	if(!rank())
	{
		m_population.LoadPopulation();
		
		distr_vector=new unsigned long[nodes()];
		for(int i=0;i<5;i++)
		{
			unsigned long id=m_population.FetchRnd();
			if(id!=NONE)
				m_infect.push_back(id);
		}
		printf("Epoch, General, Immune, Dead, Sick, NewCases\n");
	}
	return true;
}

void AreaRank::prepare_distr_vector(int num)
{
	memset(distr_vector,0,sizeof(unsigned long)*nodes());
	for(int i=0,j=0;i<num;i++,j=(j+1)%nodes())
	{
		distr_vector[j]++;	
	}
}

void AreaRank::store_day(unsigned long stats[STATS_SIZE])
{
	unsigned long g,i,d;

	if(rank())
		return;
	m_population.stats(&g,&i,&d);
	printf("%d, %lu, %lu, %lu, %lu, %lu\n",m_epoch,g,i,d,stats[STATS_SICK],stats[STATS_NEW]);
}

void AreaRank::next_epoch()
{
	unsigned long number;
	unsigned long stats[STATS_SIZE];
	std::vector<unsigned long> ids,touched;
	MPI_Status status;

	AreaBase::next_epoch();
	if(!rank())
	{
		prepare_distr_vector(m_infect.size());
		m_population.next_epoch();
	}
	MPI_Scatter(distr_vector,1,MPI_LONG,&number,1,MPI_LONG,0,MPI_COMM_WORLD);
	if(!rank())
	{
		int j=0;
		for(int i=1;i<nodes();i++)
		{
			ids.clear();
			for(int k=0;k<distr_vector[i];k++)
				ids.push_back(m_infect[j++]);
			MPI_Send(ids.data(),ids.size(),MPI_LONG,i,0,MPI_COMM_WORLD);
		}
		ids.clear();
		for(int k=0;k<distr_vector[0];k++)
		{
			ids.push_back(m_infect[j++]);
		}
		
	}
	else
	{
		ids.resize(number);
		MPI_Recv(ids.data(),number,MPI_LONG,0,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
	}
	m_recovered.clear();
	m_dead.clear();
	for(int i=0;i<ids.size();i++)
	{
		SPERSON p(new Person);
		p->id()=ids[i];
		m_sick.push_back(p);
	}
	run();
	memset(stats,0,sizeof(unsigned long)*STATS_SIZE);
	for(PERSONS::iterator it=m_sick.begin();it!=m_sick.end();++it)
	{
		if((*it)->touched())
		{
			touched.push_back((*it)->id());
			touched.push_back((*it)->touched());
		}
	}
	stats[STATS_NEW]=touched.size();
	stats[STATS_RECOVERED]=m_recovered.size();
	stats[STATS_DEAD]=m_dead.size();
	stats[STATS_SICK]=m_sick.size();
	ids.clear();
	if(!rank())
		ids.resize(nodes()*STATS_SIZE);
	MPI_Gather(stats,STATS_SIZE,MPI_LONG,ids.data(),STATS_SIZE,MPI_LONG,0,MPI_COMM_WORLD);
	//reecv  specifics of the epoch
	if(!rank())
	{
		m_infect.clear();
		stats[STATS_NEW]=0;
		for(int i=STATS_SIZE;i<nodes()*STATS_SIZE;i+=STATS_SIZE)
		{
			std::vector<unsigned long> data;
			int slave=i/STATS_SIZE;

			data.resize(ids[i+STATS_NEW]+ids[i+STATS_DEAD]+ids[i+STATS_RECOVERED]);
			//if(data.size())
				MPI_Recv(data.data(),data.size(),MPI_LONG,slave,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
			stats[STATS_SICK]+=ids[i+STATS_SICK];
			for(int j=0;j<ids[i+STATS_NEW];j+=2)
			{
				for(int k=0;k<data[j+1];k++)
				{
					unsigned long l=m_population.FetchNearby(data[j]);
					if(l!=NONE)
					{
						m_infect.push_back(l);
					}
				}
			}
			
			for(int j=0;j<ids[i+STATS_DEAD];j++)
				m_population.dead(data[j+ids[i+STATS_NEW]]);
			for(int j=0;j<ids[i+STATS_RECOVERED];j++)
				m_population.recovered(data[j+ids[i+STATS_NEW]]+ids[i+STATS_DEAD]);
		}
		for(int j=0;j<touched.size();j+=2)
		{
			for(int k=0;k<touched[j+1];k++)
			{
				unsigned long l=m_population.FetchNearby(touched[j]);
				if(l!=NONE)
					m_infect.push_back(l);
			}
		}
		stats[STATS_NEW]=m_infect.size();
		stats[STATS_DEAD]=m_dead.size();
		for(PERSONS::iterator it=m_dead.begin();it!=m_dead.end();++it)
			m_population.dead((*it)->id());
		stats[STATS_RECOVERED]=m_recovered.size();
		for(PERSONS::iterator it=m_recovered.begin();it!=m_recovered.end();++it)
			m_population.recovered((*it)->id());
        	store_day(stats);	
	}
	else
	{
		std::vector<unsigned long> data;

		for(int i=0;i<touched.size();i++)
			data.push_back(touched[i]);
		for(PERSONS::iterator it=m_dead.begin();it!=m_dead.end();++it)
			data.push_back((*it)->id());
		for(PERSONS::iterator it=m_recovered.begin();it!=m_recovered.end();++it)
			data.push_back((*it)->id());
		//if(data.size())
			MPI_Send(data.data(),data.size(),MPI_LONG,0,0,MPI_COMM_WORLD);
	}
}



