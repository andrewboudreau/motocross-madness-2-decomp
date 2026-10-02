PYTHON ?= python3
INSTALLER ?= MCM2PCG.exe
EXE ?= $(if $(MCM2_EXE),$(MCM2_EXE),work/game/mcm2.exe)
VC6_ROOT ?=
SKELETON_ROOT ?= generated/krusty2-skeletons
PRIVATE_ROOT ?= $(if $(MCM2_PRIVATE_ROOT),$(MCM2_PRIVATE_ROOT),$(HOME)/.cache/mcm2-private)
PRIVATE_BUNDLE ?=

.PHONY: bootstrap extract analyze class-evidence msvc-artifacts ensure-work smoke smoke-vc6 easy-smoke easy-smoke-vc6 calibration calibration-vc6 status easy manifest work-queue selftest static-check test wine-init import-vc6 probe-vc6 vc6-gate progress progress-check

bootstrap:
	PYTHONPATH=. $(PYTHON) tools/bootstrap.py "$(INSTALLER)"

extract:
	PYTHONPATH=. $(PYTHON) tools/extract_installer.py "$(INSTALLER)"

analyze: ensure-work
	PYTHONPATH=. $(PYTHON) tools/analyze.py "$(EXE)" --out analysis --skeleton-root "$(SKELETON_ROOT)"
	PYTHONPATH=. $(PYTHON) tools/build_class_evidence.py
	MCM2_EXE="$(EXE)" PYTHONPATH=. $(PYTHON) tools/analyze_msvc_artifacts.py
	MCM2_EXE="$(EXE)" PYTHONPATH=. $(PYTHON) tools/find_vtable_writes.py
	PYTHONPATH=. $(PYTHON) tools/build_function_manifest.py
	PYTHONPATH=. $(PYTHON) tools/build_class_dossiers.py
	PYTHONPATH=. $(PYTHON) tools/build_work_queue.py

class-evidence: ensure-work
	PYTHONPATH=. $(PYTHON) tools/build_class_evidence.py
	PYTHONPATH=. $(PYTHON) tools/build_class_dossiers.py

msvc-artifacts: ensure-work
	MCM2_EXE="$(EXE)" PYTHONPATH=. $(PYTHON) tools/analyze_msvc_artifacts.py
	MCM2_EXE="$(EXE)" PYTHONPATH=. $(PYTHON) tools/find_vtable_writes.py
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

progress:
	PYTHONPATH=. $(PYTHON) tools/decompilation_progress.py

progress-check:
	PYTHONPATH=. $(PYTHON) tools/decompilation_progress.py --check

easy:
	PYTHONPATH=. $(PYTHON) tools/discover_easy_targets.py

manifest:
	PYTHONPATH=. $(PYTHON) tools/build_function_manifest.py
	PYTHONPATH=. $(PYTHON) tools/build_class_dossiers.py
	PYTHONPATH=. $(PYTHON) tools/build_work_queue.py

work-queue:
	PYTHONPATH=. $(PYTHON) tools/build_work_queue.py

selftest: ensure-work
	MCM2_EXE="$(EXE)" PYTHONPATH=. $(PYTHON) tools/selftest.py

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

static-check:
	PYTHONPATH=. $(PYTHON) tools/static_check.py

test:
	PYTHONPATH=. $(PYTHON) -m unittest discover -s tests -v

.PHONY: provenance provenance-test
provenance: ensure-work
	$(PYTHON) tools/build_provenance.py --exe "$(EXE)"

provenance-test:
	PYTHONPATH=. $(PYTHON) -m unittest discover -s tests -p 'test_provenance.py' -v

.PHONY: allocation allocation-probes allocation-test
allocation: ensure-work
	$(PYTHON) tools/analyze_allocation.py --exe "$(EXE)"

allocation-probes: ensure-work
	$(PYTHON) tools/analyze_allocation.py --exe "$(EXE)" --compile-probe

allocation-test:
	PYTHONPATH=. $(PYTHON) -m unittest discover -s tests -p 'test_allocation.py' -v

.PHONY: categories categories-test
categories: provenance
	$(PYTHON) tools/build_categories.py --exe "$(EXE)"

categories-test:
	PYTHONPATH=. $(PYTHON) -m unittest discover -s tests -p 'test_categories.py' -v

.PHONY: category-contexts category-contexts-test
category-contexts: ensure-work
	$(PYTHON) tools/build_category_contexts.py --exe "$(EXE)"

category-contexts-test:
	PYTHONPATH=. $(PYTHON) -m unittest discover -s tests -p 'test_category_contexts.py' -v

.PHONY: private-install private-ready vc6-private-gate private-bundle-test
private-install:
	@if [ -n "$(PRIVATE_BUNDLE)" ]; then \
		$(PYTHON) tools/install_private_bundle.py --archive "$(PRIVATE_BUNDLE)" --root "$(PRIVATE_ROOT)"; \
	else \
		$(PYTHON) tools/install_private_bundle.py --url-env MCM2_PRIVATE_BUNDLE_URL --root "$(PRIVATE_ROOT)"; \
	fi

private-ready:
	$(PYTHON) tools/with_private_env.py --root "$(PRIVATE_ROOT)" -- $(PYTHON) tools/vc6_acceptance.py --root "$(PRIVATE_ROOT)"

vc6-private-gate:
	$(PYTHON) tools/with_private_env.py --root "$(PRIVATE_ROOT)" -- $(PYTHON) tools/vc6_acceptance.py --root "$(PRIVATE_ROOT)" --full-gate

private-bundle-test:
	PYTHONPATH=. $(PYTHON) -m unittest discover -s tests -p 'test_private_bundle.py' -v

.PHONY: vc6-crt-atlas vc6-crt-atlas-test
vc6-crt-atlas:
	$(PYTHON) tools/with_private_env.py --root "$(PRIVATE_ROOT)" -- $(PYTHON) tools/build_vc6_crt_atlas.py

vc6-crt-atlas-test:
	PYTHONPATH=. $(PYTHON) -m unittest discover -s tests -p 'test_crt_atlas.py' -v

.PHONY: vc6-profile-matrix vc6-profile-matrix-test
vc6-profile-matrix:
	$(PYTHON) tools/with_private_env.py --root "$(PRIVATE_ROOT)" -- $(PYTHON) tools/vc6_profile_matrix.py

vc6-profile-matrix-test:
	PYTHONPATH=. $(PYTHON) -m unittest discover -s tests -p 'test_vc6_profile_matrix.py' -v

.PHONY: category-pilots category-pilots-probe category-pilots-test
category-pilots: ensure-work
	$(PYTHON) tools/review_category_pilots.py --exe "$(EXE)"

category-pilots-probe: ensure-work
	$(PYTHON) tools/review_category_pilots.py --exe "$(EXE)" --compile-probe

category-pilots-test:
	PYTHONPATH=. $(PYTHON) -m unittest discover -s tests -p 'test_category_pilots.py' -v

.PHONY: ecosystem ecosystem-probes ecosystem-test
ecosystem: ensure-work
	$(PYTHON) tools/review_ecosystem.py --exe "$(EXE)"

ecosystem-probes: ensure-work
	$(PYTHON) tools/review_ecosystem.py --exe "$(EXE)" --compile-probes

ecosystem-test:
	PYTHONPATH=. $(PYTHON) -m unittest discover -s tests -p 'test_ecosystem.py' -v
