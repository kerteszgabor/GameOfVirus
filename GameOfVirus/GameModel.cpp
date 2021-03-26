#include "GameModel.h"

Person::Person(index spawnLocation) : _id(idCounter++), _daysOfInfection(0), _location(spawnLocation), hasMoved(false)
{
	double decideInfection = ((double)rand() / (RAND_MAX));
	if (decideInfection <= params.pInfected)
	{
		_status += Status::infected;
	}
	else
	{
		_status += Status::normal;
	}
}

size_t Person::idCounter = 0;

std::vector<Status>& operator+=(std::vector<Status>& lhs, const Status& rhs)
{
	auto it = std::find_if(lhs.begin(), lhs.end(), [rhs](Status t) {return t == rhs; });
	if (it == lhs.end())
	{
		lhs.push_back(rhs);
	}
	return lhs;
}

std::vector<Status>& operator-=(std::vector<Status>& lhs, const Status& rhs)
{
	auto it = std::find_if(lhs.begin(), lhs.end(), [rhs](Status t) {return t == rhs; });
	lhs.erase(it);
	return lhs;
}

// Decides whether a given status is associated with a person's status vector. Basically 'contains' mechanincs.
bool operator==(const std::vector<Status>& lhs, const Status& rhs)
{
	auto contains = std::find_if(lhs.begin(), lhs.end(), [rhs](Status t) { return t == rhs; });
	return contains != lhs.end();
}

bool operator!=(const std::vector<Status>& lhs, const Status& rhs)
{
	return !(lhs == rhs);
}


std::vector<std::shared_ptr<Person>> GameModel::NormalPeople;
std::vector<std::shared_ptr<Person>> GameModel::InfectedPeople;
std::vector<std::shared_ptr<Person>> GameModel::ConfirmedPeople;
std::vector<std::shared_ptr<Person>> GameModel::RecoveredPeople;
std::vector<std::shared_ptr<Person>> GameModel::DeadPeople;

std::vector<std::shared_ptr<Person>> GameModel::MovedPeople;
std::vector<std::shared_ptr<Cell>> GameModel::QuarantinedCells;
std::vector<std::shared_ptr<Cell>> GameModel::NotQuarantinedCells;

std::shared_ptr<Person>& GetElementFrom(const std::shared_ptr<Person>& element, std::vector<std::shared_ptr<Person>>& category)
{
	auto it = std::find(category.begin(), category.end(), element);
	if (it != category.end())
	{
		return *it;
	}
}

std::shared_ptr<Cell>& GetElementFrom(const Cell& element, std::vector<std::shared_ptr<Cell>>& category)
{
	auto it = std::find_if(category.begin(), category.end(), [&](std::shared_ptr<Cell> iterator) 
		{
			return element._people[0]->_id == iterator.get()->_people[0]->_id; 
		});
	if (it != category.end())
	{
		return *it;
	}
}

void ConfirmInfection(std::shared_ptr<Person> person)
{
	if (std::find(GameModel::ConfirmedPeople.begin(), GameModel::ConfirmedPeople.end(), person) == GameModel::ConfirmedPeople.end())
	{
		GameModel::ConfirmedPeople.push_back(person);
	}
}

void ResetMovedPeople()
{
	for (auto& e : GameModel::MovedPeople)
	{
		e->hasMoved = false;
	}
	GameModel::MovedPeople.clear();
}

void HandleQuarantinedCells()
{
	for (auto it = GameModel::QuarantinedCells.begin(); it != GameModel::QuarantinedCells.end();)
	{
		if ((**it)._daysSpentInQuarantine == params.nQuarantineDays)
		{
			(**it)._isQuarantined = false;
			GameModel::NotQuarantinedCells.push_back(std::move(*it));
			it = GameModel::QuarantinedCells.erase(it);
		}
		else
		{
			++(**it++)._daysSpentInQuarantine;
		}
	}
}

Cell::Cell() : _isQuarantined(false), _daysSpentInQuarantine(0) 
{ 
}



