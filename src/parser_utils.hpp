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


#include <unordered_map>
#include "parser.hpp"           // bison generated

namespace ztsdb {

  const std::map<int, string> op_to_string = {
    {yy::parser::token::GT, ">"},
    {yy::parser::token::GE, ">="},
    {yy::parser::token::LT, "<"},
    {yy::parser::token::LE, "<="},
    {yy::parser::token::EQ, "=="},
    {yy::parser::token::NE, "!="},
    {yy::parser::token::AND, "&"},
    {yy::parser::token::OR, "|"},
    {yy::parser::token::AND2, "&&"},
    {yy::parser::token::OR2, "||"},
    {yy::parser::token::PLUS, "+"},
    {yy::parser::token::MINUS, "-"},
    {yy::parser::token::MUL, "*"},
    {yy::parser::token::DIV, "/"},
    {yy::parser::token::POWER, "^"},
    {yy::parser::token::NOT, "!"},
    {yy::parser::token::UMINUS, "-"},
    {yy::parser::token::UPLUS, "+"},
    {yy::parser::token::UNOT, "!"},
    {yy::parser::token::COLON, ":"}
  };

}
