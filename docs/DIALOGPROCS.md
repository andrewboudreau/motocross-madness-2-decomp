# InGameProcs.cpp and NetProcs.cpp

These files hold dialog procedures. Each one is a slot-29 override of a
UIDialog subclass named by RTTI, plus a few helpers. The sources are
`src/reconstructed/InGameProcs.*` and `NetProcs.*`; `DialogProc.h` declares
what both use. That covers the slot-29 event record, the dlgprocs.cpp calls
`0x004526b0`/`0x00452930`/`0x00453090`, and the inline-constructed
ChoiceDlg (vtable `0x00551cb0`, "messbox2.dtm") and SessionDlg
(`0x00555408`, "messbox1.dtm"). Names are provisional.

**InGameProcs.cpp** (`0x004886e0..0x0048963b`). Evidence: the `__FILE__`
literal at `0x0056c5cc` (xref `0x00488928`). It starts where GUIManager.cpp
ends. The `$E` initializers at `0x00489640` open the next unit
(InputDevice). It holds the slot 29 of ExitDlg, ContinueDlg and VCRDlg,
plus VCRDlg's slot 10, its toggles `0x004894c0` and the replay time format
`0x00488bd0`.

**NetProcs.cpp** (`0x004ae460..0x004af672`). Evidence:
- **`__FILE__`:** the literal at `0x0056eb2c`, xrefs from `0x004ae64d` to
  `0x004af306`.
- **`.data`:** starts at `0x0056eadc` with the baud table, after Net.cpp's.
- **`.bss`:** `0x006886b8..0x006886cc`, between Net.cpp's and
  NetThread.cpp's globals.

It holds the slot 29 of HostJoinDlg, SerialPopupDlg, TCPAddressDlg,
SessionDlg, WaitOrCallDlg, ConnectErrorDlg and PlayerRemovedDlg. It also
has the session join `0x004ae460` and the network drop and ensure
functions `0x004aef40`/`0x004aefa0`. The serial helpers at
`0x004ae2f0..0x004ae454` map baud, parity and other settings; Net.cpp and
NetProcs.cpp are equally possible owners, so they are not claimed.

Exact: all 16 functions (6 and 10). Source forms needed:
- `_stricmp` calls take the literal first.
- ExitDlg's two notification branches are written out in full, and VC6
  merges their tail.
- WaitOrCallDlg sets `event->field_0x20 = 1` in both branches.

Header changes, all layout-preserving:
- GameUi.h's UnknownGameUiControl derives from GameObject, as RTTI
  UIControl does.
- UIDialog.h, TrackGame.h and RaceView.h gained the members these
  procedures use.
- FollowCamera.h befriends VCRDlg, as Camera.h does for its readers.
