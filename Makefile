PYTHON ?= python3
INSTALLER ?= MCM2PCG.exe
EXE ?= work/game/mcm2.exe
VC6_ROOT ?=

.PHONY: bootstrap extract analyze class-evidence msvc-artifacts ensure-work smoke smoke-vc6 easy-smoke easy-smoke-vc6 calibration calibration-vc6 status easy manifest work-queue selftest static-check wine-init import-vc6 probe-vc6 vc6-gate clean-work

bootstrap:
	PYTHONPATH=. $(PYTHON) tools/bootstrap.py "$(INSTALLER)"

extract:
	PYTHONPATH=. $(PYTHON) tools/extract_installer.py "$(INSTALLER)"

analyze: ensure-work
	PYTHONPATH=. $(PYTHON) tools/analyze.py "$(EXE)" --out analysis --skeleton-root src/krusty2
	PYTHONPATH=. $(PYTHON) tools/build_class_evidence.py
	PYTHONPATH=. $(PYTHON) tools/analyze_msvc_artifacts.py
	PYTHONPATH=. $(PYTHON) tools/find_vtable_writes.py
	PYTHONPATH=. $(PYTHON) tools/build_function_manifest.py
	PYTHONPATH=. $(PYTHON) tools/build_class_dossiers.py
	PYTHONPATH=. $(PYTHON) tools/build_work_queue.py

class-evidence: ensure-work
	PYTHONPATH=. $(PYTHON) tools/build_class_evidence.py
	PYTHONPATH=. $(PYTHON) tools/build_class_dossiers.py

msvc-artifacts: ensure-work
	PYTHONPATH=. $(PYTHON) tools/analyze_msvc_artifacts.py
	PYTHONPATH=. $(PYTHON) tools/find_vtable_writes.py
	PYTHONPATH=. $(PYTHON) tools/build_class_dossiers.py

ensure-work:
	@test -f "$(EXE)" || (echo 'Missing $(EXE). Run: make bootstrap INSTALLER=/path/to/MCM2PCG.exe' && exit 2)

smoke: ensure-work
	PYTHONPATH=. $(PYTHON) tools/run_samples.py --exe "$(EXE)" --compiler clang-cl

smoke-vc6: ensure-work
	@test -n "$(VC6_ROOT)" || (echo 'VC6_ROOT is required' && exit 2)
	PYTHONPATH=. $(PYTHON) tools/run_samples.py --exe "$(EXE)" --compiler vc6 --vc6-root "$(VC6_ROOT)"

easy-smoke: ensure-work
	PYTHONPATH=. $(PYTHON) tools/run_easy_probes.py --exe "$(EXE)" --compiler clang-cl

easy-smoke-vc6: ensure-work
	@test -n "$(VC6_ROOT)" || (echo 'VC6_ROOT is required' && exit 2)
	PYTHONPATH=. $(PYTHON) tools/run_easy_probes.py --exe "$(EXE)" --compiler vc6 --vc6-root "$(VC6_ROOT)"

calibration: ensure-work
	PYTHONPATH=. $(PYTHON) tools/run_calibration.py --exe "$(EXE)" --compiler clang-cl

calibration-vc6: ensure-work
	@test -n "$(VC6_ROOT)" || (echo 'VC6_ROOT is required' && exit 2)
	PYTHONPATH=. $(PYTHON) tools/run_calibration.py --exe "$(EXE)" --compiler vc6 --vc6-root "$(VC6_ROOT)"

status:
	PYTHONPATH=. $(PYTHON) tools/status.py

easy:
	PYTHONPATH=. $(PYTHON) tools/discover_easy_targets.py

manifest:
	PYTHONPATH=. $(PYTHON) tools/build_function_manifest.py
	PYTHONPATH=. $(PYTHON) tools/build_class_dossiers.py
	PYTHONPATH=. $(PYTHON) tools/build_work_queue.py

work-queue:
	PYTHONPATH=. $(PYTHON) tools/build_work_queue.py

selftest: ensure-work
	PYTHONPATH=. $(PYTHON) tools/selftest.py

wine-init:
	PYTHONPATH=. $(PYTHON) tools/init_wine_prefix.py

import-vc6:
	@test -n "$(VC6_SOURCE)" || (echo 'VC6_SOURCE is required (installed VC98 tree or archive)' && exit 2)
	PYTHONPATH=. $(PYTHON) tools/import_vc6.py "$(VC6_SOURCE)" --out "$${VC6_IMPORT_OUT:-toolchains/vc6sp3}"

probe-vc6:
	@test -n "$(VC6_ROOT)" || (echo 'VC6_ROOT is required' && exit 2)
	PYTHONPATH=. $(PYTHON) tools/probe_vc6.py --vc6-root "$(VC6_ROOT)"

vc6-gate: ensure-work
	@test -n "$(VC6_ROOT)" || (echo 'VC6_ROOT is required' && exit 2)
	PYTHONPATH=. $(PYTHON) tools/vc6_gate.py --vc6-root "$(VC6_ROOT)" --exe "$(EXE)"

clean-work:
	rm -rf work

static-check:
	PYTHONPATH=. $(PYTHON) tools/static_check.py

.PHONY: provenance provenance-test
provenance: ensure-work
	$(PYTHON) tools/build_provenance.py --exe "$(EXE)"

provenance-test:
	PYTHONPATH=. $(PYTHON) -m unittest discover -s tests -p 'test_provenance.py' -v
