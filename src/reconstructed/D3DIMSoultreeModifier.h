#pragma once

// D3DIMSoultreeModifier.h -- the reconstructed D:\aardvark\VC\krusty2\
// D3DIMSoultreeModifier.cpp (literal __FILE__ at 0x00568b24, xrefs
// 0x004452fa..0x0044544a; code 0x00445240..0x00445460, after
// D3DIMSoulTree.CPP and before D3DIMSoultreeMotnctrl.cpp).
//
// Confirmed (tier 1): RTTI .?AVD3DIMSoultreeModifier@@ (COL 0x0055b310),
// D3DIMSoultreeModifier : GraphicsTest : GameObject : BaseObject, all single
// non-virtual at mdisp 0; vtable 0x0055144c (28 slots). It overrides slot 0
// (deleting destructor 0x00445270) and slot 8 (0x004452e0, a body the linker
// shares with LightManager and others) and adds slot 27, which is _purecall
// (0x00534cfe). TransparencyMod, GhostMod1 and MorphBastardModifier derive
// from it. The constructor 0x00445240 and the destructor 0x00445290 write the
// vptr. Member and method names are provisional (tier 3).
//
// The modifier keeps the D3DIMSoultreeObjects it is attached to; the objects
// call AddObject / RemoveObject when a modifier is added
// to or removed from one of their two modifier lists (+0x264 / +0x26c).

#include "Wrecker.h"       // GraphicsTest
#include "LightEmitter.h"  // D3DIMSoultreeObject

struct UnknownSoultreeMesh;      // MorphBastardModifier.h

#include "D3DIMSoultreeModifierCore.h"
