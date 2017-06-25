#ifndef __AUTODYNAMICMAP_H__
#define __AUTODYNAMICMAP_H__

#include <unordered_map>

template <typename Iterator>
class ValueIterator : public Iterator {
public:
  ValueIterator(const Iterator& it) : Iterator(it) {}

  const typename Iterator::value_type::second_type& operator * () { return (Iterator::operator *()).second; }
  //  typename Iterator::value_type::second_type& operator * () const { return (*(*this)).second; }
};

template <typename Iterator>
ValueIterator<const Iterator> value_iterator(const Iterator& it) { return ValueIterator<const Iterator>(it); }

template <typename Value>
class AutoDerefPointer : public std::unique_ptr<Value> {
public:
  AutoDerefPointer(Value * obj) : std::unique_ptr<Value>(obj) {}

  operator Value& () { return *(this->get()); }
  operator const Value& () const { return *(this->get()); }
};

template <typename Key, typename Value>
class AutoDynamicMap : public std::unordered_map<Key, AutoDerefPointer<Value>> {
public:

  typedef std::unordered_map<Key, std::unique_ptr<Value>> map_type;

  Value& operator [] (const Key& key) {
    auto item = this->find(key);
    if (item == this->end()) {
	  item = this->insert(
        std::make_pair(key, std::move(AutoDerefPointer<Value>(new Value(key))))
	  ).first;
	}
    return *((*item).second.get());
  }

  bool has (const Key& key) {
	return this->find(key) != this->end();
  }
};

#endif
