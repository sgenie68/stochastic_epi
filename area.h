#ifndef AREA_H
#define AREA_H
#include <list>
#include <memory>
#include <vector>
#include "population.h"


//States of the person
#define STATE_EXPOSED		0
#define STATE_PRESYMPTOMATIC	1
#define STATE_INFECTIOUS	2
#define STATE_DIFFICULT		3
#define STATE_SIZE		(STATE_DIFFICULT+1)

class Person
{
	public:
		Person();
		~Person();
		
		//move to nexy epoch
		//retuerns true if the person still alive
		//returns false if the person dies
		bool next_epoch();
	//statistics
		int age() const { return m_age; }
		int duration() const { return m_duration; };	//duration of sickness, days
		double death(); //probability of death

		int contacts(); //number of contacts a day
		int contaminate(int contacts); //number of people contaminated
		void quarantine(bool on) { m_quarantine=on; }
		bool quarantine() const { return m_quarantine; }

		int& touched() { return m_contaminated; }  //should be reset before each epoch
		unsigned long& id() { return m_id; }

		int state() const { return m_state; }

	protected:
		void initialise();

		int m_duration;
		int m_age;
		bool m_quarantine;
		int m_contaminated;
		unsigned long m_id;
		int m_state;
		int m_days[STATE_SIZE]; //Number of days for eeach state
};		

typedef std::shared_ptr<Person> SPERSON;

class AreaBase
{
	public:
		AreaBase();
		virtual ~AreaBase();
		virtual void next_epoch();
		int epoch() const { return m_epoch; }
		virtual bool initialise(const std::string& name);
	protected:
		virtual void run();
		typedef std::list<SPERSON> PERSONS;
		PERSONS m_sick;
		PERSONS m_recovered;
		PERSONS	m_dead;
		int m_epoch;
		bool m_quarantine;
};


class AreaRank:public AreaBase
{
	public:
		AreaRank();
		virtual ~AreaRank();
		
		void add_object(unsigned long id);
		void clear_state();
		virtual void next_epoch();
		int rank() const { return m_rank; }
		int nodes() const { return m_nodesnum; }
		virtual bool initialise(const std::string& name);

	protected:
		Population m_population;
		std::vector<unsigned long> m_infect;
		void prepare_distr_vector(int num);
		void store_day(unsigned long stats[3]);
		unsigned long *distr_vector;
		int m_nodesnum;
		int m_rank;
};


#endif

