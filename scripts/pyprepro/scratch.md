Specify format options at the top of the file. They must be the first non-whitespace lines and must start with `//`, `#`,`%`,`$`. Note: they will be parsed out.

Options include

| Command                                       | Command-line equivalent | Default |
|-----------------------------------------------|-------------------------|---------|
| `PYPREPRO_CODE` or `DPREPRO_CODE`             | `--code`                | `%`     |
| `PYPREPRO_CODE_BLOCK` or `DPREPRO_CODE_BLOCK` | `--code-block`          | `{% %}` |
| `PYPREPRO_INLINE` or `DPREPRO_INLINE`         | `--inline`              | `{ }`   |

and specified as `<comment> <command> <space and/or equals> <specifier>`. For example:

```text
# PYPREPRO_CODE {{[ ]}}
$ DPREPRO_CODE_BLOCK=[[ ]]
% PYPREPRO_INLINE={{ }}

Rest of input deck
```
Note that inline formatting is set via `setfmt()`. So you may, for example, have

```
// PYPREPRO_CODE $$
$$ setfmt('%10.4g')
```

Tests

// PYPREPRO_CODE { }
# DPREPRO_CODE_BLOCK [{ }]
$ DPREPRO_INLINE=^

tests:

- X Do they get filtered
- X specified in include different than main one
    - X nested includes
- X Test in file and in text
- X Test different top formats
- preprocessor

================================