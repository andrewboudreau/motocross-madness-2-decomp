#pragma once

// GhostMod.h -- RTTI .?AVGhostMod1@@ (COL 0x0055c9e8, vtable 0x00553d70):
// GhostMod1 : D3DIMSoultreeModifier : GraphicsTest : GameObject :
// BaseObject, all single non-virtual at mdisp 0 (tier 1). It overrides slot
// 0 (deleting destructor 0x0047bb50), slot 10 (0x0047bc20) and slot 27
// (0x0047bb70). The constructor 0x0047bb20 and the destructor 0x0047bc60
// write the vptr.
//
// Code 0x0047bb20..0x0047bc6a, after gameui.cpp and before the GraphicsTest
// code (0x0047bc70). No __FILE__ literal: the file name 'GhostMod.cpp' and
// the member names are ours (tier 3).
//
// What it does (tier 3): StartFade starts a fade of `duration`
// seconds (0: none, both fields FLT_MAX); slot 10 counts field_0x40 down by
// the frame time and slot 27 sets the alpha byte of every vertex colour to
// 200 - 100 * remaining / duration while time remains.

#include "D3DIMSoultreeModifier.h"
#include "MorphBastardModifier.h"   // UnknownSoultreeMesh

#include "GhostModCore.h"
