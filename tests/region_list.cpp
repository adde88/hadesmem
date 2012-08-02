// Copyright Joshua Boyce 2010-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// This file is part of HadesMem.
// <http://www.raptorfactor.com/> <raptorfactor@raptorfactor.com>

#include "hadesmem/region_list.hpp"

#define BOOST_TEST_MODULE region_list
#include "hadesmem/detail/warning_disable_prefix.hpp"
#include <boost/concept_check.hpp>
#include <boost/test/unit_test.hpp>
#include "hadesmem/detail/warning_disable_suffix.hpp"

#include "hadesmem/error.hpp"
#include "hadesmem/region.hpp"
#include "hadesmem/process.hpp"

// Boost.Test causes the following warning under GCC:
// error: base class 'struct boost::unit_test::ut_detail::nil_t' has a 
// non-virtual destructor [-Werror=effc++]
#if defined(HADESMEM_GCC)
#pragma GCC diagnostic ignored "-Weffc++"
#endif // #if defined(HADESMEM_GCC)

BOOST_TEST_DONT_PRINT_LOG_VALUE(hadesmem::RegionList::iterator)

BOOST_AUTO_TEST_CASE(region_list)
{
  BOOST_CONCEPT_ASSERT((boost::InputIterator<hadesmem::RegionList::
    iterator>));
  BOOST_CONCEPT_ASSERT((boost::InputIterator<hadesmem::RegionList::
    const_iterator>));
  
  hadesmem::Process const process(::GetCurrentProcessId());
  
  using std::begin;
  using std::end;
  
  hadesmem::RegionList const region_list_1(&process);
  hadesmem::RegionList region_list_2(region_list_1);
  hadesmem::RegionList region_list_3(std::move(region_list_2));
  region_list_2 = std::move(region_list_3);
  BOOST_CHECK_NE(begin(region_list_2), end(region_list_2));
  
  auto iter = begin(region_list_1);
  hadesmem::Region const first_region(&process, nullptr);
  BOOST_CHECK_NE(iter, end(region_list_1));
  BOOST_CHECK_EQUAL(*iter, first_region);
  hadesmem::Region const second_region(&process, 
    static_cast<char const* const>(first_region.GetBase()) + 
    first_region.GetSize());
  BOOST_CHECK_NE(++iter, end(region_list_1));
  BOOST_CHECK_EQUAL(*iter, second_region);
  hadesmem::Region last(&process, nullptr);
  do
  {
    hadesmem::Region current = *iter;
    BOOST_CHECK_GT(current, last);
    BOOST_CHECK_GE(current, last);
    BOOST_CHECK_LT(last,current);
    BOOST_CHECK_LE(last, current);
    last = current;
  } while (++iter != end(region_list_1));
  // TODO: Compare our last region with the 'real' last region to ensure they 
  // match.
}

BOOST_AUTO_TEST_CASE(process_list_algorithm)
{
  hadesmem::Process const process(::GetCurrentProcessId());
  
  using std::begin;
  using std::end;
  
  hadesmem::RegionList const region_list_1(&process);
  
  std::for_each(begin(region_list_1), end(region_list_1), 
    [&] (hadesmem::Region const& region)
    {
      hadesmem::Region const other(&process, region.GetBase());
      BOOST_CHECK_EQUAL(region, other);
      
      if (region.GetState() != MEM_FREE)
      {
        BOOST_CHECK_NE(region.GetBase(), static_cast<void*>(nullptr));
        BOOST_CHECK_NE(region.GetAllocBase(), static_cast<void*>(nullptr));
        BOOST_CHECK_NE(region.GetAllocProtect(), 0U);
        BOOST_CHECK_NE(region.GetType(), 0U);
      }
      
      region.GetProtect();
      
      BOOST_CHECK_NE(region.GetSize(), 0U);
      BOOST_CHECK_NE(region.GetState(), 0U);
    });
  
  HMODULE const user32_mod = GetModuleHandle(L"user32.dll");
  auto user32_iter = std::find_if(begin(region_list_1), end(region_list_1), 
    [user32_mod] (hadesmem::Region const& region)
    {
      return region.GetBase() == user32_mod;
    });
  BOOST_CHECK_NE(user32_iter, end(region_list_1));
}
