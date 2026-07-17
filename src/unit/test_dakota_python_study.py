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
        study.method.sampling({"samples": 2}, None)
    except Exception as exc:
        assert "top-level 'sampling'" in str(exc)
    else:
        raise AssertionError("sampling() accepted a missing top-level keyword")


if __name__ == "__main__":
    main()
