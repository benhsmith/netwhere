/*
 * Copyright (c) 2017, Ben Smith
 * All rights reserved.
 *
 */

#ifndef __AUTODYNAMICMAP_H__
#define __AUTODYNAMICMAP_H__

#include <unordered_map>

#include <iostream>

/**
 * @brief factory and storage for a set of objects
 *
 * A set that allocates and deletes the objects it contains, storing them as pointers
 * and providing access to them via a Key type.
 * The Object class must have a constructor that takes Key as the argument.
 *
 * ObjectSet is intended for when you have a collection of objects that you want ti
 * to store and lookup using a key and you want to avoid copying the objects.
 *
 */
template <typename Key, typename Object>
class ObjectSet {
private:
  typedef std::unordered_map<Key, Object*> MapType;

  MapType _map;

public:
  /**
   * @brief translates internal map's iterator to return Object* instead of pair
   *
   */
  template <typename IteratorType>
  class Iterator {
  public:
	Iterator(IteratorType it) : _iterator(it) {}
	Iterator(const Iterator& it) : _iterator(it._iterator) {}

	Object* operator * () {
	  return _iterator->second;
	}

	const Object* operator * () const {
	  return _iterator->second;
	}

	Object* operator -> () {
	  return _iterator->second;
	}

	const Object* operator -> () const {
	  return _iterator->second;
	}

	bool operator == (const Iterator& it) const {
	  return _iterator == it._iterator;
	}

	bool operator != (const Iterator& it) const {
	  return _iterator != it._iterator;
	}

	void operator ++ () {
	  _iterator++;
	}

  private:
	IteratorType _iterator;
  };

  typedef Iterator<typename MapType::iterator> iterator;
  typedef Iterator<typename MapType::const_iterator> const_iterator;

  ~ObjectSet() {
	for (auto && item : _map) {
	  delete item.second;
	}
  }

  iterator begin() {
	return iterator(_map.begin());
  }

  const_iterator begin() const {
	return const_iterator(_map.begin());
  }

  iterator end() {
	return iterator(_map.end());
  }

  const_iterator end() const {
	return const_iterator(_map.end());
  }

  std::pair<iterator, bool> insert(const Key& key) {
    auto item = _map.find(key);

    if (item != _map.end()) {
	  return std::make_pair(iterator(item), false);
	}

	item = _map.insert(
	  std::make_pair(key, new Object(key))
	).first;

	return std::pair<iterator, bool>(iterator(item), true);
  }

  iterator find(const Key& key) {
	return iterator(_map.find(key));
  }

  const_iterator find(const Key& key) const {
	return const_iterator(_map.find(key));
  }

  void erase(const Key& key) {
	auto it = _map.find(key);
	delete it->second;
	_map.erase(it);
  }

  size_t size() const {
	return _map.size();
  }
};

#endif
