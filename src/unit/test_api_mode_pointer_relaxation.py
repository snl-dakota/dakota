#  _______________________________________________________________________
#
#  Dakota: Explore and predict with confidence.
#  Copyright 2014-2025
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

"""Tests for API-mode pointer relaxation (plan: api-mode-pointer-relaxation.md).

These tests verify the sentinel-injection behaviour introduced in
``DakotaBaseModel._fill_api_mode_pointer_sentinels`` together with the
``input_file_mode()`` context manager and the ``POINTER_SENTINEL`` constant.

Coverage:
  * API mode (default) -- required str pointer fields receive the sentinel
  * API mode (default) -- required list[str] pointer fields receive [sentinel]
  * User-supplied values are never overwritten by the sentinel
  * ``input_file_mode()`` re-enables the Pydantic required-field check
  * Sibling (non-pointer) config fields still work in API mode
  * Optional pointer fields are left untouched in API mode
"""

from __future__ import annotations

import unittest

from pydantic import ValidationError

from dakota.spec.base import POINTER_SENTINEL, input_file_mode
from dakota.spec.model import (
    GlobalApproxTruthModelPointer,
    OrderedModelFidelitiesConfig,
    SubMethodPointer,
    ActiveSubspaceConfig,
    BuildSourceDaceMethodPointer,
)


# ---------------------------------------------------------------------------
# 1. API mode (default): str pointer field gets the sentinel
# ---------------------------------------------------------------------------

class TestApiModeStrPointerSentinel(unittest.TestCase):
    def test_missing_required_str_pointer_gets_sentinel(self):
        """Omitting a required str pointer field in API mode should inject the sentinel."""
        obj = GlobalApproxTruthModelPointer()
        self.assertEqual(obj.truth_model_pointer, POINTER_SENTINEL)

    def test_sentinel_is_correct_string_value(self):
        self.assertEqual(POINTER_SENTINEL, "__dakota_api_unused_pointer__")

    def test_single_field_class_build_source_dace(self):
        """BuildSourceDaceMethodPointer has a single required pointer field."""
        obj = BuildSourceDaceMethodPointer()
        self.assertEqual(obj.dace_method_pointer, POINTER_SENTINEL)


# ---------------------------------------------------------------------------
# 2. API mode (default): list[str] pointer field gets [sentinel]
# ---------------------------------------------------------------------------

class TestApiModeListPointerSentinel(unittest.TestCase):
    def test_missing_required_list_pointer_gets_list_sentinel(self):
        """OrderedModelFidelitiesConfig.pointers is list[str]; sentinel must be a list."""
        obj = OrderedModelFidelitiesConfig()
        self.assertEqual(obj.pointers, [POINTER_SENTINEL])

    def test_list_sentinel_is_a_list(self):
        obj = OrderedModelFidelitiesConfig()
        self.assertIsInstance(obj.pointers, list)
        self.assertEqual(len(obj.pointers), 1)


# ---------------------------------------------------------------------------
# 3. User-supplied values are never overwritten
# ---------------------------------------------------------------------------

class TestUserSuppliedValuePreserved(unittest.TestCase):
    def test_user_str_pointer_not_overwritten(self):
        obj = GlobalApproxTruthModelPointer(truth_model_pointer="my_model")
        self.assertEqual(obj.truth_model_pointer, "my_model")

    def test_user_list_pointer_not_overwritten(self):
        obj = OrderedModelFidelitiesConfig(pointers=["hi_fi", "lo_fi"])
        self.assertEqual(obj.pointers, ["hi_fi", "lo_fi"])

    def test_user_pointer_is_not_sentinel(self):
        obj = GlobalApproxTruthModelPointer(truth_model_pointer="real_model")
        self.assertNotEqual(obj.truth_model_pointer, POINTER_SENTINEL)


# ---------------------------------------------------------------------------
# 4. input_file_mode() re-enables required-field enforcement
# ---------------------------------------------------------------------------

class TestInputFileModeEnforcesRequired(unittest.TestCase):
    def test_missing_pointer_raises_in_input_file_mode(self):
        with input_file_mode():
            with self.assertRaises(ValidationError):
                GlobalApproxTruthModelPointer()

    def test_missing_list_pointer_raises_in_input_file_mode(self):
        with input_file_mode():
            with self.assertRaises(ValidationError):
                OrderedModelFidelitiesConfig()

    def test_provided_pointer_succeeds_in_input_file_mode(self):
        with input_file_mode():
            obj = GlobalApproxTruthModelPointer(truth_model_pointer="my_model")
        self.assertEqual(obj.truth_model_pointer, "my_model")

    def test_input_file_mode_is_context_local(self):
        """After exiting input_file_mode(), API mode is restored."""
        with input_file_mode():
            pass  # just enter and exit
        # Must succeed again outside the context
        obj = GlobalApproxTruthModelPointer()
        self.assertEqual(obj.truth_model_pointer, POINTER_SENTINEL)

    def test_input_file_mode_nested_reset(self):
        """Nested context managers correctly restore the outer mode."""
        with input_file_mode():
            with self.assertRaises(ValidationError):
                GlobalApproxTruthModelPointer()
        # After exiting, back to API mode
        obj = GlobalApproxTruthModelPointer()
        self.assertEqual(obj.truth_model_pointer, POINTER_SENTINEL)


# ---------------------------------------------------------------------------
# 5. Sibling (non-pointer) config fields still work in API mode
# ---------------------------------------------------------------------------

class TestSiblingConfigFieldsInApiMode(unittest.TestCase):
    def test_submethod_pointer_with_sibling_field(self):
        """SubMethodPointer has a required pointer + optional iterator_servers sibling."""
        obj = SubMethodPointer(iterator_servers=4)
        self.assertEqual(obj.pointer, POINTER_SENTINEL)
        self.assertEqual(obj.iterator_servers, 4)

    def test_active_subspace_with_sibling_fields(self):
        """ActiveSubspaceConfig has a required pointer + many optional config fields."""
        obj = ActiveSubspaceConfig(initial_samples=50)
        self.assertEqual(obj.truth_model_pointer, POINTER_SENTINEL)
        self.assertEqual(obj.initial_samples, 50)


# ---------------------------------------------------------------------------
# 6. Optional pointer fields are left untouched
# ---------------------------------------------------------------------------

class TestOptionalPointerFieldsUntouched(unittest.TestCase):
    def test_optional_pointer_absent_stays_non_sentinel(self):
        """Optional pointer fields must not be filled with the sentinel.

        SubMethodPointer.optional_interface_responses_pointer (if it exists) is
        an *optional* pointer and must remain None, not the sentinel.
        """
        obj = SubMethodPointer()
        # The required 'pointer' field gets the sentinel
        self.assertEqual(obj.pointer, POINTER_SENTINEL)
        # Any optional pointer-like field must NOT be the sentinel
        if hasattr(obj, "optional_interface_responses_pointer"):
            val = obj.optional_interface_responses_pointer
            self.assertTrue(val is None or val != POINTER_SENTINEL)


if __name__ == "__main__":
    unittest.main()
