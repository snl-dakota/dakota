#  _______________________________________________________________________
#
#  Dakota: Explore and predict with confidence.
#  Copyright 2014-2025
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

"""Tests for pointer-group and pointer-union API-mode sentinel injection.

Verifies the sentinel-injection behaviour introduced in
``DakotaBaseModel._fill_api_mode_pointer_sentinels`` for the two
higher-level structural patterns:

* **pointer-group** (``x-pointer-group``): a required model-typed field
  whose wrapped config contains required block-pointer fields plus optional
  configuration siblings.  In API mode, if absent, injected as ``{}``; the
  child model fills its own pointer sentinel.

* **pointer-union** (``x-pointer-union``): a required ``Union`` field where
  at least one branch is a pointer-only wrapper.  In API mode, if absent,
  the first all-pointer branch is selected automatically and the pointer
  sentinel is injected into it.

Coverage:
  1.  Plain block_pointer sentinel -- existing behaviour unchanged.
  2.  Pointer-group absent in API mode -- sentinel injected into child.
  3.  Pointer-group present with optional config -- config preserved.
  4.  Pointer-group absent in input_file_mode -- ValidationError raised.
  5.  Pointer-union absent in API mode -- all-pointer branch auto-selected.
  6.  Pointer-union: user supplies name branch -- name branch preserved.
  7.  Pointer-union absent in input_file_mode -- ValidationError raised.
  8.  Pointer-union where every branch is all-pointer (Ensemble).
  9.  Defensive check -- pointer_union with no qualifying branch raises.
  10. Pre-validated pointer-group child instance accepted directly.
"""

from __future__ import annotations

import unittest
from typing import Union

from pydantic import ValidationError

from dakota.spec.base import (
    DakotaBaseModel,
    DakotaField,
    POINTER_SENTINEL,
    input_file_mode,
)
from dakota.spec.model import (
    ActiveSubspaceSelection,
    Ensemble,
    NestedConfig,
    RandomFieldSelection,
    SubMethodPointer,
    TruthModelPointerConfig,
)
from dakota.spec.method.branch_and_bound import BranchAndBoundConfig
from dakota.spec.method.hybrid import EmbeddedConfig
from dakota.spec.method.multi_start import MultiStartConfig
from dakota.spec.method.pareto_set import ParetoSetConfig
from dakota.spec.method.surrogate_based_local import (
    SurrogateBasedLocalConfig,
    SurrogateBasedLocalSelection,
)
from dakota.spec.method.surrogate_based_global import SurrogateBasedGlobalSelection


# ---------------------------------------------------------------------------
# 1. Plain block_pointer sentinel (existing behaviour)
# ---------------------------------------------------------------------------

class TestPlainBlockPointer(unittest.TestCase):
    """Plain x-block-pointer fields still receive the sentinel in API mode."""

    def test_api_mode_injects_sentinel(self):
        """Omitting a required block_pointer field in API mode injects POINTER_SENTINEL."""
        # TruthModelPointerConfig has a single required str field 'pointer'
        # annotated with x-block-pointer.
        obj = TruthModelPointerConfig()
        self.assertEqual(obj.pointer, POINTER_SENTINEL)

    def test_input_file_mode_enforces_pointer(self):
        """Omitting a required block_pointer in input_file_mode raises ValidationError."""
        with input_file_mode():
            with self.assertRaises(ValidationError):
                TruthModelPointerConfig()


# ---------------------------------------------------------------------------
# 2. Pointer-group absent in API mode
# ---------------------------------------------------------------------------

class TestPointerGroupAbsentApiMode(unittest.TestCase):
    """Pointer-group fields absent in API mode are injected as {} so the child
    model fills its own pointer sentinel."""

    def test_nested_config_sentinel_injected(self):
        """NestedConfig.sub_method_pointer absent -> child pointer gets sentinel."""
        obj = NestedConfig()
        self.assertIsNotNone(obj.sub_method_pointer)
        self.assertEqual(obj.sub_method_pointer.pointer, POINTER_SENTINEL)

    def test_active_subspace_selection_fills(self):
        """ActiveSubspaceSelection.active_subspace absent -> child fills sentinel."""
        obj = ActiveSubspaceSelection()
        self.assertIsNotNone(obj.active_subspace)

    def test_random_field_selection_fills(self):
        """RandomFieldSelection.random_field absent -> child fills sentinel."""
        obj = RandomFieldSelection()
        self.assertIsNotNone(obj.random_field)

    def test_surrogate_based_local_selection_fills(self):
        """SurrogateBasedLocalSelection absent -> nested pointer_union also resolved."""
        obj = SurrogateBasedLocalSelection()
        self.assertIsNotNone(obj.surrogate_based_local)
        # The nested pointer_union sub_method must also be resolved.
        self.assertIsNotNone(obj.surrogate_based_local.sub_method)
        # model_pointer is a plain block_pointer on the config -- must be sentinel.
        self.assertEqual(obj.surrogate_based_local.model_pointer, POINTER_SENTINEL)

    def test_surrogate_based_global_selection_fills(self):
        """SurrogateBasedGlobalSelection absent -> child fills sentinel."""
        obj = SurrogateBasedGlobalSelection()
        self.assertIsNotNone(obj.surrogate_based_global)


# ---------------------------------------------------------------------------
# 3. Pointer-group present with optional config
# ---------------------------------------------------------------------------

class TestPointerGroupPresentWithConfig(unittest.TestCase):
    """When the pointer-group field IS provided by the user, optional sibling
    config is preserved and the pointer still gets the sentinel."""

    def test_iterator_servers_preserved(self):
        """sub_method_pointer dict with iterator_servers: sentinel + config preserved."""
        obj = NestedConfig(sub_method_pointer={"iterator_servers": 4})
        self.assertEqual(obj.sub_method_pointer.pointer, POINTER_SENTINEL)
        self.assertEqual(obj.sub_method_pointer.iterator_servers, 4)

    def test_explicit_pointer_preserved(self):
        """An explicit pointer value supplied by the user is not overwritten."""
        obj = NestedConfig(sub_method_pointer={"pointer": "my_method"})
        self.assertEqual(obj.sub_method_pointer.pointer, "my_method")
        self.assertNotEqual(obj.sub_method_pointer.pointer, POINTER_SENTINEL)


# ---------------------------------------------------------------------------
# 4. Pointer-group absent in input_file_mode
# ---------------------------------------------------------------------------

class TestPointerGroupInputFileMode(unittest.TestCase):
    """Pointer-group fields absent in input_file_mode are enforced normally."""

    def test_nested_config_raises(self):
        """NestedConfig without sub_method_pointer in input_file_mode -> ValidationError."""
        with input_file_mode():
            with self.assertRaises(ValidationError):
                NestedConfig()

    def test_active_subspace_raises(self):
        """ActiveSubspaceSelection without active_subspace -> ValidationError."""
        with input_file_mode():
            with self.assertRaises(ValidationError):
                ActiveSubspaceSelection()


# ---------------------------------------------------------------------------
# 5. Pointer-union absent in API mode
# ---------------------------------------------------------------------------

class TestPointerUnionAbsentApiMode(unittest.TestCase):
    """When a pointer_union field is absent in API mode the first all-pointer
    branch is selected and its pointer field receives the sentinel."""

    def test_embedded_config_sub_method(self):
        """EmbeddedConfig.sub_method absent -> GlobalMethodPointer branch selected."""
        obj = EmbeddedConfig()
        sm = obj.sub_method
        self.assertTrue(
            hasattr(sm, "global_method_pointer"),
            "Expected GlobalMethodPointer branch, got {}".format(type(sm).__name__),
        )
        self.assertEqual(sm.global_method_pointer, POINTER_SENTINEL)

    def test_embedded_config_local_sub_method(self):
        """EmbeddedConfig.local_sub_method absent -> LocalMethodPointer branch selected."""
        obj = EmbeddedConfig()
        lsm = obj.local_sub_method
        self.assertTrue(
            hasattr(lsm, "local_method_pointer"),
            "Expected LocalMethodPointer branch, got {}".format(type(lsm).__name__),
        )
        self.assertEqual(lsm.local_method_pointer, POINTER_SENTINEL)

    def test_pareto_set_config_sub_method(self):
        """ParetoSetConfig.sub_method absent -> ParetoSetMethodPointer branch selected."""
        obj = ParetoSetConfig()
        sm = obj.sub_method
        self.assertTrue(
            hasattr(sm, "method_pointer"),
            "Expected ParetoSetMethodPointer branch, got {}".format(type(sm).__name__),
        )
        self.assertEqual(sm.method_pointer, POINTER_SENTINEL)

    def test_multi_start_config_sub_method(self):
        """MultiStartConfig.sub_method absent -> MultiStartMethodPointer branch selected."""
        obj = MultiStartConfig()
        sm = obj.sub_method
        self.assertTrue(
            hasattr(sm, "method_pointer"),
            "Expected MultiStartMethodPointer branch, got {}".format(type(sm).__name__),
        )
        self.assertEqual(sm.method_pointer, POINTER_SENTINEL)

    def test_branch_and_bound_sub_method(self):
        """BranchAndBoundConfig.sub_method absent -> BranchAndBoundMethodPointer selected."""
        obj = BranchAndBoundConfig()
        sm = obj.sub_method
        self.assertTrue(
            hasattr(sm, "method_pointer"),
            "Expected BranchAndBoundMethodPointer branch, got {}".format(type(sm).__name__),
        )
        self.assertEqual(sm.method_pointer, POINTER_SENTINEL)

    def test_surrogate_based_local_config_sub_method(self):
        """SurrogateBasedLocalConfig.sub_method absent -> pointer branch selected."""
        obj = SurrogateBasedLocalConfig()
        sm = obj.sub_method
        self.assertTrue(
            hasattr(sm, "method_pointer"),
            "Expected SurrogateBasedLocalMethodPointer branch, got {}".format(
                type(sm).__name__
            ),
        )
        self.assertEqual(sm.method_pointer, POINTER_SENTINEL)


# ---------------------------------------------------------------------------
# 6. Pointer-union: user supplies name branch
# ---------------------------------------------------------------------------

class TestPointerUnionUserNameBranch(unittest.TestCase):
    """When the user provides the name branch of a pointer_union it is
    validated normally; the sentinel is not injected."""

    def test_embedded_config_global_method_name(self):
        """Providing global_method_name branch is accepted; name preserved."""
        obj = EmbeddedConfig(
            sub_method={"global_method_name": {"name": "coliny_ea"}}
        )
        sm = obj.sub_method
        self.assertTrue(
            hasattr(sm, "global_method_name"),
            "Expected GlobalMethodName branch, got {}".format(type(sm).__name__),
        )
        self.assertEqual(sm.global_method_name.name, "coliny_ea")

    def test_pareto_set_method_name_branch(self):
        """Providing method_name branch for ParetoSetConfig is accepted."""
        obj = ParetoSetConfig(
            sub_method={"method_name": {"method_name": "coliny_ea"}}
        )
        sm = obj.sub_method
        self.assertTrue(
            hasattr(sm, "method_name"),
            "Expected ParetoSetMethodName branch, got {}".format(type(sm).__name__),
        )
        self.assertEqual(sm.method_name.method_name, "coliny_ea")


# ---------------------------------------------------------------------------
# 7. Pointer-union absent in input_file_mode
# ---------------------------------------------------------------------------

class TestPointerUnionInputFileMode(unittest.TestCase):
    """Pointer-union fields absent in input_file_mode are enforced normally."""

    def test_embedded_config_raises(self):
        """EmbeddedConfig without sub_method in input_file_mode -> ValidationError."""
        with input_file_mode():
            with self.assertRaises(ValidationError):
                EmbeddedConfig()

    def test_pareto_set_config_raises(self):
        """ParetoSetConfig without sub_method in input_file_mode -> ValidationError."""
        with input_file_mode():
            with self.assertRaises(ValidationError):
                ParetoSetConfig()

    def test_multi_start_config_raises(self):
        """MultiStartConfig without sub_method in input_file_mode -> ValidationError."""
        with input_file_mode():
            with self.assertRaises(ValidationError):
                MultiStartConfig()


# ---------------------------------------------------------------------------
# 8. Pointer-union where every branch is all-pointer (Ensemble)
# ---------------------------------------------------------------------------

class TestPointerUnionAllPointerBranches(unittest.TestCase):
    """Ensemble.ensemble has two all-pointer branches; the first is selected
    automatically in API mode."""

    def test_ensemble_absent_api_mode_selects_first_branch(self):
        """Ensemble.ensemble absent -> first all-pointer branch injected."""
        obj = Ensemble()
        ens = obj.ensemble
        self.assertIsNotNone(ens, "ensemble field should have been injected")
        type_name = type(ens).__name__
        self.assertIn(
            type_name,
            ("OrderedModelFidelities", "EnsembleTruthModelPointer"),
            "Unexpected branch type: {}".format(type_name),
        )

    def test_ensemble_input_file_mode_raises(self):
        """Ensemble.ensemble absent in input_file_mode -> ValidationError."""
        with input_file_mode():
            with self.assertRaises(ValidationError):
                Ensemble()


# ---------------------------------------------------------------------------
# 9. Defensive check -- pointer_union with no qualifying branch
# ---------------------------------------------------------------------------

class TestPointerUnionDefensiveCheck(unittest.TestCase):
    """If a pointer_union field has no all-pointer branch a ValueError is
    raised during validation, preventing silent misconfiguration."""

    def test_no_all_pointer_branch_raises(self):
        """pointer_union annotated on a Union with no all-pointer branch raises."""

        class _NonPointerA(DakotaBaseModel):
            name: str = DakotaField(description="A method name")

        class _NonPointerB(DakotaBaseModel):
            count: int = DakotaField(description="An integer count")

        class _BadParent(DakotaBaseModel):
            sub_method: Union[_NonPointerA, _NonPointerB] = DakotaField(
                description="No pointer branch here",
                dakota={"pointer_union": True},
            )

        # Either ValueError (from the sentinel helper) or ValidationError
        # (Pydantic wrapping it) must be raised.
        with self.assertRaises((ValueError, ValidationError)):
            _BadParent()


# ---------------------------------------------------------------------------
# 10. Pre-validated pointer-group child instance accepted directly
# ---------------------------------------------------------------------------

class TestPointerGroupPrevalidatedChild(unittest.TestCase):
    """An already-validated child model instance can be passed as the value
    of a pointer-group field without re-running sentinel injection."""

    def test_prevalidated_instance_accepted(self):
        """A SubMethodPointer instance (with sentinel) is accepted by NestedConfig."""
        child = SubMethodPointer()
        self.assertEqual(child.pointer, POINTER_SENTINEL)
        obj = NestedConfig(sub_method_pointer=child)
        self.assertEqual(obj.sub_method_pointer.pointer, POINTER_SENTINEL)

    def test_prevalidated_instance_with_explicit_pointer(self):
        """A SubMethodPointer with an explicit pointer flows through unchanged."""
        child = SubMethodPointer(pointer="explicit_method")
        obj = NestedConfig(sub_method_pointer=child)
        self.assertEqual(obj.sub_method_pointer.pointer, "explicit_method")
        self.assertNotEqual(obj.sub_method_pointer.pointer, POINTER_SENTINEL)


if __name__ == "__main__":
    unittest.main()
