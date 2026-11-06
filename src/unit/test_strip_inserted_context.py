"""Unit tests for dakota.tools._strip stripping logic."""

import unittest
from dakota.tools._strip import _load_rules, apply


class StripAlwaysFieldsTest(unittest.TestCase):
    """Fields in always_strip are unconditionally removed."""

    def _make_normal(self, **overrides):
        block = {
            "count": 2,
            "means": [1.0, 2.0],
            "std_deviations": [0.5, 0.5],
            "lower_bounds": [0.0, 1.0],
            "upper_bounds": [2.0, 3.0],
            "initial_point": [1.0, 2.0],
            "initial_point_user_provided": True,
            "inferred_lower_bounds": [0.0, 1.0],
            "inferred_upper_bounds": [2.0, 3.0],
        }
        block.update(overrides)
        return block

    def _data(self, block):
        return {"variables": [{"normal_uncertain": block}]}

    def test_always_strips_internal_flag(self):
        block = self._make_normal()
        apply(self._data(block), _load_rules())
        self.assertNotIn("initial_point_user_provided", block)

    def test_always_strips_inferred_bounds(self):
        block = self._make_normal()
        apply(self._data(block), _load_rules())
        self.assertNotIn("inferred_lower_bounds", block)
        self.assertNotIn("inferred_upper_bounds", block)

    def test_strips_purely_computed_bounds(self):
        block = {
            "count": 1,
            "betas": [2.0],
            "initial_point": [2.0],
            "initial_point_user_provided": True,
            "lower_bounds": [0.0],
            "upper_bounds": [8.0],
        }
        data = {"variables": [{"exponential_uncertain": block}]}
        apply(data, _load_rules())
        self.assertNotIn("lower_bounds", block)
        self.assertNotIn("upper_bounds", block)


class StripIfFlagFalseTest(unittest.TestCase):
    """initial_point is stripped when initial_point_user_provided is False."""

    def test_strips_defaulted_initial_point(self):
        block = {
            "count": 1,
            "means": [1.0],
            "std_deviations": [0.5],
            "lower_bounds": [0.0],
            "upper_bounds": [2.0],
            "initial_point": [1.0],
            "initial_point_user_provided": False,
            "inferred_lower_bounds": [0.0],
            "inferred_upper_bounds": [2.0],
        }
        data = {"variables": [{"normal_uncertain": block}]}
        apply(data, _load_rules())
        self.assertNotIn("initial_point", block)

    def test_keeps_user_provided_initial_point(self):
        block = {
            "count": 1,
            "means": [1.0],
            "std_deviations": [0.5],
            "lower_bounds": [0.0],
            "upper_bounds": [2.0],
            "initial_point": [0.5],
            "initial_point_user_provided": True,
            "inferred_lower_bounds": [0.0],
            "inferred_upper_bounds": [2.0],
        }
        data = {"variables": [{"normal_uncertain": block}]}
        apply(data, _load_rules())
        self.assertIn("initial_point", block)
        self.assertEqual(block["initial_point"], [0.5])


class StripIfInferredDiffersTest(unittest.TestCase):
    """ordinary bounds stripped when they differ from inferred (i.e. were defaulted)."""

    def test_keeps_user_bounds_when_inferred_equal(self):
        block = {
            "count": 1,
            "means": [1.0],
            "std_deviations": [0.5],
            "lower_bounds": [-1.0],
            "upper_bounds": [3.0],
            "initial_point": [1.0],
            "initial_point_user_provided": True,
            "inferred_lower_bounds": [-1.0],
            "inferred_upper_bounds": [3.0],
        }
        data = {"variables": [{"normal_uncertain": block}]}
        apply(data, _load_rules())
        self.assertIn("lower_bounds", block)
        self.assertIn("upper_bounds", block)
        self.assertEqual(block["lower_bounds"], [-1.0])
        self.assertEqual(block["upper_bounds"], [3.0])

    def test_strips_both_bounds_when_defaulted(self):
        """Inferred bounds differ from ordinary → user omitted bounds → strip both."""
        block = {
            "count": 1,
            "means": [1.0],
            "std_deviations": [0.5],
            "lower_bounds": [float("-inf")],
            "upper_bounds": [float("inf")],
            "initial_point": [1.0],
            "initial_point_user_provided": False,
            "inferred_lower_bounds": [-0.5],
            "inferred_upper_bounds": [2.5],
        }
        data = {"variables": [{"normal_uncertain": block}]}
        apply(data, _load_rules())
        self.assertNotIn("lower_bounds", block)
        self.assertNotIn("upper_bounds", block)


class NestedBlockTest(unittest.TestCase):
    """Nested variable blocks (e.g. discrete_design_set.integer) are handled."""

    def test_strips_computed_bounds_in_nested_block(self):
        block = {
            "count": 2,
            "elements": [1, 2, 3, 4],
            "initial_point": [1, 3],
            "lower_bounds": [1, 3],
            "upper_bounds": [2, 4],
        }
        data = {"variables": [{"discrete_design_set": {"integer": block}}]}
        apply(data, _load_rules())
        self.assertNotIn("lower_bounds", block)
        self.assertNotIn("upper_bounds", block)
        self.assertIn("initial_point", block)

    def test_strips_initial_point_when_not_user_provided_in_nested_block(self):
        block = {
            "count": 1,
            "elements": ["a", "b"],
            "initial_point": ["a"],
            "initial_point_user_provided": False,
            "lower_bounds": ["a"],
            "upper_bounds": ["b"],
        }
        data = {"variables": [{"histogram_point_uncertain": {"string": block}}]}
        apply(data, _load_rules())
        self.assertNotIn("initial_point", block)
        self.assertNotIn("initial_point_user_provided", block)


class MultipleVariablesBlocksTest(unittest.TestCase):
    """Rules apply to every VariablesConfig object in the variables list."""

    def test_strips_across_multiple_variable_blocks(self):
        block_a = {
            "count": 1,
            "means": [0.0],
            "std_deviations": [1.0],
            "initial_point": [0.0],
            "initial_point_user_provided": False,
            "inferred_lower_bounds": [-3.0],
            "inferred_upper_bounds": [3.0],
        }
        block_b = {
            "count": 1,
            "betas": [1.0],
            "initial_point": [1.0],
            "initial_point_user_provided": True,
            "lower_bounds": [0.0],
            "upper_bounds": [4.0],
        }
        data = {
            "variables": [
                {"normal_uncertain": block_a},
                {"exponential_uncertain": block_b},
            ]
        }
        apply(data, _load_rules())
        self.assertNotIn("initial_point_user_provided", block_a)
        self.assertNotIn("initial_point", block_a)
        self.assertNotIn("lower_bounds", block_b)
        self.assertNotIn("upper_bounds", block_b)
        self.assertIn("initial_point", block_b)


if __name__ == "__main__":
    unittest.main()
