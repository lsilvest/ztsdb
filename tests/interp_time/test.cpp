// -*- compile-command: "make -j -k test" -*-

// Copyright () 2015 Leonardo Silvestri
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


#include "../interp_setup.hpp"
#include "timezone/ztime.hpp"

// subset --------------------------------
// point in time subsetting in array and zts

TEST(itime_subset_dtime_dtime) {
  auto eout = parse
    ("idx <- c(|.2015-03-09 06:38:01 America/New_York.|,|.2015-03-09 06:38:02 America/New_York.|,"
     "         |.2015-03-09 06:38:03 America/New_York.|);"
     "idx[idx[c(1,2)]]\n");
  auto dt1 = tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones);
  auto dt2 = tz::dtime_from_string("2015-03-09 06:38:02 America/New_York", tzones);
  auto a = arr::Array<Global::dtime>({2}, {dt1, dt2}, {{}});
  ASSERT_TRUE(eval(eout) == make_cow<arr::Array<Global::dtime>>(false, a));
}

TEST(itime_subset_dtime_dtime_supersets) {
  auto eout = parse
    ("a <- c(|.2015-03-09 06:38:00 America/New_York.|,|.2015-03-09 06:38:02 America/New_York.|,"
     "       |.2015-03-09 06:38:03 America/New_York.|);"
     "b <- c(|.2015-03-09 06:38:01 America/New_York.|,|.2015-03-09 06:38:02 America/New_York.|,"
     "       |.2015-03-09 06:38:04 America/New_York.|);"
     "a[b]\n");
  auto dt2 = tz::dtime_from_string("2015-03-09 06:38:02 America/New_York", tzones);
  ASSERT_TRUE(eval(eout) == val::make_array(dt2));
}

TEST(itime_subset_dtime_dtime_null_set) {
  auto eout = parse
    ("a <- c(|.2015-03-09 06:38:00 America/New_York.|,|.2015-03-09 06:38:02 America/New_York.|,"
     "       |.2015-03-09 06:38:03 America/New_York.|);"
     "b <- c(|.2015-03-10 06:38:01 America/New_York.|,|.2015-03-10 06:38:02 America/New_York.|,"
     "       |.2015-03-10 06:38:04 America/New_York.|);"
     "a[b]\n");
  auto a = arr::Array<Global::dtime>({0}, {}, std::vector<arr::Vector<arr::zstring>>{{}});
  ASSERT_TRUE(eval(eout) == make_cow<arr::Array<Global::dtime>>(false, a));
}

TEST(itime_subset_dtime_dtime_matrix_error) {
  auto eout = parse
    ("a <- c(|.2015-03-09 06:38:00 America/New_York.|,|.2015-03-09 06:38:02 America/New_York.|,"
     "       |.2015-03-09 06:38:03 America/New_York.|);"
     "b <- c(|.2015-03-10 06:38:01 America/New_York.|,|.2015-03-10 06:38:02 America/New_York.|,"
     "       |.2015-03-10 06:38:04 America/New_York.|);"
     "cbind(a,a)[b]\n");
  ASSERT_THROW(eval(eout), interp::EvalException, "time point index into non-vector");
}

TEST(itime_subset_double_dtime_matrix_error) {
  auto eout = parse
    ("a <- 1.0:10;"
     "b <- c(|.2015-03-10 06:38:01 America/New_York.|,|.2015-03-10 06:38:02 America/New_York.|,"
     "       |.2015-03-10 06:38:04 America/New_York.|);"
     "a[b]\n");
  ASSERT_THROW(eval(eout), interp::EvalException, "time point index into non time point vector");
}

TEST(itime_subset_dtime_scalar_dtime_matrix_error) {
  auto eout = parse
    ("a <- c(|.2015-03-09 06:38:00 America/New_York.|,|.2015-03-09 06:38:02 America/New_York.|,"
     "       |.2015-03-09 06:38:03 America/New_York.|);"
     "b <-   |.2015-03-10 06:38:01 America/New_York.|;"
     "cbind(a,a)[b]\n");
  ASSERT_THROW(eval(eout), interp::EvalException, "time point index into non-vector");
}

TEST(itime_subset_double_scalar_dtime_matrix_error) {
  auto eout = parse
    ("a <- 1.0:10;"
     "b <- |.2015-03-10 06:38:01 America/New_York.|;"
     "a[b]\n");
  ASSERT_THROW(eval(eout), interp::EvalException, "time point index into non time point vector");
}

TEST(itime_subset_zts_dtime) {
  auto eout = parse
    ("idx <- c(|.2015-03-09 06:38:01 America/New_York.|,|.2015-03-09 06:38:02 America/New_York.|,"
     "         |.2015-03-09 06:38:03 America/New_York.|);"
     "z   <- zts(idx, 1.0:6, dim=c(3, 2));"
     "z[idx[c(1,2)],]\n");
  auto dt1 = tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones);
  auto dt2 = tz::dtime_from_string("2015-03-09 06:38:02 America/New_York", tzones);
  auto z = arr::zts({2,2}, {dt1, dt2}, {1,2,4,5}, {{}, {}});
  auto ee = val::to_string(eval(eout));
  val::Value yy = make_cow<arr::zts>(false, z);
  cout << "ee: " << ee << endl;
  cout << "yy: " << val::to_string(yy) << endl;
  ASSERT_TRUE(eval(eout) == make_cow<arr::zts>(false, z));
}

TEST(itime_subset_zts_dtime_names) {
  auto eout = parse
    ("idx <- c(|.2015-03-09 06:38:01 America/New_York.|,"
     "         |.2015-03-09 06:38:02 America/New_York.|,"
     "         |.2015-03-09 06:38:03 America/New_York.|);"
     "z   <- zts(idx, 1.0:6, dim=c(3, 2), dimnames=list(NULL, c(\"one\", \"two\")));"
     "z[idx[c(1,2)],]\n");
  auto dt1 = tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones);
  auto dt2 = tz::dtime_from_string("2015-03-09 06:38:02 America/New_York", tzones);
  auto z = arr::zts({2,2}, {dt1, dt2}, {1,2,4,5}, {{}, {"one", "two"}});
  auto ee = val::to_string(eval(eout));
  val::Value yy = make_cow<arr::zts>(false, z);
  cout << "ee: " << ee << endl;
  cout << "yy: " << val::to_string(yy) << endl;
  ASSERT_TRUE(eval(eout) == make_cow<arr::zts>(false, z));
}

TEST(itime_subset_zts_dtime_1row) {
  auto eout = parse
    ("idx <- c(|.2015-03-09 06:38:01 America/New_York.|,"
     "         |.2015-03-09 06:38:02 America/New_York.|,"
     "         |.2015-03-09 06:38:03 America/New_York.|);"
     "z   <- zts(idx, 1.0:6, dim=c(3, 2));"
     "z[idx[2],]\n");
  auto dt1 = tz::dtime_from_string("2015-03-09 06:38:02 America/New_York", tzones);
  auto z = arr::zts({1,2}, {dt1}, {2,5}, {{}, {}});
  auto ee = val::to_string(eval(eout));
  val::Value yy = make_cow<arr::zts>(false, z);
  cout << "ee: " << ee << endl;
  cout << "yy: " << val::to_string(yy) << endl;
  ASSERT_TRUE(eval(eout) == make_cow<arr::zts>(false, z));
}

TEST(itime_subset_zts_dtime_names_1row) {
  auto eout = parse
    ("idx <- c(|.2015-03-09 06:38:01 America/New_York.|,"
     "         |.2015-03-09 06:38:02 America/New_York.|,"
     "         |.2015-03-09 06:38:03 America/New_York.|);"
     "z   <- zts(idx, 1.0:6, dim=c(3, 2), dimnames=list(NULL, c(\"one\", \"two\")));"
     "z[idx[1],]\n");
  auto dt1 = tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones);
  auto z = arr::zts({1,2}, {dt1}, {1,4}, {{}, {"one", "two"}});
  auto ee = val::to_string(eval(eout));
  val::Value yy = make_cow<arr::zts>(false, z);
  cout << "ee: " << ee << endl;
  cout << "yy: " << val::to_string(yy) << endl;
  ASSERT_TRUE(eval(eout) == make_cow<arr::zts>(false, z));
}


// interval subsetting in arrays and zts
TEST(itime_subset_dtime_interval) {
  auto eout = parse
    ("idx <- c(|.2015-03-09 06:38:01 America/New_York.|,"
     "         |.2015-03-09 06:38:02 America/New_York.|,"
     "         |.2015-03-08 06:38:03 America/New_York.|);"
     "ivl <- |+2015-03-09 06:38:01 America/New_York -> "
     "2015-03-09 06:38:02 America/New_York+|;"
     "idx[ivl]\n");
  auto dt1 = tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones);
  auto dt2 = tz::dtime_from_string("2015-03-09 06:38:02 America/New_York", tzones);
  auto a = arr::Array<Global::dtime>({2}, {dt1, dt2}, {{}});
  ASSERT_TRUE(eval(eout) == make_cow<arr::Array<Global::dtime>>(false, a));
}

TEST(itime_subset_dtime_interval_null_set) {
  auto eout = parse
    ("a <- c(|.2015-03-09 06:38:00 America/New_York.|,|.2015-03-09 06:38:02 America/New_York.|,"
     "       |.2015-03-09 06:38:03 America/New_York.|);"
     "ivl <- |+2015-03-10 06:38:01 America/New_York -> 2015-03-10 06:38:02 America/New_York+|;"
     "a[ivl]\n");
  auto a = arr::Array<Global::dtime>({0}, {}, std::vector<arr::Vector<arr::zstring>>{{}});
  ASSERT_TRUE(eval(eout) == make_cow<arr::Array<Global::dtime>>(false, a));
}

TEST(itime_subset_dtime_interval_matrix_error) {
  auto eout = parse
    ("a <- c(|.2015-03-09 06:38:00 America/New_York.|,"
     "       |.2015-03-09 06:38:02 America/New_York.|,"
     "       |.2015-03-09 06:38:03 America/New_York.|);"
     "ivl <- c(|+2015-03-10 06:38:01 America/New_York -> 2015-03-10 06:38:02 America/New_York+|,"
     "         |+2015-03-10 06:38:03 America/New_York -> 2015-03-10 06:38:04 America/New_York+|);"
     "cbind(a,a)[ivl]\n");
  ASSERT_THROW(eval(eout), interp::EvalException, "interval index into non-vector");
}

TEST(itime_subset_double_interval_matrix_error) {
  auto eout = parse
    ("a <- 1.0:10;"
     "ivl <- c(|+2015-03-10 06:38:01 America/New_York -> 2015-03-10 06:38:02 America/New_York+|,"
     "         |+2015-03-10 06:38:03 America/New_York -> 2015-03-10 06:38:04 America/New_York+|);"
     "a[ivl]\n");
  ASSERT_THROW(eval(eout), interp::EvalException, "interval index into non time point vector");
}

TEST(itime_subset_dtime_scalar_interval_matrix_error) {
  auto eout = parse
    ("a <- c(|.2015-03-09 06:38:00 America/New_York.|,|.2015-03-09 06:38:02 America/New_York.|,"
     "       |.2015-03-09 06:38:03 America/New_York.|);"
     "ivl <- |+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|;"
     "cbind(a,a)[ivl]\n");
  ASSERT_THROW(eval(eout), interp::EvalException, "interval index into non-vector");
}

TEST(itime_subset_double_scalar_interval_matrix_error) {
  auto eout = parse
    ("a <- 1.0:10;"
     "ivl <- |+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|;"
     "a[ivl]\n");
  ASSERT_THROW(eval(eout), interp::EvalException, "interval index into non time point vector");
}

TEST(itime_subset_zts_interval) {
  auto eout = parse
    ("idx <- c(|.2015-03-09 06:38:01 America/New_York.|,|.2015-03-09 06:38:02 America/New_York.|,"
     "         |.2015-03-09 06:38:03 America/New_York.|);"
     "z   <- zts(idx, 1.0:6, dim=c(3, 2));"
     "ivl <- |+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|;"
     "z[ivl,]\n");
  auto dt1 = tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones);
  auto dt2 = tz::dtime_from_string("2015-03-09 06:38:02 America/New_York", tzones);
  auto z = arr::zts({2,2}, {dt1, dt2}, {1,2,4,5}, {{}, {}});
  auto ee = val::to_string(eval(eout));
  val::Value yy = make_cow<arr::zts>(false, z);
  cout << "ee: " << ee << endl;
  cout << "yy: " << val::to_string(yy) << endl;
  ASSERT_TRUE(eval(eout) == make_cow<arr::zts>(false, z));
}

TEST(itime_subset_zts_interval_names) {
  auto eout = parse
    ("idx <- c(|.2015-03-09 06:38:01 America/New_York.|,|.2015-03-09 06:38:02 America/New_York.|,"
     "         |.2015-03-09 06:38:03 America/New_York.|);"
     "z   <- zts(idx, 1.0:6, dim=c(3, 2), dimnames=list(NULL, c(\"one\", \"two\")));"
     "ivl <- |+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|;"
     "z[ivl,]\n");
  auto dt1 = tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones);
  auto dt2 = tz::dtime_from_string("2015-03-09 06:38:02 America/New_York", tzones);
  auto z = arr::zts({2,2}, {dt1, dt2}, {1,2,4,5}, {{}, {"one", "two"}});
  ASSERT_TRUE(eval(eout) == make_cow<arr::zts>(false, z));
}

TEST(itime_subset_zts_interval_1row) {
  auto eout = parse
    ("idx <- c(|.2015-03-09 06:38:01 America/New_York.|,|.2015-03-09 06:38:02 America/New_York.|,"
     "         |.2015-03-09 06:38:03 America/New_York.|);"
     "z   <- zts(idx, 1.0:6, dim=c(3, 2));"
     "ivl <- |-2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|;"
     "z[ivl,]\n");
  auto dt2 = tz::dtime_from_string("2015-03-09 06:38:02 America/New_York", tzones);
  auto z = arr::zts({1,2}, {dt2}, {2,5}, {{}, {}});
  ASSERT_TRUE(eval(eout) == make_cow<arr::zts>(false, z));
}

TEST(itime_subset_zts_interval_names_1row) {
  auto eout = parse
    ("idx <- c(|.2015-03-09 06:38:01 America/New_York.|,|.2015-03-09 06:38:02 America/New_York.|,"
     "         |.2015-03-09 06:38:03 America/New_York.|);"
     "z   <- zts(idx, 1.0:6, dim=c(3, 2), dimnames=list(NULL, c(\"one\", \"two\")));"
     "ivl <- |+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York-|;"
     "z[ivl,]\n");
  auto dt1 = tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones);
  auto z = arr::zts({1,2}, {dt1}, {1,4}, {{}, {"one", "two"}});
  ASSERT_TRUE(eval(eout) == make_cow<arr::zts>(false, z));
}


// subassign --------------------------------
// point in time indexing in array and zts
TEST(itime_subsassign_zts_dtime) {
  auto eout = parse
    ("idx <- c(|.2015-03-09 06:38:01 America/New_York.|,|.2015-03-09 06:38:02 America/New_York.|,"
     "         |.2015-03-09 06:38:03 America/New_York.|);"
     "z   <- zts(idx, 1.0:6, dim=c(3, 2), dimnames=list(NULL, c(\"one\", \"two\")));"
     "z[idx[c(1,2)],] <- c(11,12,14,15)\n");
  auto dt1 = tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones);
  auto dt2 = tz::dtime_from_string("2015-03-09 06:38:02 America/New_York", tzones);
  auto dt3 = tz::dtime_from_string("2015-03-09 06:38:03 America/New_York", tzones);
  auto z = arr::zts({3,2}, {dt1, dt2, dt3}, {11,12,3,14,15,6}, {{}, {"one", "two"}});
  ASSERT_TRUE(eval(eout) == make_cow<arr::zts>(false, z));
}

TEST(itime_scalar_subsassign_zts_dtime) {
  auto eout = parse
    ("idx <- c(|.2015-03-09 06:38:01 America/New_York.|,|.2015-03-09 06:38:02 America/New_York.|,"
     "         |.2015-03-09 06:38:03 America/New_York.|);"
     "z   <- zts(idx, 1.0:6, dim=c(3, 2), dimnames=list(NULL, c(\"one\", \"two\")));"
     "z[idx[c(1,2)],] <- 0\n");
  auto dt1 = tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones);
  auto dt2 = tz::dtime_from_string("2015-03-09 06:38:02 America/New_York", tzones);
  auto dt3 = tz::dtime_from_string("2015-03-09 06:38:03 America/New_York", tzones);
  auto z = arr::zts({3,2}, {dt1, dt2, dt3}, {0,0,3,0,0,6}, {{}, {"one", "two"}});
  ASSERT_TRUE(eval(eout) == make_cow<arr::zts>(false, z));
}


// interval subassign in arrays and zts
TEST(itime_subassign_zts_interval) {
  auto eout = parse
    ("idx <- c(|.2015-03-09 06:38:01 America/New_York.|,"
     "         |.2015-03-09 06:38:02 America/New_York.|,"
     "         |.2015-03-09 06:38:03 America/New_York.|);"
     "z   <- zts(idx, 1.0:6, dim=c(3, 2), dimnames=list(NULL, c(\"one\", \"two\")));"
     "ivl <- |+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|;"
     "z[ivl,] <- c(11,12,14,15)\n");
  auto dt1 = tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones);
  auto dt2 = tz::dtime_from_string("2015-03-09 06:38:02 America/New_York", tzones);
  auto dt3 = tz::dtime_from_string("2015-03-09 06:38:03 America/New_York", tzones);
  auto z = arr::zts({3,2}, {dt1, dt2, dt3}, {11,12,3,14,15,6}, {{}, {"one", "two"}});

  cout << "z.getIndex().isOrdered(): " << z.getIndex().isOrdered() << endl;

  ASSERT_TRUE(eval(eout) == make_cow<arr::zts>(false, z));
}

TEST(itime_scalar_subassign_zts_interval) {
  auto eout = parse
    ("idx <- c(|.2015-03-09 06:38:01 America/New_York.|,|.2015-03-09 06:38:02 America/New_York.|,"
     "         |.2015-03-09 06:38:03 America/New_York.|);"
     "z   <- zts(idx, 1.0:6, dim=c(3, 2), dimnames=list(NULL, c(\"one\", \"two\")));"
     "ivl <- |+2015-03-09 06:38:01 America/New_York -> 2015-03-09 06:38:02 America/New_York+|;"
     "z[ivl,] <- 0\n");
  auto dt1 = tz::dtime_from_string("2015-03-09 06:38:01 America/New_York", tzones);
  auto dt2 = tz::dtime_from_string("2015-03-09 06:38:02 America/New_York", tzones);
  auto dt3 = tz::dtime_from_string("2015-03-09 06:38:03 America/New_York", tzones);
  auto z = arr::zts({3,2}, {dt1, dt2, dt3}, {0,0,3,0,0,6}, {{}, {"one", "two"}});
  ASSERT_TRUE(eval(eout) == make_cow<arr::zts>(false, z));
}


// do time conversions LLL

int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
