#ifndef _C_UMLCLASSDIAGRAM_H_
#define _C_UMLCLASSDIAGRAM_H_


class cUMLClassDiagram { 
protected: 
  static std::string class_xmi;

	
public:

  std::string getClassXMI () { return class_xmi; }  // get the xmi string 
  void setClassXMI (std::string x) { class_xmi = x; } 
  
};

#endif
