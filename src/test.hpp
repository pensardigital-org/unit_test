#ifndef TEST_HPP
#define TEST_HPP

#define _WINSOCKAPI_
#include <winsock2.h>

#include "../../cpplib/src/constant.hpp"

#include "../../cpplib/src/generator.hpp"
#include "../../cpplib/src/macros.hpp"
#include "../../cpplib//src/s.hpp"
#include "../../cpplib/src/error.hpp"
#include "../../cpplib/src/concept.hpp"
#include "../../cpplib/src/stop_watch.hpp"
#include "../../cpplib/src/path.hpp"
#include "../../cpplib/src/stream_util.hpp"
#include "../../cpplib/src/version.hpp"

#include <string>
#include <stdexcept>
#include <sstream>
#include <unordered_map>
#include <queue>
#include <iostream>
#include <concepts>
#include <span>
#include <cmath> // Added for std::abs

namespace pensar_digital
{
    namespace pd = pensar_digital::cpplib;
    namespace unit_test
    {
        using namespace cpplib;

        const int UNORDERED = -1;
        class Failure : Error
        {
            public:
                //inline static const pd::VersionPtr VERSION = pd::Version::get (1, 1, 1);
            using Error::get_error_message;
            Failure (const Id id,
                     const std::basic_string<C>&     name,
                     const std::basic_string<C>&     err_msg,
                     const std::basic_string<C>&     afile,
                     const unsigned    aline):
                Error(err_msg, id),
                test_id (id),
                test_name (name),
                file (afile),
                line (aline)
            {
				SStream ss;
				ss << file << W(" line \t") << line << W("\ttest_id = ") << id << W("\ttest_name = ") << name << W("\terror = ") << err_msg;
				Error::set_error_message (ss.str ());
            };

            private:
            const Id test_id;
            const S& test_name;
            const S& file;
            const unsigned line;
        };

        /// Test is the interface to be implemented by all test classes..
        class Test : public Object
        {
            private:
                /// ID generator for tests.
                static Generator<Test> generator;
                int order;
                bool stop_on_failure;
                bool enabled;
                S name;
            public:
                static constexpr double DEFAULT_DELTA = 0.0000001; // Added default delta
                //inline static const VersionPtr VERSION = pd::Version::get (1, 1, 1);
                typedef Test T;

            /// Constructor.
            /// \param test_name The test name.
            /// \param aid The test ID. One will be provided by default if you do not pass one.
                Test(const   S& test_name,
                     const   Id       aid          =   NULL_ID,
                            int       aorder       = UNORDERED,
                           bool       stop_on_fail =      true,
                           bool       is_enabled   = true) :
                name            (test_name   ),
                order           (aorder      ),
                stop_on_failure (stop_on_fail),
                enabled         (is_enabled  ) {}

            /// Virtual destructor.
            virtual ~Test() {}

            /// Tests run calling this method.
            virtual bool run () = 0;

            S get_name () const { return name; }
			
            void set_name (const S& a_name) { name = a_name; }

            // Specialization for floating-point types
            template <std::floating_point T>
            bool check_equal(const T& actual, const T& expected, const S& error_message, const S& file, const unsigned line, double delta = DEFAULT_DELTA) const
            {
                bool ok = std::abs(actual - expected) < delta;
                if (!ok)
                    error<T>(actual, expected, error_message + W(" (delta = ") + pd::to_string(delta) + W(")"), file, line);
                return ok;
            }

            bool check_equal(const char* actual, const char* expected, const S& error_message, const S& file, const unsigned line) const
            {
                bool ok = (strcmp(actual, expected) == 0);
                if (!ok)
                {
                    std::string sactual(actual);
                    std::string sexpected(expected);
                    error(sactual, sexpected, error_message, file, line);
                }
                return ok;
            }
            template <OutputStreamable T>
            void error(const T& actual, const T& expected, const S& error_message, const S& file, const unsigned line) const
			{
                if constexpr (std::is_same_v<T, std::string>) 
                {
                    S sactual;
                    S sexpected;
                    #ifdef WIDE_CHAR
                        sactual = to_wstring(actual);
                        sexpected = to_wstring(expected);
                        out() << file << W(" line \t") << line << W("\t actual [") << sactual << W("] != [") << sexpected << W("] expected\t") << error_message << std::endl;
                    #else
                        sactual = actual;
                        sexpected = expected;
                        out() << file << W(" line \t") << line << W("\t actual [") << sactual << W("] != [") << sexpected << W("] expected\t") << error_message << std::endl;
                    #endif      
                }
                else
                {
                    out() << file << W(" line \t") << line << W("\t actual [") << actual << W("] != [") << expected << W("] expected\t") << error_message << std::endl;
                }
                if (stop_on_failure)
                    throw Failure (pd::Object::id(),
                        get_name (),
                        error_message, file, line);
            }
            
            /// If expression is false and stop_on_failure = true throws an Error exception.
            bool check(bool expression, const S& error_message, const S& file, const unsigned line) const
            {
                if (!expression)
                {
                    error<bool>(expression, true, error_message, file, line);
                }
                return expression;
            }

            /// <summary>
            /// 
            /// </summary>
            /// <typeparam name="T"></typeparam>
            /// <param name="actual"></param>
            /// <param name="expected"></param>
            /// <param name="error_message"></param>
            /// <param name="file"></param>
            /// <param name="line"></param>
            /// <returns></returns>
            template <OutputStreamable  T> bool check_equal(const T& actual, const T& expected, const S& error_message, const S& file, const unsigned line) const
            {
                bool ok = (actual == expected);
                if (! ok)
                    error<T>(actual, expected, error_message, file, line);
                return ok;
            };


           /* bool check_equal(const wchar_t* actual, const wchar_t* expected, const S& error_message, const S& file, const unsigned line) const
            {
               bool ok = (wcscmp(actual, expected) == 0);
               if (! ok)
               {
				   std::wstring sactual(actual);
				   std::wstring sexpected(expected);
				   error<std::wstring>(sactual, sexpected, error_message, file, line);
			   }
               return ok;
			}*/
            
            //----------------
            template <OutputStreamable  T> bool check_not_equal(const T& actual, const T& expected, const S& error_message, const S& file, const unsigned line) const
            {
                bool ok = (actual != expected);
                if (!ok)
                    error(actual, expected, error_message, file, line);
                return ok;
            };

            // Specialization for floating-point types
            template <std::floating_point T>
            bool check_not_equal(const T& actual, const T& expected, const S& error_message, const S& file, const unsigned line, double delta = DEFAULT_DELTA) const
            {
                bool ok = std::abs(actual - expected) >= delta;
                if (!ok)
                    error<T>(actual, expected, error_message + W(" (delta = ") + pd::to_string(delta) + W(")"), file, line);
                return ok;
            }

            bool check_not_equal(const char* actual, const char* expected, const S& error_message, const S& file, const unsigned line) const
            {
                bool ok = (strcmp(actual, expected) != 0);
                if (!ok)
                {
                    std::string sactual(actual);
                    std::string sexpected(expected);
                    error<std::string>(sactual, sexpected, error_message, file, line);
                }
                return ok;
            }

            /*bool check_not_equal(const wchar_t* actual, const wchar_t* expected, const S& error_message, const S& file, const unsigned line) const
            {
                bool ok = (wcscmp(actual, expected) != 0);
                if (!ok)
                {
                    std::wstring sactual(actual);
                    std::wstring sexpected(expected);
                    error(sactual, sexpected, error_message, file, line);
                }
                return ok;
            }*/

            template <Container A, Container E> 
            bool check_equal_collection(const A& actual, const E& expected, const S& error_message, const S& file, const unsigned line) const
            {
                bool ok = false;
                try
                {
                    ok = (actual.size() == expected.size());
                    if (not ok)
                    {
                        out () << file << W(" line \t");
                        out () << line << W("\t actual size [") << actual.size();
                        out () << W("] != [") << expected.size() << W("] expected size\t") << error_message << std::endl;
                        if (stop_on_failure)
                            throw Failure (pd::Object::id (),
                                              get_name (),
                                              error_message, file, line);
                    }

                    if (ok)
                    {
                        for (size_t i = 0; i < actual.size(); i++)
                        {
                            S err = error_message + W(" at index ");
                            err += pd::to_string<size_t, false>(i);
                            typename E::value_type const expected_value = expected[i];
                            typename A::value_type const actual_value = actual[i];
                            ok = check_equal<typename A::value_type>(actual_value, expected_value, err, file, line);
                        }
                    }
                    return ok;
                }
                catch (const Exception& e)
                {
                    ok = false;
                    if (stop_on_failure)
                        throw Failure (pd::Object::id(), 
                                          get_name (), 
                                          error_message + e.what_error(), file, line);
                }
                return ok;
            };


            int           get_order          () const { return order;             }
            bool          get_stop_on_failure() const { return stop_on_failure;   }
            bool          is_enabled         () const { return enabled;           }
            bool          is_ordered         () const { return order == UNORDERED;}

            Test& enable () { enabled = true ; return *this; }
            Test& disable() { enabled = false; return *this; }

            Test& set_stop_on_failure ( bool Stop         ) { stop_on_failure = Stop; return *this; }

            bool operator==  (const T& t) const { return pd::Object::id () == t.id ();}
            bool operator<   (const T& t) const { return order < t.order;       }
            bool operator!=  (const T& t) const { return !(*this == t);          }

            virtual std::istream& read (std::istream& is, const std::endian& byte_order = std::endian::native)
            {
                return is;
            }

            virtual std::ostream& write (std::ostream& os, const std::endian& byte_order = std::endian::native) const
            {
                return os;
            }
        };

        /// CompositeTest aggregates several tests together.
        class CompositeTest : public Test
        {
            public:
			inline static const VersionPtr VERSION = pd::Version::get (1, 1, 1);
            typedef Test T;
            typedef std::unordered_map<Id, T*> UnorderedTestMap;
            typedef std::priority_queue<T*> OrderedTestQueue;

            CompositeTest(const S& test_name = W(""), const Id aid = NULL_ID, int aorder = UNORDERED, bool stop_on_fail = true) :
                T(test_name, aid, aorder, stop_on_fail),
                unordered_tests(UnorderedTestMap()),
                ordered_tests (OrderedTestQueue ())
            {
                set_id (generator.get_id ());
            }

            void add (T& test)
            {
                test.set_stop_on_failure (Test::get_stop_on_failure ());
                if (test.is_ordered ())
                    ordered_tests.push (&test);
                else
                    unordered_tests.insert (std::pair<Id, T*> (test.get_order (), &test));
            };

            void add (T* test) { add (*test); };

            virtual bool run ()
            {
                namespace pd = pensar_digital::cpplib;
                bool ok = true;
                size_t count = CompositeTest::count ();
                StopWatch<> sw;
                try
                {
                    for (UnorderedTestMap::value_type t: unordered_tests)
                    {
                        if (t.second->is_enabled())
                        {
                            t.second->set_stop_on_failure(Test::get_stop_on_failure());
                            sw.mark ();
                            ok = t.second->run();
                            out () << pd::pad_left0(--count) << W(" ") << pd::pad_copy (t.second->get_name (), W(' '), 25) << W(" ") << sw.elapsed_since_mark_formatted() << std::endl;
                        }
                        else
                        {
                            ok = true;
                            out () << pd::pad_left0 (--count) << W(" ") << t.second->get_name () << W(" is disabled.") << std::endl;
						}
                        if (!ok && Test::get_stop_on_failure ())
                            break;
                    }
                    if (ok)
                    {
                        while (! ordered_tests.empty ())
                        {
                            T *t = ordered_tests.top();
                            if (t->is_enabled())
                            {
                                t->set_stop_on_failure(Test::get_stop_on_failure());
                                sw.mark ();
                                ok = t->run();
                                out () << pd::pad_copy (pd::to_string<size_t, false>(--count), W('0'), 4, PAD_LEFT) << W(" ") << pd::pad_copy(t->get_name (), W(' '), 30) << W(" ") << sw.elapsed_since_mark_formatted() << std::endl;
                            }
                            else
							{
								ok = true;
                                out () << --count << W(" ") << t->get_name () << W(" is disabled.") << std::endl;
							}
                            ordered_tests.pop ();
                            if (!ok && Test::get_stop_on_failure ())
                                break;
                        }
                    }
                    sw.stop();
                    if (ok) { out () << W("ok") << W(" ") << sw.elapsed_formatted (); }
                }
                catch (const Failure& f)
                {
                    out () << f.get_error_message ();
                }
                return ok;
            }

            size_t count () { return ordered_tests.size () + unordered_tests.size ();}
            private:
            UnorderedTestMap unordered_tests;
            OrderedTestQueue ordered_tests;
            static Generator<CompositeTest> generator;
        };


        extern CompositeTest& all_tests ();

        #define CHECK(bool_expression, error_message)       \
                                  check (bool_expression,   \
                                 error_message,             \
                                 __FILE__,                  \
                                 __LINE__);


        #define CHECK_EQ(T, actual, expected, error_message)   \
                                  check_equal<T> (actual, expected,  \
                                 error_message,             \
                                 __FILE__,                  \
                                 __LINE__);

        #define CHECK_EQ_STR(actual, expected, error_message)   \
                                  check_equal (actual, expected,  \
                                 error_message,             \
                                 __FILE__,                  \
                                 __LINE__);

        #define CHECK_NOT_EQ(T, actual, expected, error_message)   \
                                  check_not_equal<T> (actual, expected,  \
                                 error_message,             \
                                 __FILE__,                  \
                                 __LINE__);

#define TEST_PREDICATE(name, bool_expression, error_message)        \
                      class Test ## name : public Test\
                      {                                            \
                        public:                                    \
                        inline static const Version VERSION = Version (1, 1, 1); \
                        Test ## name ()                            \
                        : Test (W(#name)){all_tests ().add(this);};             \
                        bool run ()                                \
                        {                                          \
                          CHECK(bool_expression, error_message)    \
                          return true;                             \
                        }                                          \
                      }; Test ## name test_ ## name;

        #define TEST(name, is_enabled) \
                      class Test##name : public Test               \
                      {                                            \
                        public:                                    \
                        inline static const Version VERSION = Version (1, 1, 1); \
                        Test##name ()                              \
                        : Test (W(#name)){is_enabled ? enable () : disable ();all_tests ().add(this);};      \
                        bool run ()                                \
                        {

        #define TEST_END(name)                                           \
                        return true; }                             \
                      }; Test ## name test_ ## name;

        #define WCHECK(bool_expression, error_message)       \
                                  check (bool_expression,   \
                                 error_message,             \
                                 __FILEW__,                  \
                                 __LINE__);


        #define WCHECK_EQ(T, actual, expected, error_message)   \
                                  check_equal<T> (actual, expected,  \
                                 error_message,             \
                                 __FILEW__,                  \
                                 __LINE__);

        #define WCHECK_NOT_EQ(T, actual, expected, error_message)   \
                                  check_not_equal<T> (actual, expected,  \
                                 error_message,             \
                                 __FILEW__,                  \
                                 __LINE__);

            }  // namespace unit_test
 }  // namespace pensar_digital

#endif // TEST_HPP
