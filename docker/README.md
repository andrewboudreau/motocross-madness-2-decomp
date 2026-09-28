# Linux container workflow

Put `MCM2PCG.exe` in a local `input/` directory (ignored by Git) or point `MCM2_INPUT_DIR` at a directory containing it.

```bash
docker compose build decomp
docker compose run --rm decomp
```

Run other bootstrap commands in the same environment:

```bash
docker compose run --rm decomp make status
docker compose run --rm decomp make easy
docker compose run --rm decomp make smoke
docker compose run --rm decomp make calibration
```

For the real historical compiler, point `VC6_ROOT_HOST` at a private VC6 SP3 tree:

```bash
VC6_ROOT_HOST=/absolute/path/to/vc6sp3 \
  docker compose run --rm decomp python3 tools/probe_vc6.py

VC6_ROOT_HOST=/absolute/path/to/vc6sp3 \
  docker compose run --rm decomp make smoke-vc6 VC6_ROOT=/toolchains/vc6sp3
```

Microsoft compiler binaries are deliberately not distributed with this project.
