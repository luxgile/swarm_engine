#pragma once
#include <vector>
#include <algorithm>


template <typename T>
class MemPool {
	std::vector<T*> items = std::vector<T*>();

public:
	T* create() {
		T* item = new T();
		items.push_back(item);
		return item;
	}

	template <typename G>
	G* create() {
		G* item = new G();
		items.push_back((T*)item);
		return item;
	}

	void destroy(T* item) {
		auto it = std::remove(items.begin(), items.end(), item);
		if (it != items.end()) {
			delete item;
			items.erase(it, items.end());
		}
	}

	size_t size() const { return items.size(); }
	inline T* operator[](int idx) { return items[idx]; }

	typedef typename std::vector<T*>::iterator iter;
	typedef typename std::vector<T*>::const_iterator citer;
	iter begin() { return items.begin(); }
	citer begin() const { return items.begin(); }
	iter end() { return items.end(); }
	citer end() const { return items.end(); }
};
