#include <iostream>
#include "population.h"
#include <math.h>
#include "utils.h"


using namespace std;
using namespace libconfig;

Population::Population()
{
}

Population::~Population()
{
	for(POPULATION::iterator it=m_general.begin();it!=m_general.end();++it)
		delete *it;	
}

void Population::Initialise(const std::string& configname)
{
	m_configname=configname;
	m_epoch=0;
	try
	{
		m_config.readFile(configname.c_str());
	}
	catch(const ParseException &pex)
  	{
    		std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine() << " - " << pex.getError() << std::endl;
		exit(1);
  	}
	catch(...)
	{
		cerr << "Error reading config file " << configname << endl;
		exit(1);
	}
 	m_root = &m_config.getRoot();
}

void Population::CreatePopulation(COMP_LIST& compartments)
{
	m_comp_list.clear();

	for(COMP_LIST::iterator it=compartments.begin();it!=compartments.end();++it)
		m_comp_list.push_back(*it);
	//create 
	for(int index=0;index<m_comp_list.size();index++)
	{
		for(int i=0;i<m_comp_list[index].amount;i++)
		{
			Object *p=new Object;
		 	p->flags=0;
			p->index=index;
			uniform(&p->coordLat,1,m_comp_list[index].centreLat,meters2rad(m_comp_list[index].radius));
			uniform(&p->coordLong,1,m_comp_list[index].centreLong,meters2rad(m_comp_list[index].radius));
			m_general.push_back(p);
		}
	}
}

bool Population::LoadPopulation()
{
	COMP_LIST lst;

	try
  	{
    		const Setting &comps = (*m_root)["components"];
    		int count = comps.getLength();
		for(int i=0;i<count;i++)
		{
			const Setting& c=comps[i];
			Compartment cmp;

			c.lookupValue("name",cmp.name);
			c.lookupValue("coordlat",cmp.centreLat);
			c.lookupValue("coordlong",cmp.centreLong);
			c.lookupValue("radius",cmp.radius);
			c.lookupValue("population",cmp.amount);
			lst.push_back(cmp);
		}
		CreatePopulation(lst);
		return true;	
	}
	catch(...)
	{
	}
	return false;
}

bool Population::LoadConstants()
{
	double d;
	try
  	{
		m_root->lookupValue("distance_isolation",d);
		return true;	
	}
	catch(...)
	{
	}

	return false;
}


Object *Population::fetch_object_nearby(double coordLat,double coordLong,double max_distance)
{
	Object *p=NULL;

	for(POPULATION::iterator it=m_general.begin();it!=m_general.end();++it)
	{
		if(::distance((*it)->coordLat,(*it)->coordLong,coordLat,coordLong)<=max_distance)
		{
			p=*it;
			m_general.erase(it);
			break;
		}
	}
	return p;
}

Object *Population::fetch_object_rnd(int comp_index)
{
	double val;
	int i=0;
	Object *p=NULL;
	POPULATION::iterator it=m_general.begin();
	
	if(!m_general.size())
		return NULL;
	uniform(&val,1,0,m_general.size());
	for(i=0;i<(int)floor(val);i++)
		++it;
	for(;;i=(i+1)%m_general.size())
	{
		if(!i)
			it=m_general.begin();
		if((*it)->index==comp_index)
		{
			p=*it;
			m_general.erase(it);
			break;
		}
		++it;
	}
	return p;
}


unsigned long Population::FetchNearby(unsigned long id)
{
	double dist=100;
	Object *p=(Object *)id;
	if(!p)
		return NONE;
	Object *p1=fetch_object_nearby(p->coordLat,p->coordLong,dist);
	return (p?(unsigned long)p1:NONE);
}

unsigned long Population::FetchRnd()
{
	//fetch the compartment first
	std::vector<double> weights;
	for(COMP_LIST::iterator it=m_comp_list.begin();it!=m_comp_list.end();++it)
	{
		weights.push_back((double)it->amount);
	}
	int i=(m_comp_list.size()>1?weighted_choice(weights.data(),weights.size()):0);
	Object *p=fetch_object_rnd(i);
	return (p?(unsigned long)p:NONE);
}

void Population::dead(unsigned long v)
{
	m_dead.push_back((Object *)v);
}

void Population::recovered(unsigned long v)
{
	m_immune.push_back((Object *)v);
}

void Population::stats(unsigned long *general,unsigned long *immune, unsigned long *dead)
{
	*general=m_general.size();
	*immune=m_immune.size();
	*dead=m_dead.size();
}

void Population::next_epoch()
{
	m_epoch++;
}

