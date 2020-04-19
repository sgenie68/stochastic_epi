#ifndef POPULATION_H
#define POPULATION_H
#include <vector>
#include <list>
#include <string>
#include <libconfig.h++>

#define NONE 0

struct Object
{
	unsigned long flags; //various flags
	int index; //index in the compartments
	double coordLat; //coordinates of the base
	double coordLong;	
};

struct Compartment
{
	std::string name;
	double centreLat;	//Centre of the compartment coordinates
	double centreLong;
	double radius;		//compartment radius
	unsigned int amount;	//Number of people in the compartment
};
typedef std::vector<Compartment> COMP_LIST;


struct State
{
	std::string name;
	double distance;
	int contacts;
	std::vector<std::pair<int,double> > days;
};
typedef std::vector<State> STATE_LIST;


struct Parameters
{
	double immunity;
	double death;
	double difficult;		
	double stages[4];
};


//???We also need a matrix of compartment connectivity to emulate signals passing

class Population
{
	public:
		Population();
		~Population();
		void Initialise(const std::string& configname);
		void CreatePopulation(COMP_LIST& compartments); //???we need to pass connectvity matrix as well
		bool LoadConstants();
		bool LoadPopulation();
		bool LoadStates();

		void next_epoch();
		unsigned long FetchNearby(unsigned long id);
		unsigned long FetchRnd(); //get random object from a compartment

		void dead(unsigned long id);
		void recovered(unsigned long id);

		void stats(unsigned long *general,unsigned long *immune, unsigned long *dead);

		//conststs
		int get_people_met() const { return m_peoplemet; }

		int state() { return m_current_state; }
		double distance() const { return m_states[m_current_state].distance; }
		double contacts() const { return m_states[m_current_state].contacts; }
		double immunity() const { return m_parameters.immunity; }
		double death() const { return m_parameters.death; }
		double difficult() const { return m_parameters.difficult; }
		double stage(int i) const { return m_parameters.stages[i]; }

	protected:
		typedef std::list<Object *> POPULATION;

		Object *fetch_object_nearby(double coordLat,double coordLong,double max_distance);
		Object *fetch_object_rnd(int comp_index); //get random object from a compartment

		int m_peoplemet;
		int m_epoch;
		COMP_LIST m_comp_list;
		STATE_LIST m_states;
		Parameters m_parameters;
		int m_current_state;
		POPULATION m_general;
		POPULATION m_immune;
		POPULATION m_dead;
		std::string m_configname;
		libconfig::Config m_config;
		const libconfig::Setting *m_root;
};

#endif

