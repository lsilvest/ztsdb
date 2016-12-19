// (C) 2016 Leonardo Silvestri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.


#ifndef ALLOCATOR_HPP
#define ALLOCATOR_HPP


#include <string>
#include <exception> 
#include <iostream>
#include <system_error>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <boost/filesystem.hpp>
#include "misc.hpp"


// #define DEBUG
namespace fsys = boost::filesystem;
using namespace std::literals;


namespace arr {

  struct baseallocator {
    virtual void* allocate(size_t sz) = 0;
    virtual void  deallocate(void* t, size_t n) = 0;
    virtual void* reallocate(void* t, size_t n) = 0;
    virtual void* initialize() = 0;
    virtual size_t size() const = 0;
    inline virtual void msync(bool async) const {
      throw std::range_error("msync not defined for allocator type");
    }
    virtual ~baseallocator() { }
  };


  struct memallocator : baseallocator {
    memallocator() : t(nullptr) { }
    inline void* allocate(size_t n) {
      t = malloc(n); 
      if (t == nullptr) {
        throw std::system_error(std::error_code(errno, std::system_category()), "malloc");
      }
      return t;
    }
    inline void deallocate(void* address, size_t n) { 
      free(t); 
      t = nullptr;
    }
    inline void* reallocate(void* old_address, size_t n) { 
      if (old_address != t) {
        throw std::out_of_range("memallocator can't reallocate at an offset");
      }
      void* new_t = realloc(old_address, n); 
      if (new_t == nullptr) {
        throw std::system_error(std::error_code(errno, std::system_category()), "realloc");
      }
      t = new_t;
      return t;
    }
    inline void* initialize() {
      throw std::out_of_range("memallocator has nothing to initialize from");
    }
    inline size_t size() const {
      throw std::out_of_range("memallocator does not provide size");
    }
    ~memallocator() { if (t) free(t); }
  private:
    void* t;
  };


  struct flexallocator : baseallocator {
    flexallocator() : offset(0), t(nullptr), n(0), pagesz(sysconf(_SC_PAGESIZE)) { }
  
    inline void* allocate(size_t n_p) {
      size_t pages = n_p / pagesz;
      if (n_p % pagesz) {
        ++pages;
      }
      n = pages * pagesz;
      t = mmap(NULL, n, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, -1, 0);
      if (t == (void *)-1) {
        throw std::system_error(std::error_code(errno, std::system_category()), "mmap (PRIVATE|ANON)");
      }
      return t;
    }

    inline virtual void deallocate(void* t_p, size_t n_p) { 
      if (munmap(t_p, n_p) == -1) {
        throw std::system_error(std::error_code(errno, std::system_category()), "munmap");
      } 
    };

    inline virtual void* reallocate(void* t_p, size_t n_p) {
      if (t_p < t) {
        // we can't grow in front without moving the whole area...
        throw std::out_of_range("can't reallocate with an address before start of mapping");
      }
      else if (t_p == t) {
        if (n != n_p) {
          t = mremap(t, n, n_p, MREMAP_MAYMOVE);
          if (t == (void *)-1) {
            throw std::system_error(std::error_code(errno, std::system_category()), "mremap");
          }
          n = n_p;
        }
        return (char*)t + offset;
      }
      else {
        if (t_p > (char*)t + n) {
          throw std::out_of_range("can't reallocate with an address beyond end of mapping");
        }

        size_t sz = (char*)t_p - ((char*)t + offset);
        size_t pages = sz / pagesz;
        offset       = sz % pagesz;

        if (pages) {
          if (munmap(t, pages * pagesz) == -1) {
            throw std::system_error(std::error_code(errno, std::system_category()), "munmap");
          }
          
          t = (char*)t + pages * pagesz;
          n -= pages * pagesz;
        }

        return reallocate(t, n_p + offset); 
      }
    
    };

    inline void* initialize() {
      throw std::out_of_range("flexallocator has nothing to initialize from");
    }
    inline size_t size() const {
      throw std::out_of_range("flexallocator does not provide size");
    }

    ~flexallocator() { munmap(t, n); }

  private:  
    size_t offset;
    void* t;
    size_t n;
    const size_t pagesz;
  };


  struct mmapallocator : baseallocator {
    mmapallocator(const fsys::path& filename_p) : t(nullptr), n(0), fd(-1), filename(filename_p) { }

    inline void* initialize() {
      int fd = open(filename.c_str(), O_RDWR);
      if (fd == -1) {
        throw std::system_error(std::error_code(errno, std::system_category()), 
                                "cannot open "s + filename.c_str());      
      }
      // if file exists, map it:
      off_t sz = lseek(fd, 0L, SEEK_END);
      if (sz == -1) {
        close(fd);
        throw std::system_error(std::error_code(errno, std::system_category()), "lseek");
      }   
      t = static_cast<void*>(mmap(NULL, sz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
      if (t == MAP_FAILED) {
        close(fd);
        throw std::system_error(std::error_code(errno, std::system_category()), "mmap");
      }
      n = sz;
      return t;                   // nullptr if the file didn't exist
    }

    inline size_t size() const { return n; }

    inline void* allocate(size_t sz) {
      fd = open(filename.c_str(), O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
      if (fd == -1) {
        throw std::system_error(std::error_code(errno, std::system_category()), "open");
      }  
      if (posix_fallocate(fd, 0, sz) == -1) {
        close(fd);
        throw std::system_error(std::error_code(errno, std::system_category()), "posix_fallocate");
      }
      t = static_cast<void*>(mmap(NULL, sz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
      if (t == MAP_FAILED) {
        close(fd);
        throw std::system_error(std::error_code(errno, std::system_category()), "mmap");
      }
      // close(fd);                  // file descriptor can be closed now
      n = sz;
      return t;
    }

    inline void deallocate(void* address, size_t n) {
      if (t) {
        // unmap and delete the file
        if (munmap(address, n) == -1) {
          throw std::system_error(std::error_code(errno, std::system_category()), "munmap");    
        }
        t = nullptr;
        n = 0;
        if (remove(filename.c_str()) != 0) {
          throw std::system_error(std::error_code(errno, std::system_category()), 
                                  "can't remove "s + filename.c_str());    
        }
      }
    }

    inline void* reallocate(void* old_address, size_t new_size) {
      if (old_address != t) {
        throw std::out_of_range("mmapallocator can't reallocate at an offset");
      }
      if (lseek(fd, new_size, SEEK_SET) == -1) {
        throw std::system_error(std::error_code(errno, std::system_category()), "lseek");
      }
      if (write(fd, "", 1) != 1) {
        close(fd);
        throw std::system_error(std::error_code(errno, std::system_category()), "write");
      }
      void *new_t = mremap(t, n, new_size, MREMAP_MAYMOVE);
      if (new_t == MAP_FAILED) {
        throw std::system_error(std::error_code(errno, std::system_category()), "mremap");
      }
      n = new_size;
      t = new_t;
      return t;
    }

    inline virtual void msync(bool async) const {
      if (async) {
        if (::msync(t, n, MS_SYNC) == -1) {
          throw std::system_error(std::error_code(errno, std::system_category()), "msync");    
        }
      }
      else {
        if (::msync(t, n, MS_ASYNC) == -1) {
          throw std::system_error(std::error_code(errno, std::system_category()), "msync");    
        }
      }
    }          
        
    virtual ~mmapallocator() {
      if (fd != -1) {
        close(fd);                // we can close
      }
      if (t) {
        // std::cout << filename << " msync" << std::endl;
        if (::msync(t, n, MS_SYNC) == -1) {
          throw std::system_error(std::error_code(errno, std::system_category()), "msync");    
        }
        // std::cout << filename << " munmap" << std::endl;
        if (munmap(t, n) == -1) {
          throw std::system_error(std::error_code(errno, std::system_category()), "munmap");    
        }
      }
    }

  private:
    void* t;
    size_t n;
    int fd;
    const fsys::path filename;
  };


} // end namespace arr

#endif
