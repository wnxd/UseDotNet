#include "stdafx.h"
#include "UseDotNet.h"

using namespace System;
using namespace System::Reflection;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;

const BindingFlags BINDING_ALL = BindingFlags::Public | BindingFlags::NonPublic | BindingFlags::Instance | BindingFlags::Static;
const BindingFlags BINDING_ALLSTATIC = BindingFlags::Public | BindingFlags::NonPublic | BindingFlags::Static;
const BindingFlags BINDING_ALLINSTANCE = BindingFlags::Public | BindingFlags::NonPublic | BindingFlags::Instance;

#define String2LPSTR(str) (char*)(void*)Marshal::StringToHGlobalAnsi(str)
#define LPSTR2String(lpstr) Marshal::PtrToStringAnsi((IntPtr)(void*)lpstr)
#define HOBJECT2Type(obj, type) *(type^*)&obj

static ref class Global
{
internal:
	static List<Object^>^ FixedList;
	static List<Assembly^>^ LibList;
	static List<Type^>^ LibTypeList;
	static Dictionary<String^, Type^>^ TypeList;

	static Global()
	{
		Global::FixedList = gcnew List<Object^>();
		Global::LibList = gcnew List<Assembly^>();
		Global::LibTypeList = gcnew List<Type^>();
		Global::TypeList = gcnew Dictionary<String^, Type^>();
		Global::TypeList->Add("void", typeof(void));
		Global::TypeList->Add("byte", typeof(byte));
		Global::TypeList->Add("short", typeof(short));
		Global::TypeList->Add("int", typeof(int));
		Global::TypeList->Add("bool", typeof(bool));
		Global::TypeList->Add("float", typeof(float));
		Global::TypeList->Add("double", typeof(double));
		Global::TypeList->Add("long", typeof(long));
		Global::TypeList->Add("string", typeof(String));
		Global::TypeList->Add("object", typeof(Object));
		Global::TypeList->Add("type", typeof(Type));
	}
};

HOBJECT Object2HOBJECT(Object^ obj)
{
	return *(HOBJECT*)&obj;
}

template<typename T>
array<T^>^ CArray2array(CArray<HOBJECT> arr)
{
	array<T^>^ arr2 = gcnew array<T^>(arr.Count);
	for (int i = 0; i < arr.Count; i++) arr2[i] = HOBJECT2Type(arr.List[i], T);
	return arr2;
}

bool WINAPI LoadLibrary(LPSTR path)
{
	Assembly^ assembly;
	assembly = Assembly::LoadFrom(LPSTR2String(path));
	if (assembly != null)
	{
		if (!Global::LibList->Contains(assembly))
		{
			Global::LibList->Add(assembly);
			Global::LibTypeList->AddRange(assembly->GetTypes());
		}
		return true;
	}
	return false;
}

HTYPE WINAPI FindType(LPSTR fullname)
{
	String^ name = LPSTR2String(fullname);
	Type^ type;
	if (Global::TypeList->ContainsKey(name)) type = Global::TypeList[name];
	else
	{
		type = Type::GetType(name);
		if (type == null)
		{
			for each (Type^ t in Global::LibTypeList)
			{
				if (t->FullName == name)
				{
					type = t;
					break;
				}
			}
		}
	}
	return Object2HOBJECT(type);
}

HOBJECT WINAPI CreateInstance(HTYPE type, CArray<HOBJECT>* params)
{
	array<Object^>^ arr;
	if (params == null) arr = gcnew array<Object^>(0);
	else arr = CArray2array<Object>(*params);
	Object^ obj = Activator::CreateInstance(HOBJECT2Type(type, Type), arr);
	return Object2HOBJECT(obj);
}

HSTRING WINAPI CreateString(LPSTR str)
{
	return Object2HOBJECT(LPSTR2String(str));
}

Method_CreateValueType(Int, int, HINT)
Method_CreateValueType(Bool, bool, HBOOL)
Method_CreateValueType(Byte, byte, HBYTE)
Method_CreateValueType(Long, INT64, HLONG)
Method_CreateValueType(Short, short, HSHORT)
Method_CreateValueType(Float, float, HFLOAT)
Method_CreateValueType(Double, double, HDOUBLE)

LPSTR WINAPI GetString(HSTRING str)
{
	return String2LPSTR(HOBJECT2Type(str, String));
}

Method_Object2ValueType(Int, HINT, int)
Method_Object2ValueType(Bool, HBOOL, bool)
Method_Object2ValueType(Byte, HBYTE, byte)
Method_Object2ValueType(Long, HLONG, INT64)
Method_Object2ValueType(Short, HSHORT, short)
Method_Object2ValueType(Float, HFLOAT, float)
Method_Object2ValueType(Double, HDOUBLE, double)

HMETHOD WINAPI FindMethod(HTYPE type, LPSTR name, CArray<HTYPE>* args, bool isstatic)
{
	array<Type^>^ types;
	if (args == null) types = gcnew array<Type^>(0);
	else types = CArray2array<Type>(*args);
	Type^ t = HOBJECT2Type(type, Type);
	BindingFlags flags = isstatic ? BINDING_ALLSTATIC : BINDING_ALLINSTANCE;
	return Object2HOBJECT(t->GetMethod(LPSTR2String(name), flags, null, types, null));
}

HOBJECT WINAPI CallMethod(HMETHOD method, HOBJECT obj, CArray<HOBJECT>* params)
{
	array<Object^>^ arr;
	if (params == null) arr = gcnew array<Object^>(0);
	else arr = CArray2array<Object>(*params);
	MethodInfo^ m = HOBJECT2Type(method, MethodInfo);
	return Object2HOBJECT(m->Invoke(HOBJECT2Type(obj, Object), arr));
}

void WINAPI Fixed(HOBJECT obj)
{
	Global::FixedList->Add(HOBJECT2Type(obj, Object));
}

void WINAPI Free(HOBJECT obj)
{
	Global::FixedList->Remove(HOBJECT2Type(obj, Object));
}