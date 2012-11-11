// Copyright Joshua Boyce 2010-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// This file is part of HadesMem.
// <http://www.raptorfactor.com/> <raptorfactor@raptorfactor.com>

#include "hadesmem/alloc.hpp"

#include <iostream>

#include "hadesmem/error.hpp"
#include "hadesmem/process.hpp"

namespace hadesmem
{

PVOID Alloc(Process const& process, SIZE_T size)
{
  PVOID const address = ::VirtualAllocEx(process.GetHandle(), nullptr, 
    size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
  if (!address)
  {
    DWORD const last_error = GetLastError();
    HADESMEM_THROW_EXCEPTION(Error() << 
      ErrorString("VirtualAllocEx failed.") << 
      ErrorCodeWinLast(last_error));
  }

  return address;
}

void Free(Process const& process, LPVOID address)
{
  if (!::VirtualFreeEx(process.GetHandle(), address, 0, MEM_RELEASE))
  {
    DWORD const last_error = GetLastError();
    HADESMEM_THROW_EXCEPTION(Error() << 
      ErrorString("VirtualFreeEx failed.") << 
      ErrorCodeWinLast(last_error));
  }
}

Allocator::Allocator(Process const* process, SIZE_T size)
  : process_(process), 
  base_(Alloc(*process, size)), 
  size_(size)
{
  assert(process != nullptr);
  assert(size != 0);
}

Allocator::Allocator(Allocator&& other) HADESMEM_NOEXCEPT
  : process_(other.process_), 
  base_(other.base_), 
  size_(other.size_)
{
  other.process_ = nullptr;
  other.base_ = nullptr;
  other.size_ = 0;
}

Allocator& Allocator::operator=(Allocator&& other) HADESMEM_NOEXCEPT
{
  FreeUnchecked();
  
  assert(process_ == nullptr);
  assert(base_ == nullptr);
  assert(size_ == 0);
  
  process_ = other.process_;
  base_ = other.base_;
  size_ = other.size_;
  
  other.process_ = nullptr;
  other.base_ = nullptr;
  other.size_ = 0;
  
  return *this;
}

Allocator::~Allocator()
{
  FreeUnchecked();
}

void Allocator::Free()
{
  if (!process_)
  {
    return;
  }
  
  assert(base_ != nullptr);
  assert(size_ != 0);
  
  ::hadesmem::Free(*process_, base_);
  
  process_ = nullptr;
  base_ = nullptr;
  size_ = 0;
}

PVOID Allocator::GetBase() const HADESMEM_NOEXCEPT
{
  return base_;
}

SIZE_T Allocator::GetSize() const HADESMEM_NOEXCEPT
{
  return size_;
}

void Allocator::FreeUnchecked() HADESMEM_NOEXCEPT
{
  try
  {
    Free();
  }
  catch (std::exception const& e)
  {
    (void)e;
    
    // WARNING: Memory in remote process is leaked if 'Free' fails
    assert(e.what() && false);
    
    process_ = nullptr;
    base_ = nullptr;
    size_ = 0;
  }
}

bool operator==(Allocator const& lhs, Allocator const& rhs) HADESMEM_NOEXCEPT
{
  return lhs.GetBase() == rhs.GetBase();
}

bool operator!=(Allocator const& lhs, Allocator const& rhs) HADESMEM_NOEXCEPT
{
  return !(lhs == rhs);
}

bool operator<(Allocator const& lhs, Allocator const& rhs) HADESMEM_NOEXCEPT
{
  return lhs.GetBase() < rhs.GetBase();
}

bool operator<=(Allocator const& lhs, Allocator const& rhs) HADESMEM_NOEXCEPT
{
  return lhs.GetBase() <= rhs.GetBase();
}

bool operator>(Allocator const& lhs, Allocator const& rhs) HADESMEM_NOEXCEPT
{
  return lhs.GetBase() > rhs.GetBase();
}

bool operator>=(Allocator const& lhs, Allocator const& rhs) HADESMEM_NOEXCEPT
{
  return lhs.GetBase() >= rhs.GetBase();
}

std::ostream& operator<<(std::ostream& lhs, Allocator const& rhs)
{
  return (lhs << rhs.GetBase());
}

std::wostream& operator<<(std::wostream& lhs, Allocator const& rhs)
{
  return (lhs << rhs.GetBase());
}

}
