# NetThread.cpp

`src/reconstructed/NetThread.h` / `NetThread.cpp`. Evidence: the
`__FILE__` literal for NetThread.cpp at `0x0056ec6c`. Its xrefs fall in
`0x004af6a0`, `0x004af8e0` and `0x004afa90`, and the unwind funclets
`0x0054c626`/`0x0054c63d` belong to `0x004afa90`. The file statics
`0x006886d0..0x006886e4` are used only inside this range.

Extent: `0x004af680..0x004aff73`. The dialog procs `0x004af470` and
`0x004af590` call FindControl and are left to NetProcs.cpp. `0x004aff80`
starts NormalDistribution.cpp, which is already matched in
`src/krusty2/effects/NormalDistribution.cpp`. `0x004af680` is placed here
by adjacency only: its sole caller is `0x004af6a0`.

The receive thread `0x004af6a0` is the `_beginthreadex` target in Net.cpp
`0x004ab6b0`. It runs the receive loop `0x004af8e0` and the message
dispatcher `0x004afa90`, and it drops duplicate messages through
`0x004afa40`.

Exact: 3 calibration cases (`0x004af680`, `0x004af6a0`, `0x004afa40`).
In `0x004af6a0` the second 0.001 factor must be written `/ 1000.0f`. VC6
then multiplies by the reciprocal, which is retail's separate constant at
`0x005507d4`.

Near misses (`samples/net/NetThreadNearMisses.cpp`): the receive loop
`0x004af8e0` and the dispatcher `0x004afa90`. Both differ only in
register assignment.

Open question: `0x004af6a0` passes a float (`fstp [esp]`) to
NetworkInterface `0x004acc50`, but Net.cpp's body uses that argument as an
int, as the initial value of the lost player id. The real parameter type
is unresolved. NetThread.h therefore calls the method through
`NetKeepAliveView`. A float overload in Net.h was tried and
rejected: it makes VC6 swap the operands of `availPhys + availPageFile` in
TrackGame slot 1 (`0x00520ab0`). Equal-cost commutative operand order can
depend on unrelated declarations in included headers.
