"""Unit tests for dakota.tools.freeform_to_json (dakota_parser subprocess stubbed)."""

import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import MagicMock, patch

from dakota.tools.freeform_to_json import convert, main


_SAMPLE_VALIDATED = {
    "variables": [
        {
            "normal_uncertain": {
                "count": 1,
                "means": [0.0],
                "std_deviations": [1.0],
                "initial_point": [0.0],
                "initial_point_user_provided": False,
                "inferred_lower_bounds": [-3.0],
                "inferred_upper_bounds": [3.0],
            }
        }
    ]
}


def _make_parser_mock(payload: dict):
    """Return a mock for subprocess.run that yields *payload* as stdout JSON."""
    mock_result = MagicMock()
    mock_result.stdout = json.dumps(payload)
    mock_result.returncode = 0
    return mock_result


class ConvertFunctionTest(unittest.TestCase):

    def test_convert_strips_internal_fields_and_writes_output(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            input_path = Path(tmpdir) / "study.in"
            input_path.write_text("# stub\n")
            output_path = Path(tmpdir) / "study.json"

            with patch("subprocess.run", return_value=_make_parser_mock(_SAMPLE_VALIDATED)):
                convert(input_path, output_path)

            result = json.loads(output_path.read_text())
            normal = result["variables"][0]["normal_uncertain"]
            self.assertNotIn("initial_point_user_provided", normal)
            self.assertNotIn("inferred_lower_bounds", normal)
            self.assertNotIn("inferred_upper_bounds", normal)
            self.assertNotIn("initial_point", normal)
            self.assertIn("means", normal)

    def test_convert_default_output_extension(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            input_path = Path(tmpdir) / "study.in"
            input_path.write_text("# stub\n")

            with patch("subprocess.run", return_value=_make_parser_mock(_SAMPLE_VALIDATED)):
                convert(input_path, input_path.with_suffix(".json"))

            output_path = Path(tmpdir) / "study.json"
            self.assertTrue(output_path.exists())

    def test_convert_passes_dakota_parser_arg(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            input_path = Path(tmpdir) / "study.in"
            input_path.write_text("# stub\n")
            output_path = Path(tmpdir) / "study.json"

            with patch("subprocess.run", return_value=_make_parser_mock(_SAMPLE_VALIDATED)) as mock_run:
                convert(input_path, output_path, dakota_parser="/custom/dakota_parser")

            called_cmd = mock_run.call_args[0][0]
            self.assertEqual(called_cmd[0], "/custom/dakota_parser")

    def test_convert_raises_on_parser_failure(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            input_path = Path(tmpdir) / "study.in"
            input_path.write_text("# stub\n")
            output_path = Path(tmpdir) / "study.json"

            with patch(
                "subprocess.run",
                side_effect=subprocess.CalledProcessError(1, "dakota_parser", stderr="bad input"),
            ):
                with self.assertRaises(subprocess.CalledProcessError):
                    convert(input_path, output_path)

    def test_convert_raises_on_missing_parser(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            input_path = Path(tmpdir) / "study.in"
            input_path.write_text("# stub\n")
            output_path = Path(tmpdir) / "study.json"

            with patch("subprocess.run", side_effect=FileNotFoundError("no such file")):
                with self.assertRaises(FileNotFoundError):
                    convert(input_path, output_path)


class MainFunctionTest(unittest.TestCase):

    def test_main_returns_zero_on_success(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            input_path = Path(tmpdir) / "study.in"
            input_path.write_text("# stub\n")
            output_path = Path(tmpdir) / "study.json"

            with patch("subprocess.run", return_value=_make_parser_mock(_SAMPLE_VALIDATED)):
                with patch("sys.argv", ["dakota-to-json", str(input_path), str(output_path)]):
                    rc = main()

            self.assertEqual(rc, 0)

    def test_main_defaults_output_to_json_extension(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            input_path = Path(tmpdir) / "study.in"
            input_path.write_text("# stub\n")

            with patch("subprocess.run", return_value=_make_parser_mock(_SAMPLE_VALIDATED)):
                with patch("sys.argv", ["dakota-to-json", str(input_path)]):
                    rc = main()

            self.assertEqual(rc, 0)
            self.assertTrue((Path(tmpdir) / "study.json").exists())

    def test_main_returns_nonzero_on_missing_parser(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            input_path = Path(tmpdir) / "study.in"
            input_path.write_text("# stub\n")

            with patch("subprocess.run", side_effect=FileNotFoundError("no such file")):
                with patch("sys.argv", ["dakota-to-json", str(input_path)]):
                    rc = main()

            self.assertNotEqual(rc, 0)


if __name__ == "__main__":
    unittest.main()
