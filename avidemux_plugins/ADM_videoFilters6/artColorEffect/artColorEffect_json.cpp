// automatically generated by admSerialization.py, do not edit!
#include "ADM_default.h"
#include "ADM_paramList.h"
#include "ADM_coreJson.h"
#include "artColorEffect.h"
bool  artColorEffect_jserialize(const char *file, const artColorEffect *key){
admJson json;
json.addUint32("effect",key->effect);
return json.dumpToFile(file);
};
bool  artColorEffect_jdeserialize(const char *file, const ADM_paramList *tmpl,artColorEffect *key){
admJsonToCouple json;
CONFcouple *c=json.readFromFile(file);
if(!c) {ADM_error("Cannot read json file");return false;}
bool r= ADM_paramLoadPartial(c,tmpl,key);
delete c;
return r;
};