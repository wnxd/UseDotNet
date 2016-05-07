#pragma once

#ifdef LoadLibrary
#undef LoadLibrary
#endif

#define typeof(type) type::typeid
#define null nullptr
#define ChangeValueType(prefix, name, intype, outtype) outtype WINAPI prefix##name(intype vt)
#define CreateValueType(name, intype, outtype) ChangeValueType(Create, name, intype, outtype)
#define Method_CreateValueType(name, intype, outtype) CreateValueType(name, intype, outtype) { return Object2HOBJECT(vt); }
#define Object2ValueType(name, intype, outtype) ChangeValueType(Get, name, intype, outtype)
#define Method_Object2ValueType(name, intype, outtype) Object2ValueType(name, intype, outtype) { return (outtype)HOBJECT2Type(vt, Object); }

typedef HINSTANCE HOBJECT;
typedef HOBJECT HTYPE;
typedef HOBJECT HMETHOD;
typedef HOBJECT HSTRING;
typedef HOBJECT HINT;
typedef HOBJECT HBOOL;
typedef HOBJECT HBYTE;
typedef HOBJECT HLONG;
typedef HOBJECT HSHORT;
typedef HOBJECT HFLOAT;
typedef HOBJECT HDOUBLE;

template<typename T>
struct CArray
{
	UINT Count;
	T List[1];
};

EXPORT bool WINAPI LoadLibrary(LPSTR path);
EXPORT HTYPE WINAPI FindType(LPSTR fullname);
EXPORT HOBJECT WINAPI CreateInstance(HTYPE type, CArray<HOBJECT>* params);
EXPORT HSTRING WINAPI CreateString(LPSTR str);
EXPORT CreateValueType(Int, int, HINT);
EXPORT CreateValueType(Bool, bool, HBOOL);
EXPORT CreateValueType(Byte, byte, HBYTE);
EXPORT CreateValueType(Long, INT64, HLONG);
EXPORT CreateValueType(Short, short, HSHORT);
EXPORT CreateValueType(Float, float, HFLOAT);
EXPORT CreateValueType(Double, double, HDOUBLE);
EXPORT LPSTR WINAPI GetString(HSTRING str);
EXPORT Object2ValueType(Int, HINT, int);
EXPORT Object2ValueType(Bool, HBOOL, bool);
EXPORT Object2ValueType(Byte, HBYTE, byte);
EXPORT Object2ValueType(Long, HLONG, INT64);
EXPORT Object2ValueType(Short, HSHORT, short);
EXPORT Object2ValueType(Float, HFLOAT, float);
EXPORT Object2ValueType(Double, HDOUBLE, double);
EXPORT HMETHOD WINAPI FindMethod(HTYPE type, LPSTR name, CArray<HTYPE>* args, bool isstatic);
EXPORT HOBJECT WINAPI CallMethod(HMETHOD method, HOBJECT obj, CArray<HOBJECT>* params);
EXPORT void WINAPI Fixed(HOBJECT obj);
EXPORT void WINAPI Free(HOBJECT obj);