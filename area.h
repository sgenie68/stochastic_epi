#ifndef AREA_H
#define AREA_H
#include <list>
#include <memory>
#include <vector>
#include "population.h"


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

	protected:
		void initialise();

		int m_duration;
		int m_age;
		bool m_quarantine;
		int m_contaminated;
		unsigned long m_id;
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

