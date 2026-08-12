"""Smoke tests for the dakota.study DI/library-mode bindings."""

from dakota.study import Study, StudyConfig


def main():
    config = StudyConfig()
    config.output.precision = 12
    config.output.output_file = "dakota_python_study.out"
    config.output.error_file = "dakota_python_study.err"

    study = Study(config)
    assert study.method is not None
    assert study.model is not None

    try:
        study.variables({"bogus": 2})
    except Exception as exc:
        assert "permit" in str(exc).lower() or "extra" in str(exc).lower()
    else:
        raise AssertionError("study.variables() accepted an invalid fragment")


if __name__ == "__main__":
    main()
