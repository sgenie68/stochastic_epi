#include <iostream>
#include "population.h"
#include <math.h>
#include "utils.h"


using namespace std;
using namespace libconfig;

Population::Population():m_current_state(0)
{
}

Population::~Population()
{
	//for(POPULATION::iterator it=m_general.begin();it!=m_general.end();++it)
	//	delete *it;	
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
			double mr=meters2rad(m_comp_list[index].radius);
			uniform(&p->coordLat,1,m_comp_list[index].centreLat-mr,m_comp_list[index].centreLat+mr);
			uniform(&p->coordLong,1,m_comp_list[index].centreLong-mr,m_comp_list[index].centreLong+mr);
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
			cmp.centreLat=deg2rad(cmp.centreLat);
			cmp.centreLong=deg2rad(cmp.centreLong);
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
    		const Setting &params = (*m_root)["parameters"];
		params.lookupValue("immunity",m_parameters.immunity);
		params.lookupValue("death",m_parameters.death);
		params.lookupValue("difficult",m_parameters.difficult);
    		const Setting &cs = params.lookup("contagion");
		for(int i=0;i<4;i++)
			m_parameters.stages[i]=(double)cs[i];
		return true;	
	}
	catch(...)
	{
	}

	return false;
}

bool Population::LoadStates()
{
	try
	{
    		const Setting &states = (*m_root)["states"];
    		int count = states.getLength();
		for(int i=0;i<count;i++)
		{
			const Setting& s=states[i];
			State st;

			s.lookupValue("name",st.name);
			s.lookupValue("distance",st.distance);
			s.lookupValue("contacts",st.contacts);
			try
			{
    				const Setting &days = s["days"];
    				int count1 = days.getLength();
				for(int j=0;j<count1;j++)
				{
					int d;
					double prob;
					const Setting& ddd=days[j];
					ddd.lookupValue("days",d);
					ddd.lookupValue("probability",prob);
					st.days.push_back(make_pair(d,prob));
				}
			}
			catch(...)
			{
			}
			m_states.push_back(st);
		}
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
	double dist=distance();
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
	if(trigger(immunity()))
		m_immune.push_back((Object *)v);
	else
		m_general.push_back((Object *)v);
}

void Population::stats(unsigned long *general,unsigned long *immune, unsigned long *dead)
{
	*general=m_general.size();
	*immune=m_immune.size();
	*dead=m_dead.size();
}


void Population::next_epoch()
{
	bool switched=false;

	m_epoch++;
	//Load proper state
	for(int i=0;i<m_states.size();i++)
	{
		for(int j=0;j<m_states[i].days.size();j++)
		{
			if(m_epoch==m_states[i].days[j].first && trigger(m_states[i].days[j].second))
			{
				m_current_state=i;
				switched=true;
				break;
			}
		}
		if(switched)
			break;
	}
}

