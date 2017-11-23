#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE netwhere_test
#include <boost/test/unit_test.hpp>

#include "object_set.hpp"

using namespace std;

class TestObject {
public:
  TestObject(int key) : _key(key) {
	instances++;
  }
  ~TestObject() {
	instances--;
  }

  int key() { return _key; }

  static void reset() { instances = 0; }
  static int get_instances() { return instances; }

private:
  static int instances;
  int _key;
};

int TestObject::instances = 0;

BOOST_AUTO_TEST_CASE( auto_dynamic_map_insert ) {
  ObjectSet<int, TestObject> map;

  auto value1 = map.insert(1);
  BOOST_CHECK_EQUAL(value1.second, true);

  auto value2 = map.insert(1);
  BOOST_CHECK_EQUAL(value2.second, false);
  BOOST_CHECK_EQUAL(value2.first->key(), 1);

  BOOST_CHECK(value1.first == value2.first);
}

BOOST_AUTO_TEST_CASE( auto_dynamic_map_deletes ) {
  TestObject::reset();
  {
    ObjectSet<int, TestObject> map;

    map.insert(1);
  }

  BOOST_CHECK_EQUAL(TestObject::get_instances(), 0);

  TestObject::reset();
  {
    ObjectSet<int, TestObject> map;

    map.insert(1);
	map.insert(2);
	map.insert(3);
	map.insert(4);

	BOOST_CHECK_EQUAL(TestObject::get_instances(), 4);
  }

  BOOST_CHECK_EQUAL(TestObject::get_instances(), 0);
}
