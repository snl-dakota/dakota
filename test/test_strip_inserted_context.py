#!/usr/bin/env python3
import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


ROOT = Path("/projects")
SCRIPT = ROOT / "tools" / "strip_inserted_context.py"
SCHEMA = ROOT / "src" / "dakota.json"


class StripInsertedContextTest(unittest.TestCase):
    def run_tool(self, payload: dict) -> tuple[dict, str]:
        with tempfile.TemporaryDirectory() as tmpdir:
            tmp = Path(tmpdir)
            input_path = tmp / "input.json"
            output_path = tmp / "output.json"
            input_path.write_text(json.dumps(payload, indent=2), encoding="utf-8")

            proc = subprocess.run(
                [
                    sys.executable,
                    "-I",
                    str(SCRIPT),
                    "--schema",
                    str(SCHEMA),
                    "--verbose",
                    str(input_path),
                    str(output_path),
                ],
                cwd=ROOT,
                text=True,
                capture_output=True,
                check=True,
            )

            cleaned = json.loads(output_path.read_text(encoding="utf-8"))
            return cleaned, proc.stderr

    def test_strips_internal_only_and_computed_fields(self):
        payload = {
            "variables": [
                {
                    "normal_uncertain": {
                        "count": 1,
                        "means": [1.0],
                        "std_deviations": [0.5],
                        "lower_bounds": [0.0],
                        "upper_bounds": [2.0],
                        "initial_point": [1.0],
                        "initial_point_user_provided": True,
                        "inferred_lower_bounds": [0.0],
                        "inferred_upper_bounds": [2.0],
                    }
                },
                {
                    "continuous_interval_uncertain": {
                        "count": 1,
                        "num_intervals": [2],
                        "lower_bounds": [0.0, 1.0],
                        "upper_bounds": [2.0, 3.0],
                        "initial_point": [1.5],
                        "initial_point_user_provided": True,
                        "inferred_lower_bounds": [0.0],
                        "inferred_upper_bounds": [3.0],
                    }
                },
            ]
        }

        cleaned, stderr = self.run_tool(payload)

        normal = cleaned["variables"][0]["normal_uncertain"]
        self.assertNotIn("initial_point_user_provided", normal)
        self.assertNotIn("inferred_lower_bounds", normal)
        self.assertNotIn("inferred_upper_bounds", normal)
        self.assertEqual(normal["means"], [1.0])
        self.assertEqual(normal["std_deviations"], [0.5])

        interval = cleaned["variables"][1]["continuous_interval_uncertain"]
        self.assertNotIn("initial_point_user_provided", interval)
        self.assertNotIn("inferred_lower_bounds", interval)
        self.assertNotIn("inferred_upper_bounds", interval)
        self.assertEqual(interval["num_intervals"], [2])
        self.assertEqual(interval["lower_bounds"], [0.0, 1.0])
        self.assertEqual(interval["upper_bounds"], [2.0, 3.0])

        self.assertIn("$.variables[0].normal_uncertain.initial_point_user_provided", stderr)
        self.assertIn("$.variables[0].normal_uncertain.inferred_lower_bounds", stderr)
        self.assertIn("$.variables[1].continuous_interval_uncertain.inferred_upper_bounds", stderr)


if __name__ == "__main__":
    unittest.main()
