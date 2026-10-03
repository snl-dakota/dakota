"""Harness checks requiring only Python and Perl, not native bindings."""

from pathlib import Path
import shutil
import json
import subprocess
import sys
import tempfile
import unittest

from run_pair import execute, result_structure, compare_outputs

SOURCE = Path(__file__).resolve().parents[1]
PERL = shutil.which("perl")
RAW = """<<<<< Function evaluation summary: 5 total (5 new, 0 duplicate)
<<<<< Best parameters          =
                      1.0000000000e+00 cdv_1
                      1.0000000000e+00 cdv_2
                      1.0000000000e+00 cdv_3
<<<<< Best objective function  =
                      0.0000000000e+00
<<<<< Best constraint values   =
                      5.0000000000e-01
                      5.0000000000e-01
<<<<< Best evaluation ID: 1
"""


class HarnessTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.work = Path(self.temp.name)

    def reduce(self, raw, name):
        output = self.work / (name + ".out")
        result = self.work / (name + ".tst")
        output.write_text(raw)
        execute(
            [
                PERL,
                str(SOURCE / "dakota_test.perl"),
                "--reduce-output=" + str(output),
                "--reduce-destination=" + str(result),
                "0",
            ],
            self.work,
            name,
        )
        return result

    def test_matching_and_out_of_tolerance(self):
        a = self.reduce(RAW, "a")
        b = self.reduce(RAW, "b")
        self.assertEqual(result_structure(a), result_structure(b))
        command = [
            PERL,
            str(SOURCE / "dakota_diff.perl"),
            "dakota_pstudy.in",
            str(a),
            str(b),
        ]
        execute(command, self.work, "match")
        self.reduce(RAW.replace("5.0000000000e-01", "9.0000000000e-01"), "b")
        with self.assertRaises(subprocess.CalledProcessError):
            execute(command, self.work, "mismatch")

    def test_verification_and_integration_results(self):
        raw = RAW + """
Refinement Rate = 2
Refinement Reference Pt  =
  8 resolution

Final Convergence Rates  =
             resolution
response_fn_1 2.0000000000e+00

Extrapolated QOI         =
             resolution
response_fn_1 2.0000000000e+00

Final QOI Error Estimate =
             resolution
response_fn_1 1.5625000000e-02

Estimated integral of response_fn_1 = 1.0000000000e-01
"""
        a = self.reduce(raw, "verification_a")
        self.assertIn("Final Convergence Rates", a.read_text())
        self.assertIn("Estimated integral", a.read_text())
        b = self.reduce(
            raw.replace("2.0000000000e+00", "3.0000000000e+00"), "verification_b"
        )
        with self.assertRaises(subprocess.CalledProcessError):
            compare_outputs(
                [
                    PERL,
                    str(SOURCE / "dakota_diff.perl"),
                    "richardson_extrap.in",
                    str(a),
                    str(b),
                ],
                [a, b],
                self.work,
                "verification",
                0,
            )

    def test_pair_diff_file(self):
        a = self.reduce(RAW, "a")
        b = self.reduce(RAW, "b")
        command = [
            PERL,
            str(SOURCE / "dakota_diff.perl"),
            "dakota_pstudy.in",
            str(a),
            str(b),
        ]
        compare_outputs(command, [a, b], self.work, "example", 0)
        report = self.work / "dakota_diffs.out"
        self.assertIn("PASS test 0", report.read_text())
        self.reduce(RAW.replace("5.0000000000e-01", "9.0000000000e-01"), "b")
        with self.assertRaises(subprocess.CalledProcessError):
            compare_outputs(command, [a, b], self.work, "example", 0)
        self.assertIn("DIFF test 0", report.read_text())
        self.assertIn("base<", report.read_text())
        self.reduce(RAW.replace("cdv_3", "other_label"), "b")
        with self.assertRaises((ValueError, subprocess.CalledProcessError)):
            compare_outputs(command, [a, b], self.work, "example", 0)
        self.assertIn("--- executable/results.tst", report.read_text())
        self.assertIn("+++ python/results.tst", report.read_text())

    def test_timeout(self):
        with self.assertRaises(subprocess.TimeoutExpired):
            execute([PERL, "-e", "sleep 5"], self.work, "timeout", timeout=0.02)
        self.assertTrue((self.work / "timeout.stderr").exists())

    @unittest.skipUnless(shutil.which("false"), "false executable required")
    def test_failed_pair_keeps_report_and_clears_stale_output(self):
        for kind in ("freeform", "python"):
            directory = self.work / kind
            directory.mkdir()
            (directory / "stale").write_text("stale")
        (self.work / "comparison.stdout").write_text("PASS test 0")
        command = [
            sys.executable,
            str(SOURCE / "python_regression/run_pair.py"),
            "--case",
            "pstudy_s0",
            "--source",
            str(SOURCE),
            "--work",
            str(self.work),
            "--dakota",
            shutil.which("false"),
            "--driver",
            PERL,
            "--python",
            sys.executable,
            "--python-path",
            str(SOURCE.parent / "python"),
            "--perl",
            PERL,
            "--runner",
            str(SOURCE / "dakota_test.perl"),
            "--diff",
            str(SOURCE / "dakota_diff.perl"),
        ]
        result = subprocess.run(command, capture_output=True, text=True)
        self.assertEqual(result.returncode, 1)
        self.assertIn("FAIL test 0", (self.work / "dakota_diffs.out").read_text())
        self.assertFalse((self.work / "comparison.stdout").exists())
        for kind in ("freeform", "python"):
            self.assertFalse((self.work / kind / "stale").exists())

    def test_empty_and_structurally_different_output(self):
        original = self.reduce(RAW, "original")
        with self.assertRaises(ValueError):
            result_structure(self.reduce("", "empty"))
        for index, raw in enumerate(
            (
                RAW.split("<<<<< Best constraint")[0],
                RAW.replace("                      1.0000000000e+00 cdv_3\n", ""),
                RAW.replace("cdv_3", "different_label"),
            )
        ):
            with self.subTest(index=index):
                self.assertNotEqual(
                    result_structure(original),
                    result_structure(self.reduce(raw, str(index))),
                )

    def test_extract_all_manifest_cases(self):
        cases = json.loads((SOURCE / "python_regression/cases.json").read_text())
        for case in cases:
            with self.subTest(case=case["id"]):
                path = self.work / (case["id"] + ".in")
                execute(
                    [
                        PERL,
                        str(SOURCE / "dakota_test.perl"),
                        "--file-extract=" + str(path),
                        str(SOURCE / case["input"]),
                        str(case["subtest"]),
                    ],
                    self.work,
                    case["id"],
                )
                self.assertTrue(path.read_text().strip())
                self.assertTrue(
                    (SOURCE / "python_regression" / case["script"]).is_file()
                )
                for file in case["required_files"]:
                    if file != "text_book":
                        self.assertTrue((SOURCE / file).is_file(), file)

    def test_failed_process_and_missing_output(self):
        with self.assertRaises(subprocess.CalledProcessError):
            execute([PERL, "-e", "exit 7"], self.work, "failure")
        with self.assertRaises(subprocess.CalledProcessError):
            execute(
                [
                    PERL,
                    str(SOURCE / "dakota_test.perl"),
                    "--reduce-output=absent",
                    "--reduce-destination=absent.tst",
                    "0",
                ],
                self.work,
                "missing",
            )

    @unittest.skipUnless(shutil.which("cmake"), "CMake required")
    def test_diff_collection_includes_pairs(self):
        legacy = self.work / "test/dakota_legacy"
        pair = self.work / "test/python_regression/pair"
        legacy.mkdir(parents=True)
        pair.mkdir(parents=True)
        (legacy / "dakota_diffs.out").write_text("legacy\nPASS test 0\n")
        (pair / "dakota_diffs.out").write_text("python_regression_pair\nDIFF test 0\n")
        subprocess.run(
            [
                "cmake",
                "-DCMAKE_MODULE_PATH=" + str(SOURCE.parent / "cmake"),
                "-DDakota_BINARY_DIR=" + str(self.work),
                "-P",
                str(SOURCE.parent / "cmake/DakotaConcatDiffs.cmake"),
            ],
            check=True,
            capture_output=True,
        )
        report = (self.work / "test/dakota_diffs.out").read_text()
        self.assertIn("legacy", report)
        self.assertIn("python_regression_pair", report)

    @unittest.skipUnless(
        shutil.which("cmake") and shutil.which("ctest"), "CMake required"
    )
    def test_registration_enabled_and_disabled(self):
        cmake = (SOURCE / "CMakeLists.txt").read_text()
        start = cmake.index("  if(DAKOTA_PYTHON_STUDY)")
        end = cmake.index("  # Create one CTest per Dakota input", start)
        project = self.work / "project"
        project.mkdir()
        (project / "python_regression").mkdir()
        shutil.copy2(
            SOURCE / "python_regression/cases.json",
            project / "python_regression/cases.json",
        )
        cases = json.loads((SOURCE / "python_regression/cases.json").read_text())
        feature_names = sorted(
            {feature for case in cases for feature in case.get("requires_features", [])}
        )
        (project / "CMakeLists.txt").write_text(
            "cmake_minimum_required(VERSION 3.23)\nproject(Registration NONE)\nenable_testing()\n"
            "add_executable(dakota IMPORTED)\nset_target_properties(dakota PROPERTIES IMPORTED_LOCATION /bin/true)\n"
            "add_executable(text_book IMPORTED)\nset_target_properties(text_book PROPERTIES IMPORTED_LOCATION /bin/true)\n"
            + "".join(f'option({name} "{name}" ON)\n' for name in feature_names)
            + "set(Python3_EXECUTABLE /usr/bin/python3)\nset(PERL_EXECUTABLE /usr/bin/perl)\n"
            + cmake[start:end]
        )
        case_count = len(cases)
        unconditional = sum(not case.get("requires_features") for case in cases)
        for enabled, features, count in (
            ("ON", "ON", case_count),
            ("ON", "OFF", unconditional),
            ("OFF", "ON", 0),
        ):
            build = self.work / (enabled + features)
            subprocess.run(
                [
                    "cmake",
                    "-S",
                    str(project),
                    "-B",
                    str(build),
                    "-DDAKOTA_PYTHON_STUDY=" + enabled,
                    *["-D" + name + "=" + features for name in feature_names],
                ],
                check=True,
                capture_output=True,
            )
            result = subprocess.run(
                ["ctest", "--test-dir", str(build), "--show-only=json-v1"],
                check=True,
                capture_output=True,
                text=True,
            )
            tests = json.loads(result.stdout)["tests"]
            self.assertEqual(len(tests), count)
            for test in tests:
                props = {p["name"]: p["value"] for p in test["properties"]}
                self.assertEqual(props["LABELS"], ["PythonRegression", "SerialTest"])
                self.assertEqual(props["TIMEOUT"], 120)

    def test_extract_original_subtests(self):
        for number, method in ((0, "vector"), (3, "list"), (4, "centered")):
            path = self.work / f"s{number}.in"
            execute(
                [
                    PERL,
                    str(SOURCE / "dakota_test.perl"),
                    "--file-extract=" + str(path),
                    str(SOURCE / "dakota_pstudy.in"),
                    str(number),
                ],
                self.work,
                f"extract{number}",
            )
            text = path.read_text()
            self.assertIn(method + "_parameter_study", text)
            self.assertIn("analytic_gradients" if number == 0 else "no_gradients", text)
            self.assertIn("analytic_hessians" if number == 0 else "no_hessians", text)
            self.assertIn("evaluation_concurrency = 5", text)


if __name__ == "__main__":
    unittest.main()
