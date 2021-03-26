#pragma once
#include "config.h"
#include <vector>
#include <memory>


enum class Status
{
	normal, infected, confirmed, recovered, dead
};

struct index
{
	index() = default;
	index(size_t X, size_t Y) : x(X), y(Y) {}
	int x;
	int y;
	bool operator==(const index& rhs)
	{
		return (this->x == rhs.x && this->y == rhs.y);
	}
};

struct Person
{
	static size_t idCounter;
	Person(index spawnLocation);

	size_t _id;
	std::vector<Status> _status;
	bool hasMoved;
	index _location;
	size_t _daysOfInfection;
};


struct Cell: std::enable_shared_from_this<Cell>
{
	Cell();
	std::vector<std::shared_ptr<Person>> _people;
	bool _isQuarantined;
	size_t _daysSpentInQuarantine;	
};


std::vector<Status>& operator+=(std::vector<Status>& lhs, const Status& rhs);
std::vector<Status>& operator-=(std::vector<Status>& lhs, const Status& rhs);
bool operator==(const std::vector<Status>& lhs, const Status& rhs);
bool operator!=(const std::vector<Status>& lhs, const Status& rhs);

class GameModel
{
public:

	static std::vector<std::shared_ptr<Person>> NormalPeople;
	static std::vector<std::shared_ptr<Person>> InfectedPeople;
	static std::vector<std::shared_ptr<Person>> ConfirmedPeople;
	static std::vector<std::shared_ptr<Person>> RecoveredPeople;
	static std::vector<std::shared_ptr<Person>> DeadPeople;

	static std::vector<std::shared_ptr<Person>> MovedPeople;
	static std::vector<std::shared_ptr<Cell>> QuarantinedCells;
	static std::vector<std::shared_ptr<Cell>> NotQuarantinedCells;

};

std::shared_ptr<Person>& GetElementFrom(const std::shared_ptr<Person>& element, std::vector<std::shared_ptr<Person>>& category);

std::shared_ptr<Cell>& GetElementFrom(const Cell& element, std::vector<std::shared_ptr<Cell>>& category);

template <typename T>
bool ClearUpEmpty(std::vector<std::shared_ptr<T>>& v);
void ConfirmInfection(std::shared_ptr<Person> person);
void ResetMovedPeople();
void HandleQuarantinedCells();


template<typename T>
inline bool ClearUpEmpty(std::vector<std::shared_ptr<T>>& v)
{
	auto it = std::find_if(v.begin(), v.end(), [](const std::shared_ptr<T>& p)
		{
			if (p == nullptr)
			{
				return true;
			}
		});
	v.erase(it);
	return true;
}