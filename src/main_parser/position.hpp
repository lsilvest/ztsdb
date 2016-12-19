// A Bison parser, made by GNU Bison 3.0.2.

// Positions for Bison parsers in C++

// Copyright (C) 2002-2013 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.

// This version for ztsdb is lightly modified from the original
// Bison-generated location/position files. Mainly, we add a
// shared_ptr to the buffer being parsed in the 'position' class, and
// make sure the addition of this element if properly handled by the
// constructors.

/**
 ** \file position.hpp
 ** Define the yy::position class.
 */

#ifndef YY_YY_POSITION_HH_INCLUDED
# define YY_YY_POSITION_HH_INCLUDED

# include <algorithm> // std::max3
# include <iostream>
# include <string>

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif


namespace yy {
  /// Abstract a position.
  class position
  {
  public:
    /// Construct a position.
    explicit position (const std::string& filename_p = std::string(),
                       unsigned int l = 1u,
                       unsigned int c = 1u,
                       std::shared_ptr<const std::string> s_p = 
                         std::make_shared<const std::string>())
      : filename(filename_p)
      , line (l)
      , column (c)
      , s(s_p)
    {
    }


    /// Initialization.
    void initialize (const std::string& filename_p,
                     unsigned int l,
                     unsigned int c,
                     std::shared_ptr<const std::string> s_p)
    {
      filename = filename_p;
      line = l;
      column = c;
      s = s_p;
    }

    /** \name Line and Column related manipulators
     ** \{ */
    /// (line related) Advance to the COUNT next lines.
    void lines (int count = 1)
    {
      if (count)
        {
          column = 1u;
          line = add_ (line, count, 1);
        }
    }

    /// (column related) Advance to the COUNT next columns.
    void columns (int count = 1)
    {
      column = add_ (column, count, 1);
    }
    /** \} */

    /// File name to which this location refers.
    std::string filename;
    /// Current line number.
    unsigned int line;
    /// Current column number.
    unsigned int column;
    /// Shared pointer to the buffer being parsed;
    std::shared_ptr<const std::string> s;

  private:
    /// Compute max(min, lhs+rhs) (provided min <= lhs).
    static unsigned int add_ (unsigned int lhs, int rhs, unsigned int min)
    {
      return (0 < rhs || -static_cast<unsigned int>(rhs) < lhs
              ? rhs + lhs
              : min);
    }
  };

  /// Add and assign a position.
  inline position&
  operator+= (position& res, int width)
  {
    res.columns (width);
    return res;
  }

  /// Add two position objects.
  inline position
  operator+ (position res, int width)
  {
    return res += width;
  }

  /// Add and assign a position.
  inline position&
  operator-= (position& res, int width)
  {
    return res += -width;
  }

  /// Add two position objects.
  inline position
  operator- (position res, int width)
  {
    return res -= width;
  }

  /// Compare two position objects.
  inline bool
  operator== (const position& pos1, const position& pos2)
  {
     return (pos1.line == pos2.line
            && pos1.column == pos2.column
             && (pos1.filename == pos2.filename));
  }

  /// Compare two position objects.
  inline bool
  operator!= (const position& pos1, const position& pos2)
  {
    return !(pos1 == pos2);
  }

  /** \brief Intercept output stream redirection.
   ** \param ostr the destination output stream
   ** \param pos a reference to the position to redirect
   */
  template <typename YYChar>
  inline std::basic_ostream<YYChar>&
  operator<< (std::basic_ostream<YYChar>& ostr, const position& pos)
  {
    if (!pos.filename.empty())
      ostr << pos.filename << ':';
    return ostr << pos.line << '.' << pos.column;
  }


} // yy
#endif // !YY_YY_POSITION_HH_INCLUDED
