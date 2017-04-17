#ifndef __POINTER_ITERATOR__
#define __POINTER_ITERATOR__

template <typename Iterator, typename Value>
class PointerIteratorWrapper {
public:
  PointerIteratorWrapper(Iterator itr) : itr(itr) {}

  Value& operator * () {
    return *((*itr).second);
  }

  PointerIteratorWrapper& operator ++ () {
    itr++;
    return *this;
  }

  bool operator == (PointerIteratorWrapper other) const {
    return itr == other.itr;
  }

  bool operator != (PointerIteratorWrapper other) const {
    return itr != other.itr;
  }

 private:
  Iterator itr;
};

#endif
