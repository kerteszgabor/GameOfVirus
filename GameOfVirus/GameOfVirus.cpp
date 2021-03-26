#include <iostream>
#include <chrono>
#include <vector>
#include <cstdlib>
#include <time.h>
#include <algorithm> // for std::count_if
#include "config.h"
#include "GameModel.h"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <chrono>

using Grid = std::vector<std::vector<std::shared_ptr<Cell>>>;

void Init(Grid& grid)
{
	int peopleToDistribute = params.nPersons;
	for (auto& r : grid)
	{
		for (auto& cell : r)
		{
			cell = std::make_shared<Cell>();
			GameModel::NotQuarantinedCells.push_back(cell);
		}
	}

	while (peopleToDistribute > 0)
	{
		size_t currX(0);
		for (auto& r : grid)
		{
			size_t currY(0);
			for (auto& cell : r)
			{
				auto newPerson = std::make_shared<Person>(index(currX, currY));
				cell->_people.push_back(newPerson);

				if (newPerson->_status == Status::infected)
				{
					GameModel::InfectedPeople.push_back(newPerson);
				}
				else
				{
					GameModel::NormalPeople.push_back(newPerson);
				}
				if (--peopleToDistribute == 0)
				{
					return;
				}
				++currY;
			}
			++currX;
		}
	}
}


std::vector<std::shared_ptr<Person>> GetInfectedInCell(const Cell& cell)
{
	std::vector<std::shared_ptr<Person>> infected;
	std::copy_if(cell._people.begin(), cell._people.end(), std::back_inserter(infected),[&](std::shared_ptr<Person> t) { return  t->_status == Status::infected; });
	return infected;
}

// Tests a chosen cell for infections. If found infected, sets the person's status to confirmed. Returns the number of infected people.
size_t TestACell(Cell& cell)
{
	auto numInfected = std::count_if(cell._people.begin(), cell._people.end(), [](std::shared_ptr<Person> p) 
		{ 
			if (p->_status == Status::infected)
			{
				p->_status += Status::confirmed;
				ConfirmInfection(p);
				return true;
			}
			return false;
		});
	return numInfected;
}

// there's a huge gain in perfomance using NotQuarantined when there are only one or two cells remaining (searching for their index was horrible)
std::vector<index> GetTestLocations(Grid& grid)
{
	std::vector<index> locations;
	if (params.nSamplePlace > GameModel::NotQuarantinedCells.size())
	{
		for (auto e : GameModel::NotQuarantinedCells)
		{
			if (e->_people.size())
			{
				locations.push_back(index(e->_people[0]->_location.x, e->_people[0]->_location.y));
			}
		}
	}
	else
	{
		size_t numberOfEmptyCells = 0;
		do
		{
			auto& cell = GameModel::NotQuarantinedCells[(rand() % GameModel::NotQuarantinedCells.size())];
			if (cell->_people.size())
			{
				index idx(cell->_people[0]->_location.x, cell->_people[0]->_location.y);
				auto found = std::find(locations.begin(), locations.end(), idx);

				if (found == locations.end())
				{
					locations.push_back(idx);
				}
			}
			else
			{
				++numberOfEmptyCells;
			}
		} while (locations.size() < params.nSamplePlace && params.nSamplePlace - locations.size() <= pow(params.nSamplePlace,2) - numberOfEmptyCells);
	}
	return locations;
}

void SimulateAuthoritiesTesting(Grid& grid)
{
	std::vector<index> testLocations(GetTestLocations(grid));
	for (auto e : testLocations)
	{
		auto cellIT = std::find_if(GameModel::NotQuarantinedCells.begin(), GameModel::NotQuarantinedCells.end(),[e](std::shared_ptr<Cell>& c)
			{
				return c->_people[0]->_location.x == e.x && c->_people[0]->_location.y == e.y;
			}
		);
		auto& cell = **cellIT;
		if (TestACell(cell))
		{
			cell._isQuarantined = true;
			GameModel::QuarantinedCells.push_back(std::move(GetElementFrom(cell, GameModel::NotQuarantinedCells)));

			ClearUpEmpty(GameModel::NotQuarantinedCells);
		}
	}
}

void RandomPersonContactsVirus(Cell& cell)
{
	size_t idx;
	int foundInfected(-1);
	do
	{
		idx = (rand() % cell._people.size());
		++foundInfected;
	} while (cell._people[idx]->_status != Status::normal && foundInfected < cell._people.size());

	if (cell._people.size() != foundInfected)
	{
		double chanceOfInfection = ((double)rand() / RAND_MAX);
		if (chanceOfInfection <= params.pInfects)
		{
			cell._people[idx]->_status -= Status::normal;
			cell._people[idx]->_status += Status::infected;

			GameModel::InfectedPeople.push_back(std::move(GetElementFrom(cell._people[idx], GameModel::NormalPeople)));
			auto it = std::find_if(GameModel::NormalPeople.begin(), GameModel::NormalPeople.end(), [](std::shared_ptr<Person>& p) 
				{ 
					if (p == nullptr)
					{
						return true;
					}
				});
			GameModel::NormalPeople.erase(it);
		}
	}
}

void SimulateVirusSpreading(Grid& grid)
{
	for (auto& r : grid)
	{
		for (auto& cell : r)
		{
			std::vector<std::shared_ptr<Person>> infectedInCell = GetInfectedInCell(*cell);
			for (auto inf : infectedInCell)
			{
				RandomPersonContactsVirus(*cell);
			}
		}
	}
}


void HealPerson(std::vector<std::shared_ptr<Person>>::const_iterator& person)
{
	(*person)->_status -= Status::infected;
	(*person)->_status += Status::recovered;

	GameModel::RecoveredPeople.push_back(std::move(GetElementFrom(*person, GameModel::InfectedPeople)));
	person = GameModel::InfectedPeople.erase(person); // remove not the element, not the ptr, but the iterator to it
}

void CalculateDeathAndHeals(Grid& grid)
{
	srand((unsigned)time(NULL));
	for (auto person = GameModel::InfectedPeople.begin(); person != GameModel::InfectedPeople.end();)
	{
		double chanceOfHealing = ((double)rand() / RAND_MAX);

		if ((*person)->_daysOfInfection > params.maxHealDay)
		{
			HealPerson(person);
		}
		else if (++((*person)->_daysOfInfection) >= params.minHealDay
				&& ((*person))->_daysOfInfection <= params.maxHealDay
				&& chanceOfHealing <= params.pHeal)
		{
			HealPerson(person);
		}
		else
		{
			++person;
		}
	}

	srand((unsigned)time(NULL));
	for (auto person = GameModel::InfectedPeople.begin(); person != GameModel::InfectedPeople.end();)
	{
		double chanceOfDying = ((double)rand() / RAND_MAX);

		if (((*person)->_daysOfInfection) >= params.minDiseaseDay
			&& ((*person))->_daysOfInfection <= params.maxDiseaseDay
			&& chanceOfDying <= params.pDisease)
		{
			(*person)->_status -= Status::infected;
			(*person)->_status += Status::confirmed;
			(*person)->_status += Status::dead;

			ConfirmInfection(*person);
			Cell& cellOfPerson = *grid[(*person)->_location.x][(*person)->_location.y];
			if (!cellOfPerson._isQuarantined)
			{
				cellOfPerson._isQuarantined = true;
				GameModel::QuarantinedCells.push_back(std::move(GetElementFrom(cellOfPerson, GameModel::NotQuarantinedCells)));
				ClearUpEmpty(GameModel::NotQuarantinedCells);
			}
			GameModel::DeadPeople.push_back(std::move(GetElementFrom(*person, GameModel::InfectedPeople)));
			person = GameModel::InfectedPeople.erase(person);
		}
		else
		{
			++person;
		}
	}
}

bool ValidateStep(index& pos, index& step)
{
	if (!(step.x == 0 && step.y == 0))
	{
		short newX = pos.x + step.x;
		short newY = pos.y + step.y;
		if (newX >= 0 && newX <= params.size - 1 && newY >= 0 && newY <= params.size - 1)
		{
			step.x = newX;
			step.y = newY;
			return true;
		}
	}

	return false;
}

bool MovePerson(Grid& grid, std::shared_ptr<Person>& person)
{
	index nextStep;
	double chanceOfMoving = ((double)rand() / RAND_MAX);
	size_t numberOfTries = 0;
	if (chanceOfMoving < params.pMove)
	{
		do
		{
			nextStep.x = (rand() % 3) - 1;
			nextStep.y = (rand() % 3) - 1;
			++numberOfTries;
		} while (!(numberOfTries == 50 
			|| ValidateStep(person->_location, nextStep) 
			&& grid[person->_location.x][person->_location.y]->_isQuarantined
			&& grid[nextStep.x][nextStep.y]->_isQuarantined));

		if (numberOfTries != 50)
		{
			person->hasMoved = true;
			person->_location = nextStep;
			GameModel::MovedPeople.push_back(person);
			grid[nextStep.x][nextStep.y]->_people.push_back(std::move(person));
			return true;
		}
	}

	return false;
}

void SimulatePeopleMoving(Grid& grid)
{
	for (auto& r : grid)
	{
		for (auto& cell : r)
		{
			if (!cell->_isQuarantined)
			{
				for (auto personIt = cell->_people.begin(); personIt != cell->_people.end();)
				{
					if (!(*personIt)->hasMoved && MovePerson(grid, *personIt))
					{
						personIt = cell->_people.erase(personIt);
					}
					else
					{
						++personIt;
					}
				}
			}
		}
	}

	ResetMovedPeople();
}

void SimulateOneDay(Grid& grid)
{
	SimulateVirusSpreading(grid);
	SimulateAuthoritiesTesting(grid);
	CalculateDeathAndHeals(grid);
	HandleQuarantinedCells();
	SimulatePeopleMoving(grid);
}

size_t CalculatePeopleInQuarantine()
{
	size_t sum = 0;
	for (auto q : GameModel::QuarantinedCells)
	{
		sum += q->_people.size();
	}

	return sum;
}

void DisplayResult(size_t days, std::chrono::steady_clock::time_point& start)
{
	std::cout << "Day " << days << std::endl;
	std::cout << std::endl;
	std::cout << "Normal people:" << GameModel::NormalPeople.size() << std::endl;
	std::cout << "Infected people:" << GameModel::InfectedPeople.size() << std::endl;
	std::cout << "Confirmed people:" << GameModel::ConfirmedPeople.size() << std::endl;
	std::cout << "Recovered people:" << GameModel::RecoveredPeople.size() << std::endl;
	std::cout << "Dead people:" << GameModel::DeadPeople.size() << std::endl;
	std::cout << "Quarantined people:" << CalculatePeopleInQuarantine() << std::endl;
	auto end = std::chrono::steady_clock::now();
	auto diff = end - start;
	std::cout << "Elapsed time: " << std::chrono::duration <double, std::milli>(diff).count() / 1000 << "s" << std::endl;
	std::cout << std::endl;
	std::cout << "######################################## " << std::endl;
	std::cout << std::endl;
}

int main()
{
	auto start = std::chrono::steady_clock::now();
	srand((unsigned)time(NULL));
	Grid grid(params.size, std::vector<std::shared_ptr<Cell>>(params.size));
	Init(grid);
	size_t days = 0;

	while (GameModel::InfectedPeople.size() != 0)
	{
		SimulateOneDay(grid);
		DisplayResult(++days, start);
	}
}