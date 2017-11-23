#include <iostream>
using namespace std;

template <typename DataType>
class Test {
public:
  Test(DataType* data) : _data(data) {}

  const DataType* data() const {
	return _data;
  }

private:
  DataType* _data;
};

typedef Test<char> char_test;
typedef Test<const char> const_char_test;

int main(int argc, char **argv) {
  const char* data = "Hello";

  const_char_test ctest(data);
  cout << ctest.data() << endl;

  char* data2 = "Hello";

  char_test test(data2);
  cout << test.data() << endl;
}
