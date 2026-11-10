// Compiled with /QIfist: VC6 then converts with FISTP under the current x87
// rounding mode instead of calling __ftol. In signed32 range that is the
// result of retail's inline `fld; fistp dword ptr [eax]` helper.
int OracleFloatToInt(float value)
{
    return (int)value;
}
