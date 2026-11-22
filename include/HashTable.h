#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <string>
#include <vector>
#include <deque>
#include <cmath>
#include <cstdint>
#include <iostream>

template <typename T>
class HashTable
{
public:
	struct Record
	{
		std::string key;
		T value;
		int next;
	};

	HashTable(int initialSize = 13) :
		buckets(initialSize, -1)
	{}

	T* insert(const std::string& key, const T& value)
	{
		if (findIndex(key) != -1)
			return nullptr;

		if (records.size() + 1 > MAX_LOAD * buckets.size())
			grow();

		int index = static_cast<int>(records.size());
		int h = hash(key);

		records.push_back(Record{ key, value, buckets[h] });
		buckets[h] = index;

		return &records.back().value;
	}

	T* find(const std::string& key)
	{
		int i = findIndex(key);
		return i == -1 ? nullptr : &records[i].value;
	}

	const T* find(const std::string& key) const
	{
		int i = findIndex(key);
		return i == -1 ? nullptr : &records[i].value;
	}

	bool contains(const std::string& key) const
	{
		return findIndex(key) != -1;
	}

	int size() const
	{
		return static_cast<int>(records.size());
	}

	int bucketCount() const
	{
		return static_cast<int>(buckets.size());
	}

	const std::deque<Record>& getRecords() const
	{
		return records;
	}

	std::deque<Record>& getRecords()
	{
		return records;
	}

	void printBuckets(std::ostream& os = std::cout) const
	{
		os << "HashTable: " << size() << " registros, " << bucketCount() << " buckets\n";

		for (int b = 0; b < bucketCount(); b++)
		{
			os << "  [" << b << "]";
			for (int i = buckets[b]; i != -1; i = records[i].next)
				os << " -> " << records[i].key;
			os << "\n";
		}
	}

private:
	static constexpr int MAX_LOAD = 2;

	std::vector<int> buckets;
	std::deque<Record> records;

	static std::uint32_t stringToKey(const std::string& s)
	{
		std::uint32_t k = 0;
		for (unsigned char c : s)
			k = k * 31 + c;
		return k;
	}

	int hash(const std::string& key) const
	{
		const double C = 0.6180339887;

		double product = stringToKey(key) * C;
		double frac = product - std::floor(product);

		return static_cast<int>(buckets.size() * frac);
	}

	int findIndex(const std::string& key) const
	{
		int h = hash(key);

		for (int i = buckets[h]; i != -1; i = records[i].next)
			if (records[i].key == key)
				return i;

		return -1;
	}

	void grow()
	{
		int newSize = bucketCount() * 2 + 1;
		buckets.assign(newSize, -1);

		for (int i = 0; i < size(); i++)
		{
			int h = hash(records[i].key);
			records[i].next = buckets[h];
			buckets[h] = i;
		}
	}
};

#endif