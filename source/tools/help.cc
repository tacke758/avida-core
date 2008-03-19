
#include "help.hh"


using namespace std;


void cHelpType::PrintHTML()
{
  if (manager->GetVerbose()) cout << "  Category: " << GetName() << endl;

  cHelpEntry * cur_entry = NULL;
  tListIterator<cHelpEntry> entry_it(entry_list);

  while ( (cur_entry = entry_it.Next()) != NULL) {
    // Only print non-alias entries...
    if (cur_entry->IsAlias() == true) continue;
    
    ofstream fp(cur_entry->GetHTMLFilename());
    
    fp << "<html>" << endl
       << "<title>" << GetName() << " : "
       << cur_entry->GetName() << "</title>" << endl << endl
       << "<h1>" << cur_entry->GetName() << "</h1>" << endl << endl;
    
    cString out_desc( cur_entry->GetDesc() );
    int pos = 0;
    while ((pos = out_desc.Find('!', pos)) != -1) {
      // Grab the word we need to replace
      cString found_word(out_desc.GetWordAt(pos));
      cString new_word;
      
      // If we have a double '!' reduce it to a single one and continue...
      if (found_word[1] == '!') {
	out_desc.Replace("!!", "!", pos);
	pos++;
      }
      
      // Otherwise, do a proper replacement...
      else {
	// Find the root keyword...
	cString keyword(found_word);
	keyword.ClipFront(1); // Clip off the '!' on the string.
	
	// Clip end punctuation and save it if there is any.
	char end_char = ' ';
	const int last_pos = keyword.GetSize() - 1;
	if (keyword.IsNumeric(last_pos) == false &&
	    keyword.IsLetter(last_pos) == false) {
	  end_char = keyword[keyword.GetSize() - 1];
	  keyword.ClipEnd(1);
	}
	// User can end a word in a '!' to preserve other punctuation...
	if (end_char == '!') end_char = ' ';

	// Determine what filename contains the new word...
	cHelpEntry * found_entry = manager->FindEntry(keyword);
	if (found_entry == NULL) {
	  if (manager->GetVerbose() == true) {
	    cerr << "  Warning: unknown help keyword \""
		 << keyword << "\"." << endl;
	  }
	  new_word.Set("<a href=\"help.%s.html\">%s</a>%c",
		       keyword(), keyword(), end_char);
	}
	else {
	  new_word.Set("<a href=\"%s\">%s</a>%c",
		  found_entry->GetHTMLFilename()(), keyword(), end_char);
	}

	// Rebuild the description with the new word...
	out_desc.Replace(found_word, new_word, pos);
	pos += new_word.GetSize();
      }
    }

    fp << out_desc << endl;
  }
}


cHelpType * cHelpManager::GetType(const cString type_name)
{
  // See if we can find this type in the already existant list...
  tListIterator<cHelpType> type_it(type_list);
  while (type_it.Next() != NULL) {
    if (type_it.Get()->GetName() == type_name) return type_it.Get();
  }

  // Otherwise, create it.
  if (verbose == true) {
    cout << "  Creating help type \"" << type_name << "\"." << endl;
  }
  cHelpType * type = new cHelpType(type_name, this);
  type_list.Push(type);
  return type;
}

void cHelpManager::LoadFile(const cString & filename)
{
  cInitFile help_file(filename);
  help_file.Load();
  help_file.Compress();
  help_file.Close();

  cHelpType * type = NULL;
  cString keyword;

  for (int line_id = 0; line_id < help_file.GetNumLines(); line_id++) {
    cString cur_string = help_file.GetLine(line_id);
    cString command = cur_string.PopWord();
    command.ToLower();

    if (command == "type:") {
      type = GetType(cur_string);
    }
    else if (command == "keyword:") {
      keyword = cur_string;
    }
    else if (command == "desc:") {
      if (type == NULL) type = GetType("None"); // Make sure we have a type.
      if (keyword == "") {
	if (verbose == true) {
	  cerr << "  Help description set without keyword;"
	       << " setting keyword to \"None\"." << endl;
	}
	keyword = "None";
      }
      last_entry = type->AddEntry(keyword, cur_string);
    }
    else if (command == "alias:") {
      if (last_entry == NULL) {
	cerr << "  Warning: Setting aliases \"" << cur_string
	     << "\" to incomplete entry!" << endl;
      }
      while (cur_string.GetSize() > 0) {
	type->AddAlias(cur_string.PopWord(), last_entry);
      }
    }
    else if (verbose == true) {
      cerr << "Warning: Unknown command \"" << command
	   << "\" in file " << filename << endl;
    }
  }

}


void cHelpManager::PrintHTML()
{
  tListIterator<cHelpType> type_it(type_list);
  while (type_it.Next() != NULL) {
    type_it.Get()->PrintHTML();
  }
}
