"""Check the numerical fixture and Dakota's standard result ordering."""

from pathlib import Path
import tempfile
import unittest

from response_driver import evaluate, run


class ResponseDriverTests(unittest.TestCase):
    def test_derivatives(self):
        for mode, point in [
            ("high", [0.3, -0.2]),
            ("low", [0.3, -0.2]),
            ("optimization", [0.9, 1.1, 0.4]),
            ("richardson", [8.0]),
        ]:
            value, gradient, hessian = evaluate(mode, point)[0]
            for i in range(len(point)):
                plus, minus = point.copy(), point.copy()
                plus[i] += 1.0e-5
                minus[i] -= 1.0e-5
                pvalue, pgradient, _ = evaluate(mode, plus)[0]
                mvalue, mgradient, _ = evaluate(mode, minus)[0]
                self.assertAlmostEqual(
                    gradient[i], (pvalue - mvalue) / 2.0e-5, places=7
                )
                for j in range(len(point)):
                    self.assertAlmostEqual(
                        hessian[j][i], (pgradient[j] - mgradient[j]) / 2.0e-5, places=7
                    )
        self.assertAlmostEqual(
            evaluate("richardson", [8.0])[0][0] - 2.0,
            4 * (evaluate("richardson", [16.0])[0][0] - 2.0),
        )

    def test_standard_result_order_and_derivative_subset(self):
        with tempfile.TemporaryDirectory() as work:
            parameters = Path(work) / "params.in"
            results = Path(work) / "results.out"
            parameters.write_text(
                "2 variables\n0.9 x1\n1.1 x2\n"
                "2 functions\n3 ASV_1\n7 ASV_2\n"
                "1 derivative_variables\n2 DVV_1\n0 analysis_components\n"
            )
            run("least_squares", parameters, results)
            lines = results.read_text().splitlines()
            self.assertAlmostEqual(float(lines[0]), 0.1)
            self.assertAlmostEqual(float(lines[1]), -0.1)
            self.assertEqual(lines[2:], ["[ 0 ]", "[ 1 ]", "[[ 0 ]]"])


if __name__ == "__main__":
    unittest.main()
