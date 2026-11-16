"""The runtime's replay.c reads and calls the game by fixed addresses and offsets.

Retail addresses must equal the repository's bindings, every layout constant
must equal offsetof() over the reconstructed headers as VC6 compiles them,
and the vtable slot it calls must hold MarkPendingRelease in retail.
"""
import glob
import json
import os
from pathlib import Path
import re
import struct
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
REPLAY = ROOT / 'runtime/d3drm/replay.c'

# replay.c constant -> the C++ expression it must equal.
LAYOUT = {
    'TRACKGAME_PLAYER_COUNT': 'offsetof(TrackGame, playerCount)',
    'TRACKGAME_SUPERCROSS_EVENT': 'offsetof(TrackGame, supercrossEvent)',
    'TRACKGAME_UI': 'offsetof(TrackGame, ui)',
    'TRACKGAME_EVENT_MANAGER': 'offsetof(TrackGame, eventManager)',
    'TRACKGAME_MODE': 'offsetof(TrackGame, mode)',
    'TRACKGAME_RACE_SETTINGS': 'offsetof(TrackGame, mode) + offsetof(TrackGameMode, raceSettings)',
    'RACE_SETTINGS_SIZE': 'sizeof(RaceSettings)',
    'TRACKGAME_RACE_MODE': 'offsetof(TrackGame, mode) + offsetof(TrackGameMode, raceSettings)'
                           ' + offsetof(RaceSettings, raceMode)',
    'TRACKGAME_EVENT_TYPE': 'offsetof(TrackGame, mode) + offsetof(TrackGameMode, raceSettings)'
                            ' + offsetof(RaceSettings, eventType)',
    'TRACKGAME_AI_RACER_COUNT': 'offsetof(TrackGame, mode) + offsetof(TrackGameMode, raceSettings)'
                                ' + offsetof(RaceSettings, aiRacerCount)',
    'TRACKGAME_UI_BLOCKED': 'offsetof(TrackGame, uiInteractionBlocked)',
    'UI_GUI': 'offsetof(KrustyUI, gui)',
    'UI_CURRENT_MENU': 'offsetof(KrustyUI, currentMenu)',
    'EVENT_RACE_VIEW': 'offsetof(TrackGameViewOwner, raceView)',
    'EVENT_ELAPSED_MINUTES': 'offsetof(TrackGameViewOwner, elapsedMinutes)',
    'EVENT_ELAPSED_SECONDS': 'offsetof(TrackGameViewOwner, elapsedSeconds)',
    'RACE_LOCAL_RACER': 'offsetof(BikeRace, localRacer)',
    'RACE_AI_RACERS': 'offsetof(BikeRace, aiRacers)',
    'BIKE_POSITION': 'offsetof(KrustyBike, position)',
    'BIKE_VELOCITY': 'offsetof(KrustyBike, velocity)',
    'BIKE_LINEAR_SPEED': 'offsetof(KrustyBike, linearSpeed)',
    'BIKE_TOTAL_RUNNING_TIME': 'offsetof(KrustyBike, totalRunningTime)',
    'BIKE_FINISH_POSITION': 'offsetof(KrustyBike, finishPosition)',
    'BIKE_GATES_PASSED': 'offsetof(KrustyBike, gatesPassed)',
    'BIKE_LAP_COUNT': 'offsetof(KrustyBike, lapCount)',
    'MENU_MAIN': 'kMenuMain',
    'MENU_LOADING': 'kMenuLoading',
}
# The five event owners FormatState walks, four bytes apart from the first.
EVENTS = ('supercrossEvent', 'quarryEvent', 'bajaEnduroEvent', 'nationalEvent', 'tagEvent')
HEADERS = ('TrackGame.h', 'TrackGameGlobal.h', 'KrustyUI.h', 'BikeRaceCore.h', 'MenuId.h',
           'vehicle/KrustyBikeCore.h')


def constants():
    return {name: int(value, 0) for name, value in
            re.findall(r'^#define ([A-Z_]+) (0x[0-9a-f]+|\d+)\b', REPLAY.read_text(), re.M)}


def retail_symbols():
    return dict(re.findall(r'\{"([^"]+)", (0x[0-9a-f]{8})\}', REPLAY.read_text()))


class ReplaySourceTests(unittest.TestCase):
    def test_retail_symbols_equal_the_bindings(self):
        symbols = retail_symbols()
        self.assertEqual(len(symbols), 7)
        found = {}
        for path in glob.glob(str(ROOT / 'src/**/*.bindings.json'), recursive=True):
            data = json.loads(Path(path).read_text())
            if isinstance(data, dict):
                for name in symbols:
                    if name in data:
                        found.setdefault(name, set()).add(int(data[name], 0))
        for name, address in symbols.items():
            self.assertEqual(found.get(name), {int(address, 0)}, name)

    def test_every_layout_constant_is_checked(self):
        checked = set(LAYOUT) | {'TRACKGAME_EVENT_COUNT', 'GAMEOBJECT_MARK_PENDING_RELEASE',
                                 'CALLS_PER_TICK'}
        self.assertEqual(set(constants()), checked)
        self.assertEqual(constants()['TRACKGAME_EVENT_COUNT'], len(EVENTS))


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT'), 'needs VC6')
class ReplayLayoutTests(unittest.TestCase):
    def test_layout_constants_equal_offsetof_under_vc6(self):
        values = constants()
        lines = ['#include <stddef.h>'] + ['#include "%s"' % h for h in HEADERS]
        for name, expression in LAYOUT.items():
            lines.append('typedef char check_%s[(%s) == %d ? 1 : -1];' % (name, expression, values[name]))
        for index, member in enumerate(EVENTS):
            lines.append('typedef char check_event_%d[offsetof(TrackGame, %s) == %d ? 1 : -1];'
                         % (index, member, values['TRACKGAME_SUPERCROSS_EVENT'] + 4 * index))
        with tempfile.TemporaryDirectory(prefix='mcm2-replay-') as directory:
            source = Path(directory) / 'probe.cpp'
            source.write_text('\n'.join(lines) + '\n')
            result = subprocess.run(
                [sys.executable, str(ROOT / 'tools/compile.py'), str(source), '-o',
                 str(Path(directory) / 'probe.obj'), '--vc6-root', os.environ['VC6_ROOT'],
                 '--profile', 'vc6_o2_mt', '--include', str(ROOT / 'src/krusty2'),
                 '--include', str(ROOT / 'src/reconstructed')],
                cwd=ROOT, capture_output=True, text=True, encoding='latin1',
                env={**os.environ, 'PYTHONPATH': str(ROOT)})
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)


@unittest.skipUnless(os.environ.get('MCM2_EXE'), 'needs the retail executable')
class ReplayRetailTests(unittest.TestCase):
    def test_called_slot_is_mark_pending_release_in_the_dialog_vtable(self):
        from mcm2tool.pe import PEImage
        pe = PEImage(os.environ['MCM2_EXE'])
        slot = constants()['GAMEOBJECT_MARK_PENDING_RELEASE']
        dialog_vtable = 0x00552a9c  # ??_7UIDialog@@6B@ (GameUi.bindings.json)
        entry = struct.unpack('<I', pe.bytes_at_va(dialog_vtable + 4 * slot, 4))[0]
        self.assertEqual(entry, 0x004692c0)  # ?MarkPendingRelease@GameObject@@UAEXXZ


class CompareStateLogsTests(unittest.TestCase):
    RACER = '3f800000 40000000 40400000 00000000 00000000 00000000 00000000 0 1 0 7f7fffff'

    def logs(self, second_racer_x='3f800000'):
        from tools.compare_state_logs import load
        with tempfile.TemporaryDirectory() as directory:
            a, b = Path(directory) / 'a.txt', Path(directory) / 'b.txt'
            race = 'R 00000000 3c888889 N 1 | '
            a.write_text('\n'.join([
                'F 1 T 1000016 M 100',
                'F 2 T 1000032 M 104',
                'F 3 T 1000048 M 7 ' + race + self.RACER,
                'F 4 T 1000064 M 7 ' + race + self.RACER]) + '\n')
            b.write_text('\n'.join([
                'F 1 T 1000016 M 100',
                'F 2 T 1000032 M 104',
                'F 3 T 1000048 M 104',
                'F 4 T 1000064 M 7 ' + race + self.RACER,
                'F 5 T 1000080 M 7 ' + race + second_racer_x + self.RACER[8:]]) + '\n')
            return load(a), load(b)

    def test_aligns_at_the_first_race_frame(self):
        from tools.compare_state_logs import compare
        report = compare(*self.logs())
        self.assertEqual(report['race_start_frame'], [3, 4])
        self.assertEqual(report['result'], 'identical race frames')

    def test_reports_the_first_differing_field_as_a_float(self):
        from tools.compare_state_logs import compare
        report = compare(*self.logs('3f800001'))
        self.assertEqual(report['result'], 'differ')
        self.assertEqual(report['race_frames_agreeing'], 1)
        difference = report['difference']
        self.assertEqual((difference['racer'], difference['field'], difference['frames']),
                         (0, 'x', [4, 5]))
        self.assertEqual(difference['a']['float'], 1.0)
        self.assertEqual(difference['b']['bits'], '3f800001')


if __name__ == '__main__':
    unittest.main()
