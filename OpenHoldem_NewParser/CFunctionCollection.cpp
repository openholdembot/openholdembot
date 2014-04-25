//***************************************************************************** 
//
// This file is part of the OpenHoldem project
//   Download page:         http://code.google.com/p/openholdembot/
//   Forums:                http://www.maxinmontreal.com/forums/index.php
//   Licensed under GPL v3: http://www.gnu.org/licenses/gpl.html
//
//***************************************************************************** 
//
// Purpose:
//
//***************************************************************************** 

#include "stdafx.h"
#include "CFunctionCollection.h"

#include "CFunction.h"
#include "CPreferences.h"

CFunctionCollection *p_function_collection = NULL;

CFunctionCollection::CFunctionCollection(){
  _title = "";
  _path = "";
  DeleteAll();
}

CFunctionCollection::~CFunctionCollection()
{}

void CFunctionCollection::DeleteAll() {
  write_log(preferences.debug_formula(), 
    "[CFunctionCollection] DeleteAll()\n");
  _function_map.clear();
}

void CFunctionCollection::Add(COHScriptObject *new_function) {
  CString name = new_function->name();
  if (name == "") {
    write_log(preferences.debug_formula(), 
	  "[CFunctionCollection] Invalid empty name\n");
    return;
  }
  if (Exists(name)) {
    write_log(preferences.debug_formula(), 
	  "[CFunctionCollection] Name %s already exists. Deleting it\n", name);
    _function_map.erase(name);
  }
  write_log(preferences.debug_formula(), 
	"[CFunctionCollection] Adding %s -> %i\n", name, new_function);
  _function_map[name] = new_function;
}

bool CFunctionCollection::Exists(CString name) {
  std::map<CString, COHScriptObject*>::iterator it; 
  it = _function_map.find(name); 
  return (it != _function_map.end());
}

COHScriptObject *CFunctionCollection::LookUp(CString name) {
  write_log(preferences.debug_formula(), "[CFunctionCollection] Lookup %s\n", name); 
  std::map<CString, COHScriptObject*>::iterator it; 
  it = _function_map.find(name); 
  if (it == _function_map.end()) {
    // Function or list does not exist
    write_log(preferences.debug_formula(), "[CFunctionCollection] Function does not exist\n");
    return NULL;
  }
  return it->second;
}

double CFunctionCollection::Evaluate(CString function_name) {
  COHScriptObject *p_function = LookUp(function_name);
  if (p_function == NULL) {
    // Function does not exist
    return k_undefined_zero;
  }
  return p_function->Evaluate();
}

CString CFunctionCollection::DLLPath() {
  COHScriptObject *dll_node = LookUp("DLL");
  if (dll_node == NULL) {
	  return "";
  }
  CString dll_path = dll_node->function_text();
  // Remove "##DLL##"
  dll_path.Delete(0, 7);
  dll_path.Trim();
  return dll_path;
}

void CFunctionCollection::SetEmptyDefaultBot() {
  DeleteAll();
  //!!CSLock lock(m_critsec);
  _title = "NoName";
  // Adding empty standard-functions
  // http://www.maxinmontreal.com/forums/viewtopic.php?f=156&t=16230
  CheckForDefaultFormulaEntries();
}

void CFunctionCollection::CheckForDefaultFormulaEntries() {
  //!!CSLock lock(m_critsec);

  // Header comment
  AddEmptyFunctionIfFunctionDoesNotExist(CString("notes"));
  // DLL to be loaded
  AddEmptyFunctionIfFunctionDoesNotExist(CString("dll"));
  // Autoplayer, standard, ini and PrWin functions
  for (int i=0; i<k_number_of_standard_functions; ++i)
  {
	  AddEmptyFunctionIfFunctionDoesNotExist(CString(k_standard_function_names[i]));
  }
  // Debug functions	
  AddEmptyFunctionIfFunctionDoesNotExist(CString("f$test"));
  AddEmptyFunctionIfFunctionDoesNotExist(CString("f$debug"));
}

void CFunctionCollection::AddEmptyFunctionIfFunctionDoesNotExist(CString &function_name)
{
  if (Exists(function_name)) {
	 return;
  }
  // Formula not found.
  // Add the standard one.
  CString function_text;
  if ((function_name.Compare(k_standard_function_names[k_autoplayer_function_check]) == k_CString_identical)
	  || (function_name.Compare(k_standard_function_names[k_autoplayer_function_fold]) == k_CString_identical))
  {
	// f$check and f$fold should evaluate to true per default
	// for auto-check-folding instead of time-outs.
	function_text = "1 "; //!! header?
  }
  else {
	// Add an empty function.
	// The function-text should contain at least one space.
	// The editor does somehow not work for completely empty formulas.
	function_text = " "; //!! header?
  }
  CFunction *p_function = new CFunction(&function_name, 
    &function_text); // 
  Add((COHScriptObject *)p_function);
}

COHScriptObject *CFunctionCollection::GetFirst() {
  enumerator_it = _function_map.begin();
  return GetNext();
}

COHScriptObject *CFunctionCollection::GetNext() {
  if (enumerator_it == _function_map.end()) {
    // "end" points behind the last element
    return NULL;
  }
  COHScriptObject *result = enumerator_it->second;
  enumerator_it++;
  return result;
}

void CFunctionCollection::ClearCache() {
  COHScriptObject *p_oh_script_object = GetFirst();
  while (p_oh_script_object != NULL) {
    if (p_oh_script_object->IsFunction()) {
      ((CFunction*)p_oh_script_object)->ClearCache();
    }
    p_oh_script_object = GetNext();
  }
}

void CFunctionCollection::Save(CArchive &ar)
{
  // First write the date
  char nowtime[26] = {0};
  CString s;
  s.Format("##%s##\r\n\r\n", get_time(nowtime)); 
  ar.WriteString(s);
  // DLL  and notes are a bit special "functions",
  // so they get extra treatment.
  SaveObject(ar, LookUp("notes"));
  SaveObject(ar, LookUp("dll"));
  // Then write the standard formula functions...
  // These are functions and symbols, that
  //   * are essential to control the behaviour 
  //	 of (nearly) every poker bot.
  //   * configure some very important constants.
  for (int i=k_autoplayer_function_allin; 
      i<k_number_of_standard_functions; 
      ++i) {
	SaveObject(ar, LookUp(k_standard_function_names[i]));
  }
  // f$test and f$debug are again special
  SaveObject(ar, LookUp("f$test"));
  SaveObject(ar, LookUp("f$debug"));
  // Handlists
  COHScriptObject *next_object = GetFirst();
  while (next_object != NULL) {
    if (next_object->IsList()) {
      SaveObject(ar, next_object);
    }
    next_object = GetNext();
  }
  // User defined functions
  // We already saved the standard-functions
  next_object = GetFirst();
  while (next_object != NULL) {
    if (next_object->IsFunction()
        && !next_object->IsStandardFunction()) {
      SaveObject(ar, next_object);
    }
    next_object = GetNext();
  }
}

void CFunctionCollection::SaveObject(
    CArchive &ar, 
    COHScriptObject *function_or_list) {
  if (function_or_list == NULL) return;
  ar.WriteString(function_or_list->Serialize());
  
}