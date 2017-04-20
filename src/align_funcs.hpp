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


#ifndef ALIGN_FUNCS_HPP
#define ALIGN_FUNCS_HPP


#include <queue>
#include "globals.hpp"


namespace ztsdb {
  
  template <typename FIter, typename T>
  struct mean_element {
    static T f(FIter b, FIter e) {
      if (e-b == 0) return Global::ZNAN;
      return std::accumulate(b, e, static_cast<T>(0)) / (e-b);
    }
  };

  template <typename FIter, typename T>
  struct max_element {
    static T f(FIter b, FIter e) { 
      if (e-b == 0) return Global::ZNAN;
      return *std::max_element(b, e); 
    }
  };

  template <typename FIter, typename T>
  struct min_element {
    static T f(FIter b, FIter e) { 
      if (e-b == 0) return Global::ZNAN;
      return *std::min_element(b, e); 
    }
  };

  template <typename FIter, typename T>
  struct count_element {
    static T f(FIter b, FIter e) { 
      if (e-b == 0) return 0;
      return e-b;
    }
  };

  template <typename FIter, typename T>
  struct median_element {
    static T f(FIter b, FIter e) { 
      if (e-b == 0) return Global::ZNAN;

      std::priority_queue<double> left;
      std::priority_queue<
        double,
        std::priority_queue<double>::container_type,
        std::greater<double>
        > right;
          
      for (auto i=b; i!=e; ++i) {
        if (left.size() == right.size()) {
          if (!left.size() || *i < left.top()) {
            left.push(*i);
          }
          else {
            right.push(*i);
          }
        }
        else if (left.size() < right.size()) {
          if (*i < right.top()) {
            left.push(*i);
          }
          else {
            left.push(right.top());
            right.pop();
            right.push(*i);
          }
        }
        else { // left.size() > right.size()
          if (*i > left.top()) {
            right.push(*i);
          }
          else {
            right.push(left.top());
            left.pop();
            left.push(*i);
          }
        }
      }
    
      if (left.size() == right.size()) return (left.top() + right.top())/2.0;
      if (left.size() < right.size()) return right.top();
      else return left.top();
    }
  };


  template <typename FIter, typename F>
  struct applyd {
    static void f(double d, FIter b, FIter e) { 
      std::for_each(b, e, [d](double &n){ n = F()(n, d); });
    }
  };

} // end namespace ztsdb


#endif
