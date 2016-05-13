#include "stdafx.h"
#include "UseDotNet.h"

using namespace System;
using namespace System::IO;
using namespace System::Reflection;
using namespace System::Collections::Generic;
using namespace System::Text::RegularExpressions;
using namespace System::Runtime::InteropServices;

const BindingFlags BINDING_ALL = BindingFlags::Public | BindingFlags::NonPublic | BindingFlags::Instance | BindingFlags::Static;
const BindingFlags BINDING_ALLSTATIC = BindingFlags::Public | BindingFlags::NonPublic | BindingFlags::Static;
const BindingFlags BINDING_ALLINSTANCE = BindingFlags::Public | BindingFlags::NonPublic | BindingFlags::Instance;

#define String2LPSTR(str) (char*)(void*)Marshal::StringToHGlobalAnsi(str)
#define LPSTR2String(lpstr) Marshal::PtrToStringAnsi((IntPtr)(void*)lpstr)
#define I(hobj) (int)hobj

HOBJECT Object2HOBJECT(Object^ obj);

static ref class Global
{
internal:
	static String^ SystemPath;
	static Dictionary<int, Object^>^ FixedList;
	static List<Assembly^>^ LibList;
	static List<Type^>^ LibTypeList;
	static Dictionary<String^, Type^>^ TypeList;

	static Global()
	{
		Global::SystemPath = Environment::GetFolderPath(Environment::SpecialFolder::Windows) + "\\Microsoft.NET\\Framework\\v4.0.30319\\";
		Global::FixedList = gcnew Dictionary<int, Object^>();
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
		Global::TypeList->Add("array", typeof(Array));
	}
};

ref class EventHelper
{
private:
	delegate void NativeEvent(HOBJECT sender, HOBJECT e);

	static MethodInfo^ _shellmethod = typeof(EventHelper)->GetMethod("ShellMethod", BINDING_ALLINSTANCE);
	NativeEvent^ nm;
	Delegate^ d;

	void ShellMethod(Object^ sender, EventArgs^ e)
	{
		this->nm->Invoke(Object2HOBJECT(sender), Object2HOBJECT(e));
	}
internal:
	EventHelper(Type^ type, int method)
	{
		this->nm = (NativeEvent^)Marshal::GetDelegateForFunctionPointer(IntPtr(method), typeof(NativeEvent));
		ConstructorInfo^ ctor = type->GetConstructors()[0];
		this->d = (Delegate^)ctor->Invoke(gcnew array<Object^> { this, EventHelper::_shellmethod->MethodHandle.GetFunctionPointer() });
	}

	Delegate^ GetShellMethod()
	{
		return d;
	}
};

HOBJECT Object2HOBJECT(Object^ obj)
{
	return *(HOBJECT*)&obj;
}

template<typename T>
T^ HOBJECT2Type(HOBJECT obj)
{
	if (Global::FixedList->ContainsKey(I(obj))) return (T^)Global::FixedList[I(obj)];
	return *(T^*)&obj;
}

template<typename T>
array<T^>^ CArray2array(CArray<HOBJECT>* arr)
{
	array<T^>^ arr2 = gcnew array<T^>(arr->Count);
	for (int i = 0; i < arr->Count; i++) arr2[i] = HOBJECT2Type<T>(arr->List[i]);
	return arr2;
}

String^ GetFullName(Type^ gt)
{
	String^ name = gt->FullName;
	if (gt->IsGenericType)
	{
		array<Type^>^ types = gt->GetGenericArguments();
		Regex^ regex = gcnew Regex("^(.+?)`" + types->Length + "\\[.+\\]$");
		Match^ mc = regex->Match(name);
		if (mc->Success)
		{
			name = mc->Groups[1]->Value + "<";
			for (int i = 0; i < types->Length; i++)
			{
				if (i > 0) name += ", ";
				Type^ T = types[i];
				name += T->IsGenericType ? GetFullName(T) : T->FullName;
			}
			name += ">";
		}
	}
	return name;
}

bool LoadLibrary(LPSTR path)
{
	Assembly^ assembly;
	String^ name = LPSTR2String(path);
	if (!name->EndsWith(".dll")) name += ".dll";
	if (File::Exists(name))
	{
	load:
		assembly = Assembly::LoadFrom(name);
		if (assembly != null)
		{
			if (!Global::LibList->Contains(assembly))
			{
				Global::LibList->Add(assembly);
				Global::LibTypeList->AddRange(assembly->GetTypes());
			}
			return true;
		}
	}
	else
	{
		name = Global::SystemPath + name;
		if (File::Exists(name)) goto load;
	}
	return false;
}

HTYPE FindType(LPSTR fullname)
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
				if (GetFullName(t) == name)
				{
					type = t;
					break;
				}
			}
		}
	}
	return Object2HOBJECT(type);
}

HOBJECT CreateInstance(HTYPE type, CArray<HOBJECT>* params)
{
	array<Object^>^ arr;
	if (params == null) arr = gcnew array<Object^>(0);
	else arr = CArray2array<Object>(params);
	Object^ obj = Activator::CreateInstance(HOBJECT2Type<Type>(type), arr);
	return Object2HOBJECT(obj);
}

HDELEGATE CreateDelegate(HTYPE type, int method)
{
	Object^ obj = Marshal::GetDelegateForFunctionPointer(IntPtr(method), HOBJECT2Type<Type>(type));
	return Object2HOBJECT(obj);
}

HDELEGATE CreateEvent(HTYPE type, int method)
{
	EventHelper^ helper = gcnew EventHelper(HOBJECT2Type<Type>(type), method);
	return Object2HOBJECT(helper->GetShellMethod());
}

HSTRING CreateString(LPSTR str)
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

LPSTR GetString(HSTRING str)
{
	return String2LPSTR(HOBJECT2Type<String>(str));
}

Method_Object2ValueType(Int, HINT, int)
Method_Object2ValueType(Bool, HBOOL, bool)
Method_Object2ValueType(Byte, HBYTE, byte)
Method_Object2ValueType(Long, HLONG, INT64)
Method_Object2ValueType(Short, HSHORT, short)
Method_Object2ValueType(Float, HFLOAT, float)
Method_Object2ValueType(Double, HDOUBLE, double)

HMETHOD FindDefaultMethod(HTYPE type, LPSTR name)
{
	Type^ t = HOBJECT2Type<Type>(type);
	return Object2HOBJECT(t->GetMethod(LPSTR2String(name)));
}

HMETHOD FindMethod(HTYPE type, LPSTR name, CArray<HTYPE>* args, bool isstatic)
{
	array<Type^>^ types;
	if (args == null) types = gcnew array<Type^>(0);
	else types = CArray2array<Type>(args);
	Type^ t = HOBJECT2Type<Type>(type);
	BindingFlags flags = isstatic ? BINDING_ALLSTATIC : BINDING_ALLINSTANCE;
	return Object2HOBJECT(t->GetMethod(LPSTR2String(name), flags, null, types, null));
}

HOBJECT CallMethod(HMETHOD method, HOBJECT obj, CArray<HOBJECT>* params)
{
	array<Object^>^ arr;
	if (params == null) arr = gcnew array<Object^>(0);
	else arr = CArray2array<Object>(params);
	MethodInfo^ m = HOBJECT2Type<MethodInfo>(method);
	Object^ o = HOBJECT2Type<Object>(obj);
	return Object2HOBJECT(m->Invoke(o, arr));
}

void Fixed(HOBJECT obj)
{
	if (!Global::FixedList->ContainsKey(I(obj)))
	{
		Object^ o = HOBJECT2Type<Object>(obj);
		Global::FixedList->Add(I(obj), o);
	}
}

void Free(HOBJECT obj)
{
	Global::FixedList->Remove(I(obj));
}