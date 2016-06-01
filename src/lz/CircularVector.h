#pragma once

#include <vector>
#include <cassert>

template <typename T>
class CircularVector {
private:
	std::vector<T> m_vec;
	int m_start;
	int m_size;

public:

	explicit CircularVector(int size) : m_vec(size), m_start{ 0 }, m_size{ size } {}
	CircularVector(int size, const T& val) : m_vec(size, val), m_start{ 0 }, m_size{ size } {}

	T& operator[](int index) {
		assert(index >= 0 && index < m_size);
		return m_vec[(m_start + index) % m_size];
	}

	const T& operator[](int index) const {
		assert(index >= 0 && index < m_size);
		return m_vec[(m_start + index) % m_size];
	}

	T& Get(int index) {
		return m_vec[index % m_size];
	}

	const T& Get(int index) const {
		return m_vec[index % m_size];
	}

	void Shift(int amount) {
		m_start = (m_start + amount) % m_size;
	}

	int ShiftIndex(int index) const {
		return (m_start + index) % m_size;
	}

	int Size() const {
		return m_size;
	}
};