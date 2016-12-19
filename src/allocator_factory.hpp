// (C) 2016 Leonardo Silvestri
//
// This file is part of ztsdb.
//
// ztsdb is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ztsdb is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ztsdb.  If not, see <http://www.gnu.org/licenses/>.


#ifndef ALLOCATOR_FACTORY_HPP
#define ALLOCATOR_FACTORY_HPP


#include <memory>
#include <string>
#include <boost/filesystem.hpp>
#include "allocator.hpp"


namespace fsys = boost::filesystem;


namespace arr {

  struct AllocFactory {
    virtual std::unique_ptr<baseallocator> get(const std::string& name) const = 0;
    virtual std::unique_ptr<baseallocator> get(size_t nb) const = 0;
    virtual ~AllocFactory() { }
    virtual std::string to_string() const = 0;
    inline virtual fsys::path getDirname() const {
      throw std::range_error("no file mapping");
    }
  };

  struct MemAllocFactory : public AllocFactory {
    inline std::unique_ptr<baseallocator> get(const std::string&) const {
      return std::make_unique<memallocator>();
    }
    inline std::unique_ptr<baseallocator> get(size_t) const {
      return std::make_unique<memallocator>();
    }
    inline std::string to_string() const {
      return "malloc-based"s;
    }
  };

  struct FlexAllocFactory : public AllocFactory {
    inline std::unique_ptr<baseallocator> get(const std::string&) const {
      return std::make_unique<flexallocator>();
    }    
    inline std::unique_ptr<baseallocator> get(size_t) const {
      return std::make_unique<flexallocator>();
    }    
    inline std::string to_string() const {
      return "mmap-based"s;
    }
  };

  struct MmapAllocFactory : public AllocFactory {
    MmapAllocFactory(const fsys::path& dirname_p, bool expectExists) : dirname(dirname_p) {
      struct stat st = {0};
      if (stat(dirname.c_str(), &st) == 0) {
        if (!expectExists) {
          throw std::range_error(dirname.c_str() + " already exists"s);
        }
      }
      else {
        if (expectExists) {
          throw std::range_error(dirname.c_str() + " does not exist"s);
        }
        else {
          if (mkdir(dirname.c_str(), 0700) == -1) {
            throw std::system_error(std::error_code(errno, std::system_category()), 
                                    "cannot create "s + dirname.c_str());
          }
        }
      }
    }

    inline std::unique_ptr<baseallocator> get(const std::string& name) const {
      return std::make_unique<mmapallocator>(dirname / fsys::path(name));
    }
    inline std::unique_ptr<baseallocator> get(size_t nb) const {
      return std::make_unique<mmapallocator>(dirname / fsys::path(std::to_string(nb)));
    }

    inline std::string to_string() const {
      return "mmap file = "s + dirname.c_str();
    }

    inline virtual fsys::path getDirname() const { return dirname; }

  private:
    const fsys::path dirname;
  };

} // end namespace arr

#endif
