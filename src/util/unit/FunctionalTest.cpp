/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "functional.hpp"

// Functor

struct test_functor
{
  int operator()(int, double);
};

struct test_functor_n
{
  int operator()(int, double) noexcept;
};

struct test_functor_l
{
  int operator()(int, double) &;
};

struct test_functor_ln
{
  int operator()(int, double) & noexcept;
};

struct test_functor_r
{
  int operator()(int, double) &&;
};

struct test_functor_rn
{
  int operator()(int, double) && noexcept;
};

struct test_functor_c
{
  int operator()(int, double);
};

struct test_functor_cn
{
  int operator()(int, double) noexcept;
};

struct test_functor_cl
{
  int operator()(int, double) &;
};

struct test_functor_cln
{
  int operator()(int, double) & noexcept;
};

struct test_functor_cr
{
  int operator()(int, double) &&;
};

struct test_functor_crn
{
  int operator()(int, double) && noexcept;
};

static_assert(std::is_same_v<Dakota::Util::FunctionInfo<test_functor>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<test_functor>::arg_tuple, std::tuple<int, double>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<test_functor_n>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<test_functor_n>::arg_tuple, std::tuple<int, double>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<test_functor_l>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<test_functor_l>::arg_tuple, std::tuple<int, double>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<test_functor_ln>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<test_functor_ln>::arg_tuple, std::tuple<int, double>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<test_functor_r>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<test_functor_r>::arg_tuple, std::tuple<int, double>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<test_functor_rn>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<test_functor_rn>::arg_tuple, std::tuple<int, double>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<test_functor_c>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<test_functor_c>::arg_tuple, std::tuple<int, double>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<test_functor_cn>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<test_functor_cn>::arg_tuple, std::tuple<int, double>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<test_functor_cl>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<test_functor_cl>::arg_tuple, std::tuple<int, double>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<test_functor_cln>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<test_functor_cln>::arg_tuple, std::tuple<int, double>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<test_functor_cr>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<test_functor_cr>::arg_tuple, std::tuple<int, double>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<test_functor_crn>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<test_functor_crn>::arg_tuple, std::tuple<int, double>>);


// Free functions

static_assert(std::is_same_v<Dakota::Util::FunctionInfo<void()>::return_type, void>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<void()>::arg_tuple, std::tuple<>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<void(int, double)>::return_type, void>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<void(int, double)>::arg_tuple, std::tuple<int, double>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<int()>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<int()>::arg_tuple, std::tuple<>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<int(int, double)>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<int(int, double)>::arg_tuple, std::tuple<int, double>>);

// Noexcept free functions

static_assert(std::is_same_v<Dakota::Util::FunctionInfo<void() noexcept>::return_type, void>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<void() noexcept>::arg_tuple, std::tuple<>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<void(int, double) noexcept>::return_type, void>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<void(int, double) noexcept>::arg_tuple, std::tuple<int, double>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<int() noexcept>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<int() noexcept>::arg_tuple, std::tuple<>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<int(int, double) noexcept>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<int(int, double) noexcept>::arg_tuple, std::tuple<int, double>>);

// Function pointers

static_assert(std::is_same_v<Dakota::Util::FunctionInfo<void (*)()>::return_type, void>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<void (*)()>::arg_tuple, std::tuple<>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<void (*)(int, double)>::return_type, void>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<void (*)(int, double)>::arg_tuple, std::tuple<int, double>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<int (*)()>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<int (*)()>::arg_tuple, std::tuple<>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<int (*)(int, double)>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<int (*)(int, double)>::arg_tuple, std::tuple<int, double>>);

// Noexcept function pointers

static_assert(std::is_same_v<Dakota::Util::FunctionInfo<void (*)() noexcept>::return_type, void>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<void (*)() noexcept>::arg_tuple, std::tuple<>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<void (*)(int, double) noexcept>::return_type, void>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<void (*)(int, double) noexcept>::arg_tuple, std::tuple<int, double>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<int (*)() noexcept>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<int (*)() noexcept>::arg_tuple, std::tuple<>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<int (*)(int, double) noexcept>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<int (*)(int, double) noexcept>::arg_tuple, std::tuple<int, double>>);

// Member function pointers

struct test_obj
{
  void f1();
  void f2(int, double);
  int f3();
  int f4(int, double);

  void f1_n() noexcept;
  void f2_n(int, double) noexcept;
  int f3_n() noexcept;
  int f4_n(int, double) noexcept;

  void f1_l() &;
  void f2_l(int, double) &;
  int f3_l() &;
  int f4_l(int, double) &;

  void f1_ln() & noexcept;
  void f2_ln(int, double) & noexcept;
  int f3_ln() & noexcept;
  int f4_ln(int, double) & noexcept;

  void f1_r() &&;
  void f2_r(int, double) &&;
  int f3_r() &&;
  int f4_r(int, double) &&;

  void f1_rn() && noexcept;
  void f2_rn(int, double) && noexcept;
  int f3_rn() && noexcept;
  int f4_rn(int, double) && noexcept;

  void f1_c() const;
  void f2_c(int, double) const;
  int f3_c() const;
  int f4_c(int, double) const;

  void f1_cn() const noexcept;
  void f2_cn(int, double) const noexcept;
  int f3_cn() const noexcept;
  int f4_cn(int, double) const noexcept;

  void f1_cl() const &;
  void f2_cl(int, double) const &;
  int f3_cl() const &;
  int f4_cl(int, double) const &;

  void f1_cln() const & noexcept;
  void f2_cln(int, double) const & noexcept;
  int f3_cln() const & noexcept;
  int f4_cln(int, double) const & noexcept;

  void f1_cr() const &&;
  void f2_cr(int, double) const &&;
  int f3_cr() const &&;
  int f4_cr(int, double) const &&;

  void f1_crn() const && noexcept;
  void f2_crn(int, double) const && noexcept;
  int f3_crn() const && noexcept;
  int f4_crn(int, double) const && noexcept;
};

static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f1)>::return_type, void>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f1)>::arg_tuple, std::tuple<>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f2)>::return_type, void>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f2)>::arg_tuple, std::tuple<int, double>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f3)>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f3)>::arg_tuple, std::tuple<>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f4)>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f4)>::arg_tuple, std::tuple<int, double>>);

static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f1_n)>::return_type, void>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f1_n)>::arg_tuple, std::tuple<>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f2_n)>::return_type, void>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f2_n)>::arg_tuple, std::tuple<int, double>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f3_n)>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f3_n)>::arg_tuple, std::tuple<>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f4_n)>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f4_n)>::arg_tuple, std::tuple<int, double>>);

static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f1_l)>::return_type, void>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f1_l)>::arg_tuple, std::tuple<>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f2_l)>::return_type, void>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f2_l)>::arg_tuple, std::tuple<int, double>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f3_l)>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f3_l)>::arg_tuple, std::tuple<>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f4_l)>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f4_l)>::arg_tuple, std::tuple<int, double>>);

static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f1_ln)>::return_type, void>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f1_ln)>::arg_tuple, std::tuple<>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f2_ln)>::return_type, void>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f2_ln)>::arg_tuple, std::tuple<int, double>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f3_ln)>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f3_ln)>::arg_tuple, std::tuple<>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f4_ln)>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f4_ln)>::arg_tuple, std::tuple<int, double>>);

static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f1_r)>::return_type, void>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f1_r)>::arg_tuple, std::tuple<>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f2_r)>::return_type, void>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f2_r)>::arg_tuple, std::tuple<int, double>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f3_r)>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f3_r)>::arg_tuple, std::tuple<>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f4_r)>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f4_r)>::arg_tuple, std::tuple<int, double>>);

static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f1_rn)>::return_type, void>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f1_rn)>::arg_tuple, std::tuple<>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f2_rn)>::return_type, void>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f2_rn)>::arg_tuple, std::tuple<int, double>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f3_rn)>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f3_rn)>::arg_tuple, std::tuple<>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f4_rn)>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f4_rn)>::arg_tuple, std::tuple<int, double>>);

static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f1_c)>::return_type, void>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f1_c)>::arg_tuple, std::tuple<>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f2_c)>::return_type, void>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f2_c)>::arg_tuple, std::tuple<int, double>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f3_c)>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f3_c)>::arg_tuple, std::tuple<>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f4_c)>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f4_c)>::arg_tuple, std::tuple<int, double>>);

static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f1_cn)>::return_type, void>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f1_cn)>::arg_tuple, std::tuple<>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f2_cn)>::return_type, void>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f2_cn)>::arg_tuple, std::tuple<int, double>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f3_cn)>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f3_cn)>::arg_tuple, std::tuple<>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f4_cn)>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f4_cn)>::arg_tuple, std::tuple<int, double>>);

static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f1_cl)>::return_type, void>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f1_cl)>::arg_tuple, std::tuple<>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f2_cl)>::return_type, void>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f2_cl)>::arg_tuple, std::tuple<int, double>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f3_cl)>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f3_cl)>::arg_tuple, std::tuple<>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f4_cl)>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f4_cl)>::arg_tuple, std::tuple<int, double>>);

static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f1_cln)>::return_type, void>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f1_cln)>::arg_tuple, std::tuple<>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f2_cln)>::return_type, void>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f2_cln)>::arg_tuple, std::tuple<int, double>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f3_cln)>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f3_cln)>::arg_tuple, std::tuple<>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f4_cln)>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f4_cln)>::arg_tuple, std::tuple<int, double>>);

static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f1_cr)>::return_type, void>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f1_cr)>::arg_tuple, std::tuple<>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f2_cr)>::return_type, void>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f2_cr)>::arg_tuple, std::tuple<int, double>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f3_cr)>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f3_cr)>::arg_tuple, std::tuple<>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f4_cr)>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f4_cr)>::arg_tuple, std::tuple<int, double>>);

static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f1_crn)>::return_type, void>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f1_crn)>::arg_tuple, std::tuple<>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f2_crn)>::return_type, void>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f2_crn)>::arg_tuple, std::tuple<int, double>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f3_crn)>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f3_crn)>::arg_tuple, std::tuple<>>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f4_crn)>::return_type, int>);
static_assert(std::is_same_v<Dakota::Util::FunctionInfo<decltype(&test_obj::f4_crn)>::arg_tuple, std::tuple<int, double>>);

