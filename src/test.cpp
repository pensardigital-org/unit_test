// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)


#include "test.hpp"
#include "../../cpplib/src/string_def.hpp"
#include "../../cpplib/src/constant.hpp"

namespace pensar_digital
{
    namespace unit_test
    {

        Generator<CompositeTest> CompositeTest::generator = Generator<CompositeTest> ();
        Generator<Test> Test::generator = Generator<Test> ();

        CompositeTest& all_tests ()
        {
            static CompositeTest* all = new CompositeTest(W("All tests"));
            return *all;
        }
    }
}
