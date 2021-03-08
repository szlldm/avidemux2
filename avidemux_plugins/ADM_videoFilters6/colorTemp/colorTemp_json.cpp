// automatically generated by admSerialization.py, do not edit!
#include "ADM_default.h"
#include "ADM_paramList.h"
#include "ADM_coreJson.h"
#include "colorTemp.h"
bool  colorTemp_jserialize(const char *file, const colorTemp *key){
admJson json;
json.addFloat("temperature",key->temperature);
json.addFloat("angle",key->angle);
return json.dumpToFile(file);
};
bool  colorTemp_jdeserialize(const char *file, const ADM_paramList *tmpl,colorTemp *key){
admJsonToCouple json;
CONFcouple *c=json.readFromFile(file);
if(!c) {ADM_error("Cannot read json file");return false;}
bool r= ADM_paramLoadPartial(c,tmpl,key);
delete c;
return r;
};