#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE netwhere_test
#include <boost/test/unit_test.hpp>

#include "auto_dynamic_map.hpp"

using namespace std;

class TestValue {
public:
  TestValue(int key) : _key(key) { instances++; }
  ~TestValue() { instances--; }

  int key() { return _key; }

  static void reset() { instances = 0; }
  static int get_instances() { return instances; }
  
private:
  static int instances;
  int _key;
};

int TestValue::instances = 0;


BOOST_AUTO_TEST_CASE( unique_ptr_create ) {
  TestValue::reset();

  unordered_map<int, unique_ptr<TestValue>> map;

  {
 	auto& value = map[1] = unique_ptr<TestValue>(new TestValue(1));

	map.insert( make_pair(2, move(unique_ptr<TestValue>(new TestValue(2)))) );
	
	BOOST_CHECK_EQUAL(value->key(), 1);
    BOOST_CHECK_EQUAL(TestValue::get_instances(), 2);
  }

  BOOST_CHECK_EQUAL(TestValue::get_instances(), 2);

  map.clear();

  BOOST_CHECK_EQUAL(TestValue::get_instances(), 0);
}

BOOST_AUTO_TEST_CASE( auto_dynamic_map_create ) {
  AutoDynamicMap<int, TestValue> map;
  
  TestValue& value1 = map[1];
  TestValue& value2 = map[1];

  BOOST_CHECK_EQUAL(&value1, &value2);
}

BOOST_AUTO_TEST_CASE( auto_dynamic_map_deletes ) {
  TestValue::reset();
  {
    AutoDynamicMap<int, TestValue> map;

    map[1];
  }

  BOOST_CHECK_EQUAL(TestValue::get_instances(), 0);

  TestValue::reset();
  {
    AutoDynamicMap<int, TestValue> map;

    map[1];
    map[2];
    map[3];
    map[4];

    map[1];
  }

  BOOST_CHECK_EQUAL(TestValue::get_instances(), 0);

}

