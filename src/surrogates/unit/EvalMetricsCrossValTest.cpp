/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifdef HAVE_ROL
#include "SurrogatesGaussianProcess.hpp"
#endif
#include "SurrogatesPolynomialRegression.hpp"

#define BOOST_TEST_MODULE surrogates_EvalMetricsCrossValTest
#include <boost/test/included/unit_test.hpp>

using namespace dakota;
using namespace dakota::util;
using namespace dakota::surrogates;

BOOST_AUTO_TEST_CASE(test_surrogates_eval_metrics_and_cross_validation) {
  /* num_samples x num_features */
  MatrixXd xs_u(7, 1);
  /* num_samples x num_qoi */
  MatrixXd response(7, 1);
  /* num_eval_pts x num_features */
  MatrixXd eval_pts(6, 1);
  /* num_eval_pts x num_qoi */
  MatrixXd truth_pred(6, 1);

  /* samples */
  xs_u << 0.05536604, 0.28730518, 0.30391231, 0.40768703, 0.45035059,
      0.52639952, 0.78853488;

  response << -0.15149429, -0.19689361, -0.17323105, -0.02379026, 0.02013445,
      0.05011702, -0.11678312;

  eval_pts << 0.0, 0.2, 0.4, 0.6, 0.8, 1.0;

  truth_pred << 0.0, -0.28178127, -0.03336742, 0.0192193, -0.11554629,
      0.30685282;

  ParameterList poly_param_list("Polynomial Test Parameters");
  poly_param_list.set("max degree", 3);
  PolynomialRegression poly(xs_u, response, poly_param_list);
  VectorXd pred_vals_poly;
  pred_vals_poly = poly.value(eval_pts);

  /* Evaluate metrics */
  StringArray metrics_names = {"sum_squared",       "mean_squared",
                               "root_mean_squared", "sum_abs",
                               "mean_abs",          "max_abs"};
  //"rsquared"};
  //"ape, "mape", "rsquared"};

  const double metrics_difftol = 1.0e-4;
  double metrics_diff;

  VectorXd poly_gold_mvals(metrics_names.size());
  poly_gold_mvals << 2.41469, 0.402449, 0.634389, 1.77595, 0.295992, 1.54981;

  VectorXd poly_mvals =
      poly.evaluate_metrics(metrics_names, eval_pts, truth_pred);
  metrics_diff = (poly_mvals - poly_gold_mvals).norm();
  BOOST_CHECK(metrics_diff < metrics_difftol);

  std::cout << "\n\nCubic polynomial metrics:\n";
  for (int m = 0; m < metrics_names.size(); m++) {
    std::cout << "  " << metrics_names[m] << " = " << poly_mvals(m) << "\n";
  }
  std::cout << "\n";

#ifdef HAVE_ROL
  ParameterList gp_param_list("GP Test Parameters");
  gp_param_list.set("scaler name", "standardization");
  gp_param_list.set("standardize response", false);
  gp_param_list.set("num restarts", 10);
  gp_param_list.sublist("Nugget").set("fixed nugget", 1.0e-12);
  gp_param_list.set("gp seed", 42);
  GaussianProcess gp(xs_u, response, gp_param_list);

  VectorXd gp_gold_mvals(metrics_names.size());
  gp_gold_mvals << 0.129966, 0.021661, 0.147177, 0.410457, 0.0684095, 0.357531;

  VectorXd gp_mvals = gp.evaluate_metrics(metrics_names, eval_pts, truth_pred);
  metrics_diff = (gp_mvals - gp_gold_mvals).norm();
  BOOST_CHECK(metrics_diff < metrics_difftol);

  std::cout << "\n\nGP metrics:\n";
  for (int m = 0; m < metrics_names.size(); m++) {
    std::cout << "  " << metrics_names[m] << " = " << gp_mvals(m) << "\n";
  }
  std::cout << "\n";
#endif
}

BOOST_AUTO_TEST_CASE(test_surrogates_cross_validate) {
  /* Cross-validation with the polynomial */
  const double cv_norm_difftol = 1.0e-5;
  double cv_diff;
  VectorXd gold_poly_cv_metrics(2);
  gold_poly_cv_metrics << 0.0193731, 0.110726;
  const int cv_seed = 33;
  const int num_folds = 3;

  /* Uniform random points in (0,1) */
  /* True function = 0.4*x**2 + x */
  VectorXd build_pts(14);
  VectorXd target(14);

  build_pts << 0.37454012, 0.95071431, 0.73199394, 0.59865848, 0.15601864,
      0.15599452, 0.05808361, 0.86617615, 0.60111501, 0.70807258, 0.02058449,
      0.96990985, 0.83244264, 0.21233911;

  target << 0.38431047, 1.26568441, 0.97051622, 0.55068725, -0.00673642,
      0.10949948, -0.04185002, 1.19770533, 0.65484831, 0.76738892, 0.16731886,
      1.32362227, 1.11637976, 0.08789945;

  ParameterList line_poly_pl("Line Test Parameters");
  line_poly_pl.set("max degree", 1);
  PolynomialRegression line_poly(line_poly_pl);

  StringArray metrics_names = {"mean_squared", "mean_abs"};
  VectorXd cross_val_metrics;
  cross_val_metrics = line_poly.cross_validate(build_pts, target, metrics_names,
                                               num_folds, cv_seed);

  std::cout << "\nlinear polynomial cross validation scores: "
            << cross_val_metrics.transpose() << "\n";

  cv_diff = (cross_val_metrics - gold_poly_cv_metrics).norm();
  BOOST_CHECK(cv_diff < cv_norm_difftol);

#ifdef HAVE_ROL
  /* Cross-validation with the GP */
  VectorXd gold_gp_cv_metrics(2);
  gold_gp_cv_metrics << 0.0169657, 0.113947;

  ParameterList gp_opts;
  VectorXd nugget_bounds(2);
  nugget_bounds(0) = 1.0e-4;
  nugget_bounds(1) = 0.316;
  gp_opts.set("scaler name", "none");
  gp_opts.set("standardize response", false);
  gp_opts.sublist("Nugget").set("fixed nugget", 0.0);
  gp_opts.set("gp seed", 42);
  gp_opts.sublist("Nugget").set("estimate nugget", true);
  gp_opts.sublist("Nugget").sublist("Bounds").set("lower bound",
                                                  nugget_bounds(0));
  gp_opts.sublist("Nugget").sublist("Bounds").set("upper bound",
                                                  nugget_bounds(1));
  gp_opts.set("num restarts", 20);
  GaussianProcess gp_cv(gp_opts);

  cross_val_metrics = gp_cv.cross_validate(build_pts, target, metrics_names,
                                           num_folds, cv_seed);

  std::cout << "\nGaussian process cross validation scores: "
            << cross_val_metrics.transpose() << "\n\n";

  cv_diff = (cross_val_metrics - gold_gp_cv_metrics).norm();
  BOOST_CHECK(cv_diff < cv_norm_difftol);
#endif
}
